# SoftProjector

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
