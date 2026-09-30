"""Build, deploy, smoke-test, and zip a clean Windows x64 tester release."""

import argparse
import ctypes
from ctypes import wintypes
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import socket
import sqlite3
import subprocess
import tempfile
import time
import urllib.error
import urllib.request
import zipfile


ROOT = Path(__file__).resolve().parents[1]


def run(command, cwd, environment=None):
    print("Running:", subprocess.list2cmdline([str(part) for part in command]), flush=True)
    subprocess.run([str(part) for part in command], cwd=cwd, env=environment, check=True)


def compiler_environment(devcmd, qt):
    result = subprocess.run(f'call "{devcmd}" -arch=x64 >nul && set', shell=True,
                            capture_output=True, text=True, errors="replace", check=True)
    environment = dict(os.environ)
    for line in result.stdout.splitlines():
        if "=" in line and not line.startswith("="):
            key, value = line.split("=", 1)
            environment[key.upper()] = value
    environment["PATH"] = str(qt / "bin") + os.pathsep + environment["PATH"]
    return environment


def main_window(pid, title):
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    user32.EnumWindows.argtypes = [callback_type, wintypes.LPARAM]
    user32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
    user32.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
    user32.IsWindowVisible.argtypes = [wintypes.HWND]
    found = []

    @callback_type
    def visit(window, parameter):
        process_id = wintypes.DWORD()
        user32.GetWindowThreadProcessId(window, ctypes.byref(process_id))
        if process_id.value == pid and user32.IsWindowVisible(window):
            text = ctypes.create_unicode_buffer(512)
            user32.GetWindowTextW(window, text, len(text))
            if text.value == title:
                found.append(window)
        return True

    user32.EnumWindows(visit, 0)
    return found[0] if found else None


def close_application(process, window):
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    user32.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
    if not user32.PostMessageW(window, 0x0010, 0, 0):  # WM_CLOSE
        raise ctypes.WinError(ctypes.get_last_error())
    if process.wait(timeout=15) != 0:
        raise RuntimeError("Packaged application did not exit cleanly")


def verify_empty_database(database):
    with sqlite3.connect(database) as connection:
        for table in ("BibleVersions", "BibleBooks", "BibleVerse", "Songbooks", "Songs",
                      "Announcements", "Media", "SlideShows", "Slides"):
            if connection.execute(f'SELECT count(*) FROM "{table}"').fetchone()[0] != 0:
                raise RuntimeError(f"Tester database unexpectedly contains {table}")
        for table in ("ThemePassive", "ThemeBible", "ThemeSong", "ThemeAnnounce"):
            if connection.execute(f'SELECT count(*) FROM "{table}"').fetchone()[0] != 4:
                raise RuntimeError(f"Default themes are incomplete: {table}")
            if connection.execute(f'SELECT count(*) FROM "{table}" WHERE use_background = 1').fetchone()[0]:
                raise RuntimeError(f"Unexpected custom background in {table}")
        settings = dict(connection.execute("SELECT type, sets FROM Settings"))
        if len(settings) != 7 or "stream" in settings:
            raise RuntimeError("First launch did not create default settings")
        general = dict(line.strip().split("=", 1) for line in settings["general"].splitlines() if "=" in line)
        general = {key.strip(): value.strip() for key, value in general.items()}
        for key, expected in {"displayIsOnTop": "false", "displayOnStartUp": "false", "displayScreen": "0",
                              "displayScreen2": "-1", "displayScreen3": "-1", "displayScreen4": "-1"}.items():
            if general.get(key) != expected:
                raise RuntimeError(f"Unexpected default {key}: {general.get(key)}")


