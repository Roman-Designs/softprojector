"""Run against a freshly built SoftProjector executable (Qt offscreen)."""

import os
import shutil
import sqlite3
import subprocess
import sys
import tempfile
import time
from pathlib import Path


with tempfile.TemporaryDirectory() as directory:
    executable = Path(directory) / "SoftProjector"
    shutil.copy2(sys.argv[1], executable)
    environment = dict(os.environ, HOME=directory, XDG_DATA_HOME=directory,
                       XDG_CONFIG_HOME=directory, QT_QPA_PLATFORM="offscreen",
                       QT_QUICK_BACKEND="software")
    process = subprocess.Popen([executable], env=environment, stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL)
    try:
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            if process.poll() is not None:
                raise AssertionError(f"SoftProjector exited during startup: {process.returncode}")
            databases = list(Path(directory).rglob("spData.sqlite"))
            if databases:
                database = databases[0]
                with sqlite3.connect(database) as connection:
                    try:
                        counts = [connection.execute(f"SELECT count(*) FROM {table}").fetchone()[0]
                                  for table in ("ThemePassive", "ThemeBible", "ThemeSong", "ThemeAnnounce")]
                        settings = connection.execute("SELECT count(*) FROM Settings").fetchone()[0]
                        announcement = connection.execute(
                            "SELECT text_font FROM ThemeAnnounce LIMIT 1").fetchone()
                    except sqlite3.OperationalError:
                        counts, settings, announcement = [], 0, None
                    if counts == [4, 4, 4, 4] and settings == 7 and announcement and announcement[0]:
                        break
            time.sleep(0.2)
        else:
            raise AssertionError(f"Startup did not create complete default themes and settings: "
                                 f"databases={databases}, counts={counts if databases else []}, "
                                 f"settings={settings if databases else 0}")
    finally:
        process.terminate()
        process.wait(timeout=5)

print("Fresh database and four complete default themes: OK")
