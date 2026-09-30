# SoftProjector

Current tester version: **2.3.0 Beta 1**.

Qt 6 desktop software for projecting Bible verses, songs, slideshows, announcements, and media.

Build with Qt 6 (Widgets, Quick/QML, SQL/SQLite, Multimedia, Print Support, and HttpServer) and a C++17 compiler:

```sh
qmake6 src/softProjector.pro
make -j4
```

The executable is placed under `src/mac_build/bin` on macOS, `src/unix_build/bin` on Linux, or `src/win32_build/bin` on Windows. For a macOS fresh-database smoke check:

```sh
python3 tests/smoke.py src/mac_build/bin/SoftProjector.app/Contents/MacOS/SoftProjector
```

SoftProjector uses an existing writable `spData.sqlite` beside the executable. Otherwise it creates the database in the platform's application-data directory, copying an existing read-only database there when necessary. To package a Windows build, run `windeployqt --qmldir src` against the executable and include the SQLite driver and translation files.

## OBS browser source

In **Settings → Stream**, enable the LAN output, choose a 1080p or 4K canvas and copy the URL into an OBS Browser Source. Set the source width and height to match the canvas. The stream has its own Bible translation and per-content layouts (Bible lower third, songs full canvas, photos picture-in-picture by default); positioning, fonts, colors, and transparency can be adjusted without changing the projector displays. Positions and font sizes scale with the selected canvas. Hide and Clear make the stream transparent. Video also stays transparent so OBS can handle video separately. The source is read-only and available to devices on the same network at the URL shown in Settings.

If OBS runs on a different computer, both devices must be on the same network and the operating system firewall must allow incoming connections to the selected port.

To add a stream background, choose a content type under **Settings → Stream → Appearance for**, select **Choose image…**, and enable **Use background image**. Choose **Fill content region** for a background behind the lower third or other positioned content, or **Fill entire canvas** for a full-screen background. Images are centered and cropped to fill, and are saved inside the stream settings so the original file can be moved afterward. The panel color overlays the background within the content region; use a transparent panel to show the image fully. Backgrounds are independent for Bible, songs, announcements, and photos. Hide and Clear still make the entire stream transparent.

For placement, choose a preset such as **Bottom banner**, **Full screen**, or **Small overlay**, then drag the box in the preview to move it or drag its bottom-right corner to resize. **Fine-tune placement** reveals exact percentages when needed. With the preview focused, arrow keys move the box and Shift + arrow keys resize it. Each content type keeps its own placement, which scales automatically between 1080p and 4K.

Exiting SoftProjector closes all projector windows, stops video playback, and shuts down the stream server and its open connections. Network browser sources clear their image when the server disconnects or fails to respond within approximately two seconds.

## Windows tester package

With Qt 6 and Visual Studio installed, create a clean Windows x64 release ZIP with:

```powershell
python tools/package_windows.py --qt-dir C:\Qt\6.8.3\msvc2022_64 --vs-devcmd "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
```

The script builds in a fresh temporary directory, deploys Qt/QML and the app-local C++ runtime, compiles application translations, verifies first launch and network shutdown using the packaged files, and writes the ZIP plus its SHA-256 checksum to `dist/`. The package includes no database, personal settings, or installed modules. Extract the entire folder to a writable location and run `SoftProjector.exe`; a fresh database with default settings is created on first launch.