def smoke_test(package, work, label):
    # Testing a copy keeps newly created settings out of the actual ZIP.
    deployed = work / "smoke-test"
    shutil.copytree(package, deployed)
    environment = {key: value for key, value in os.environ.items()
                   if not key.upper().startswith(("QT_", "QML"))}
    windows = Path(environment.get("SystemRoot", r"C:\Windows"))
    environment["PATH"] = os.pathsep.join(map(str, (deployed, windows / "System32", windows)))
    environment["QT_FORCE_STDERR_LOGGING"] = "1"
    executable = deployed / "SoftProjector.exe"
    database = deployed / "spData.sqlite"
    title = "SoftProjector " + label

    def launch():
        process = subprocess.Popen([executable], cwd=deployed, env=environment)
        try:
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                if process.poll() is not None:
                    raise RuntimeError(f"Packaged application exited during startup: {process.returncode}")
                window = main_window(process.pid, title)
                if window and database.exists():
                    with sqlite3.connect(database) as connection:
                        try:
                            if connection.execute("SELECT count(*) FROM Settings").fetchone()[0] >= 7:
                                return process, window
                        except sqlite3.OperationalError:
                            pass
                time.sleep(.2)
            raise RuntimeError("Packaged application did not finish startup")
        except BaseException:
            process.kill()
            process.wait()
            raise

    process, window = launch()
    try:
        verify_empty_database(database)
        close_application(process, window)
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()

    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    with sqlite3.connect(database) as connection:
        connection.execute("INSERT INTO Settings (type, sets) VALUES ('stream', ?)",
                           (json.dumps({"enabled": True, "port": port, "canvas": 1080}),))
    process, window = launch()
    url = f"http://127.0.0.1:{port}/stream"
    try:
        with urllib.request.urlopen(url + "/state", timeout=5) as response:
            state = json.load(response)
            if (state["width"], state["height"]) != (1920, 1080):
                raise RuntimeError("Packaged stream has an incorrect canvas size")
        with urllib.request.urlopen(url + "/image", timeout=5) as response:
            if response.read(8) != b"\x89PNG\r\n\x1a\n":
                raise RuntimeError("Packaged stream did not produce an image")
        close_application(process, window)
        try:
            urllib.request.urlopen(url + "/state", timeout=2).close()
        except (urllib.error.URLError, OSError):
            pass
        else:
            raise RuntimeError("Packaged stream remained reachable after exit")
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()
    print("Packaged smoke checks passed: bundled runtime, beta version, empty modules, defaults, stream, clean exit.", flush=True)


