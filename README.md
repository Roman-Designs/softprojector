# softProjector

A free, open-source worship presentation software for churches and religious organizations. Display Bible verses, songs/hymns, announcements, pictures/slideshows, and videos on projector screens during worship services.

**Author:** Vladislav Kobzar  
**License:** GNU General Public License v3 (GPL-3.0)

## Technology Stack

| Category | Technology |
|----------|------------|
| **Language** | C++ |
| **Framework** | Qt 5/6 |
| **UI Toolkit** | Qt Widgets + Qt Quick/QML |
| **Build System** | qmake |
| **Database** | SQLite |
| **Multimedia** | Qt Multimedia |

### Qt Modules Used

- `QtCore` - Core functionality
- `QtGui` - GUI components
- `QtWidgets` - Widget-based UI
- `QtNetwork` - Network operations (module downloads)
- `QtSql` - SQLite database access
- `QtQml` / `QtQuick` - QML-based display rendering
- `QtPrintSupport` - Printing capabilities
- `QtMultimedia` / `QtMultimediaWidgets` - Video/audio playback

## Features

- **Bible Display** - Multiple versions, dual Bible view, import Unbound Bibles
- **Songs/Hymns** - Multiple songbooks, song editing, import/export, usage tracking
- **Announcements** - Custom text with styling options
- **Pictures/Slideshows** - Image slideshow creation and display
- **Video Playback** - Media library with video support
- **Multi-Monitor** - Support for up to 4 displays
- **Themes** - Customizable visual themes with fade transitions, shadows, and blur effects
- **Schedule Management** - Service planning and printing
- **Dark Theme** - Windows dark mode detection

## Project Structure

```
softprojector/
|
+-- .claude/
|   +-- agents/                    # AI agent definitions for development tasks
|
+-- 3rdparty/
|   +-- headers/                   # Third-party headers (QMediaPlaylist)
|   +-- sources/                   # Third-party sources (playlist parsing)
|
+-- help/                          # User documentation (HTML)
|   +-- images/                    # Documentation images
|   +-- *.html                     # Multi-language help files (EN, DE, RU)
|
+-- iconSource/                    # Original SVG icon sources
|
+-- src/                           # Main source code
|   +-- headers/                   # C++ header files (.hpp)
|   +-- sources/                   # C++ implementation files (.cpp)
|   +-- ui/                        # Qt Designer UI files (.ui)
|   +-- icons/                     # Application icons (PNG)
|   +-- qml/                       # QML files for display rendering
|   +-- translations/              # i18n files (.ts, .qm)
|   +-- softProjector.pro          # qmake project file
|   +-- softprojector.qrc          # Qt resource file
|   +-- softprojector.rc           # Windows resource file
|
+-- README.md
+-- LICENSE
```

## Key Source Files

### Core Application

| File | Purpose |
|------|---------|
| `main.cpp` | Application entry point, database initialization, dark theme |
| `softprojector.cpp/hpp` | Main window, orchestrates all features |
| `settings.cpp/hpp` | Application settings management |
| `theme.cpp/hpp` | Visual theme/styling configuration |

### Bible Module

| File | Purpose |
|------|---------|
| `bible.cpp/hpp` | Bible data model and operations |
| `biblewidget.cpp/hpp` | Bible search and display UI |
| `biblesettingwidget.cpp/hpp` | Bible appearance settings |

### Songs/Hymns Module

| File | Purpose |
|------|---------|
| `song.cpp/hpp` | Song data model and database operations |
| `songwidget.cpp/hpp` | Song search and display UI |
| `songsettingwidget.cpp/hpp` | Song appearance settings |
| `editwidget.cpp/hpp` | Song editor |

### Display/Projection System

| File | Purpose |
|------|---------|
| `projectordisplayscreen.cpp/hpp` | Projector output window management |
| `displayscreen.cpp/hpp` | Display rendering (core display logic) |
| `imagegenerator.cpp/hpp` | Image/slide generation for display |
| `spimageprovider.cpp/hpp` | QML image provider |
| `DisplayArea.qml` | QML-based display area (video, images, text) |

### Media Features

| File | Purpose |
|------|---------|
| `picturewidget.cpp/hpp` | Picture/slideshow browsing |
| `slideshow.cpp/hpp` | Slideshow data model |
| `mediawidget.cpp/hpp` | Media library UI |
| `videoplayerwidget.cpp/hpp` | Video player |

### Data Management

| File | Purpose |
|------|---------|
| `managedatadialog.cpp/hpp` | Database import/export dialog |
| `managedata.cpp/hpp` | Data import/export operations |
| `schedule.cpp/hpp` | Service schedule/playlist management |

