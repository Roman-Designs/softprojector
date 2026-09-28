# SoftProjector

Qt 6 desktop software for projecting Bible verses, songs, slideshows, announcements, and media.

Build with Qt 6 (Widgets, Quick/QML, SQL/SQLite, Multimedia, and Print Support) and a C++17 compiler:

```sh
qmake6 src/softProjector.pro
make -j4
```

The executable is placed under `src/mac_build/bin` on macOS, `src/unix_build/bin` on Linux, or `src/win32_build/bin` on Windows. For a macOS fresh-database smoke check:

```sh
python3 tests/smoke.py src/mac_build/bin/SoftProjector.app/Contents/MacOS/SoftProjector
```

SoftProjector uses an existing writable `spData.sqlite` beside the executable. Otherwise it creates the database in the platform's application-data directory, copying an existing read-only database there when necessary. To package a Windows build, run `windeployqt --qmldir src` against the executable and include the SQLite driver and translation files.