def validate_contents(package):
    required = ("SoftProjector.exe", "Qt6Core.dll", "Qt6Quick.dll", "Qt6Multimedia.dll",
                "Qt6HttpServer.dll", "platforms/qwindows.dll", "sqldrivers/qsqlite.dll",
                "qml/QtQuick/qtquick2plugin.dll", "qml/QtMultimedia/quickmultimediaplugin.dll",
                "msvcp140.dll", "vcruntime140.dll", "vcruntime140_1.dll")
    for name in required:
        if not (package / name).is_file():
            raise RuntimeError(f"Missing required runtime file: {name}")
    for file in package.rglob("*"):
        if file.is_file() and (file.suffix.lower() in {".sqlite", ".db", ".ini", ".pdb", ".obj", ".spsc", ".spsm"}
                               or file.name.lower().endswith("d.dll")):
            raise RuntimeError(f"Unexpected user data or debug artifact: {file.relative_to(package)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qt-dir", required=True, type=Path)
    parser.add_argument("--vs-devcmd", required=True, type=Path)
    parser.add_argument("--output-dir", type=Path, default=ROOT / "dist")
    parser.add_argument("--work-dir", type=Path, default=Path(tempfile.gettempdir()))
    args = parser.parse_args()
    if os.name != "nt":
        parser.error("Windows packaging must run on Windows")
    qt = args.qt_dir.resolve()
    devcmd = args.vs_devcmd.resolve()
    if not devcmd.is_file() or not (qt / "bin/windeployqt.exe").is_file():
        parser.error("Qt or Visual Studio tools were not found at the specified paths")
    if not args.work_dir.is_dir():
        parser.error("The temporary work directory must already exist")
    version_header = (ROOT / "src/headers/version.hpp").read_text(encoding="utf-8")
    version = re.search(r'#define SOFTPROJECTOR_VERSION "([^"]+)"', version_header)[1]
    label = re.search(r'#define SOFTPROJECTOR_VERSION_LABEL "([^"]+)"', version_header)[1]
    name = f"SoftProjector-{version}-windows-x64"
    output = args.output_dir.resolve()
    archive = output / (name + ".zip")
    if archive.exists():
        raise RuntimeError(f"Refusing to overwrite an existing release: {archive}")
    if subprocess.check_output(["git", "status", "--porcelain"], cwd=ROOT, text=True).strip():
        raise RuntimeError("Commit the release sources before packaging")
    commit = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    repository = subprocess.check_output(["git", "remote", "get-url", "origin"], cwd=ROOT, text=True).strip()
    repository = repository.removesuffix(".git").replace("git@github.com:", "https://github.com/")
    environment = compiler_environment(devcmd, qt)
    qt_version = subprocess.check_output([qt / "bin/qmake.exe", "-query", "QT_VERSION"], env=environment, text=True).strip()
    with tempfile.TemporaryDirectory(prefix="softprojector-release-", dir=args.work_dir) as temporary:
        work = Path(temporary)
        build = work / "build"
        package = work / name
        build.mkdir()
        package.mkdir()
        run([qt / "bin/qmake.exe", "-o", "Makefile", ROOT / "src/softProjector.pro",
             "CONFIG+=release", "CONFIG+=c++17", "CONFIG-=debug", "CONFIG-=debug_and_release",
             "QMAKE_CXXFLAGS+=/MP4", "-after", f"DESTDIR={package.as_posix()}",
             f"OUT_PWD={build.as_posix()}", f"RES_DIR={build.as_posix()}",
             "OBJECTS_DIR=obj", "MOC_DIR=moc", "UI_DIR=ui", "RCC_DIR=rcc"], build, environment)
        run(["nmake", "/f", "Makefile"], build, environment)
        run([qt / "bin/windeployqt.exe", "--release", "--no-compiler-runtime",
             "--skip-plugin-types", "qmltooling", "--translations", "de,ru,cs,uk,hy",
             "--qmldir", ROOT / "src/qml", package / "SoftProjector.exe"], build, environment)
        runtime_root = devcmd.parents[2] / "VC/Redist/MSVC"
        runtimes = sorted(runtime_root.glob("*/x64/Microsoft.VC*.CRT/msvcp140.dll"),
                          key=lambda path: tuple(int(part) for part in path.parents[2].name.split(".")))
        if not runtimes:
            raise RuntimeError("Visual Studio x64 redistributable DLLs were not found")
        for dll in runtimes[-1].parent.glob("*.dll"):
            shutil.copy2(dll, package / dll.name)
        translations = package / "translations"
        translations.mkdir(exist_ok=True)
        for source in sorted((ROOT / "src/translations").glob("softpro_*.ts")):
            run([qt / "bin/lrelease.exe", source, "-qm", translations / (source.stem + ".qm")], build, environment)
        shutil.copy2(ROOT / "LICENSE", package / "LICENSE")
        shutil.copy2(ROOT / "README.md", package / "README.md")
        shutil.copytree(ROOT / "help", package / "help")
        notices = package / "licenses"
        notices.mkdir()
        for license_id in ("LGPL-2.1-or-later", "LGPL-3.0-only"):
            url = f"https://raw.githubusercontent.com/spdx/license-list-data/main/text/{license_id}.txt"
            with urllib.request.urlopen(url, timeout=30) as response:
                (notices / (license_id + ".txt")).write_bytes(response.read())
        if (qt / "sbom").is_dir():
            for metadata in (qt / "sbom").glob("*.spdx.json"):
                shutil.copy2(metadata, notices / metadata.name)
        (package / "THIRD-PARTY-NOTICES.txt").write_text(
            f"Qt {qt_version}: https://www.qt.io/licensing/ and https://download.qt.io/archive/qt/\n"
            "Qt licensing and third-party component details are provided in licenses/*.spdx.json.\n"
            "Qt Multimedia includes FFmpeg: https://ffmpeg.org/legal.html (LGPL 2.1 or later).\n"
            "FFmpeg sources: https://ffmpeg.org/releases/\n"
            "Microsoft Visual C++ runtime DLLs are included from Visual Studio's redistributable directory.\n"
            "Application license: see LICENSE.\n", encoding="utf-8")
        (package / "START-HERE.txt").write_text(
            f"SoftProjector {label} - Windows x64 tester build\n\n"
            "Extract this entire folder to a writable location, then run SoftProjector.exe.\n"
            "No installation is required. Keep the bundled DLL and plugin folders beside the executable.\n"
            "No Bible/song modules or personal data are included. Default settings and a new database\n"
            "are created on first launch. Use Manage Database (Ctrl+M) to add your test modules.\n\n"
            "Please test: opening Settings, selecting three Bible translations, projector displays,\n"
            "video playback, stream placement/backgrounds, and exiting while outputs are active.\n"
            "Stream output starts disabled. Enable it in Settings > Stream, click Apply, and use\n"
            "the supplied URL as an OBS Browser Source with the matching 1080p or 4K size.\n\n"
            f"Report problems at {repository}/issues with this version, Windows version,\n"
            "display arrangement, steps to reproduce, and screenshots if useful.\n", encoding="utf-8")
        (package / "BUILD-INFO.txt").write_text(
            f"Version: {version}\nGit commit: {commit}\nQt: {qt_version}\n"
            f"Architecture: Windows x64\nConfiguration: Release\nSource: {repository}/tree/{commit}\n"
            "Database and installed modules: none; created with defaults at first launch\n", encoding="utf-8")
        validate_contents(package)
        smoke_test(package, work, label)
        validate_contents(package)
        output.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as bundle:
            for file in sorted(package.rglob("*")):
                if file.is_file():
                    bundle.write(file, file.relative_to(work).as_posix())
        with archive.open("rb") as stream:
            digest = hashlib.file_digest(stream, "sha256").hexdigest()
        archive.with_suffix(".zip.sha256").write_text(f"{digest}  {archive.name}\n", encoding="utf-8")
        print(f"Created: {archive}\nSHA-256: {digest}", flush=True)


if __name__ == "__main__":
    main()