## Database Schema

The application uses SQLite (`spData.sqlite`) with these main tables:

- **Songs** - Hymns/songs with lyrics, metadata, and styling
- **Songbooks** - Songbook collections
- **BibleVersions** - Bible translation metadata
- **BibleBooks** - Books within each Bible
- **BibleVerse** - Individual verses
- **Announcements** - Custom text announcements
- **SlideShows** - Slideshow metadata
- **Slides** - Individual slides with images
- **Settings** - Application settings
- **Themes** - Visual themes and configurations

## Internationalization

Supported languages:
- English (default)
- German (`softpro_de.ts`)
- Russian (`softpro_ru.ts`)
- Ukrainian (`softpro_ua.ts`)
- Czech (`softpro_cs.ts`)
- Armenian (`softpro_hy.ts`)

## Building the Project

### Prerequisites

- Qt 5.15+ or Qt 6.x
- C++17 compatible compiler
- qmake

### Build Steps

1. Open `src/softProjector.pro` in Qt Creator or your IDE
2. Build the project
3. Copy the executable to a deployment folder
4. Run windeployqt to gather dependencies:

```bash
windeployqt.exe --qmldir "<ProjectDirectory>/src/qml" "<PathToEXE>"
```

### Platform-Specific Build Directories

- Windows: `win32_build/`
- macOS: `mac_build/`
- Unix/Linux: `unix_build/`

## AI Agent Delegation

**IMPORTANT FOR AI MODELS:** When working on this project, you should select and delegate tasks to the specialized agents defined in `.claude/agents/`. This ensures consistent, high-quality work aligned with project standards.

### Available Agents

#### Development Agents
| Agent | File | Use For |
|-------|------|---------|
| Fullstack Developer | `fullstack-developer.md` | End-to-end feature development |
| Backend Developer | `backend-developer.md` | Database, data models, core logic |
| Frontend Developer | `frontend-developer.md` | UI/UX implementation, Qt Widgets |
| Mobile Developer | `mobile-developer.md` | Mobile platform considerations |
| Electron Pro | `electron-pro.md` | Desktop app packaging expertise |

#### Quality & Testing Agents
| Agent | File | Use For |
|-------|------|---------|
| QA Expert | `qa-expert.md` | Testing strategies, test cases |
| Code Reviewer | `code-reviewer.md` | Code review, quality standards |
| Error Detective | `error-detective.md` | Bug detection, debugging |
| Accessibility Tester | `accessibility-tester.md` | Accessibility compliance |

#### Design & UX Agents
| Agent | File | Use For |
|-------|------|---------|
| UI Designer | `ui-designer.md` | Interface design, visual polish |
| UX Researcher | `ux-researcher.md` | User experience improvements |

#### Documentation & Content Agents
| Agent | File | Use For |
|-------|------|---------|
| Technical Writer | `technical-writer.md` | Documentation, help files |
| Content Marketer | `content-marketer.md` | Marketing content |

#### Business & Analysis Agents
| Agent | File | Use For |
|-------|------|---------|
| Product Manager | `product-manager.md` | Feature prioritization, roadmap |
| Business Analyst | `business-analyst.md` | Requirements analysis |
| Research Analyst | `research-analyst.md` | General research tasks |
| Competitive Analyst | `competitive-analyst.md` | Competitor analysis |
| Market Researcher | `market-researcher.md` | Market research |
| Data Researcher | `data-researcher.md` | Data analysis |
| Trend Analyst | `trend-analyst.md` | Industry trends |

#### Other Agents
| Agent | File | Use For |
|-------|------|---------|
| Sales Engineer | `sales-engineer.md` | Technical sales support |
| Customer Success Manager | `customer-success-manager.md` | User success strategies |
| Legal Advisor | `legal-advisor.md` | Legal considerations, licensing |
| Search Specialist | `search-specialist.md` | Search optimization |

### How to Delegate

When performing development work:

1. Identify the type of task (development, testing, documentation, etc.)
2. Read the appropriate agent file from `.claude/agents/`
3. Follow the agent's guidelines and standards
4. Delegate subtasks to specialized agents as needed

Example workflow:
- New feature: Start with `product-manager.md` for requirements, then `fullstack-developer.md` for implementation
- Bug fix: Use `error-detective.md` to diagnose, `backend-developer.md` or `frontend-developer.md` to fix
- Code changes: Always finish with `code-reviewer.md` for quality checks

## Contributing

1. Fork the repository
2. Create a feature branch
3. Follow the coding standards (Qt/C++ conventions)
4. Ensure all translations are updated if adding user-facing strings
5. Test on multiple platforms if possible
6. Submit a pull request

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
