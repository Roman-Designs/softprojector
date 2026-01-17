/***************************************************************************
//
//    softProjector - an open source media projection software
//    Copyright (C) 2017  Vladislav Kobzar
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation version 3 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
***************************************************************************/

#include "../headers/virtualoutput.hpp"
#include <QQmlEngine>
#include <QQmlContext>
#include <QDebug>

VirtualOutput::VirtualOutput(QObject *parent)
    : QObject(parent),
      m_enabled(false),
      m_mirrorDisplay1(true),
      m_themeId(-1),
      m_resolutionPreset(RES_1080P),
      m_resolution(1920, 1080),
      m_lowerThirdEnabled(false),
      m_window(nullptr),
      m_imageProvider(nullptr),
      m_backImSwitch1(false),
      m_backImSwitch2(false),
      m_textImSwitch1(false),
      m_textImSwitch2(false),
      m_isNewBackground(false),
      m_back1to2(true),
      m_text1to2(true),
      m_backType(B_NONE)
{
    m_currentColor.setRgb(0, 0, 0, 0);

    // Set default lower third styling
    m_lowerThirdFont = QFont("Arial", 24);
    m_lowerThirdBgColor = QColor(0, 0, 0, 200); // Semi-transparent black
    m_lowerThirdTextColor = QColor(255, 255, 255); // White
}

VirtualOutput::~VirtualOutput()
{
    if (m_window)
    {
        delete m_window;
        m_window = nullptr;
    }

    m_imageProvider = nullptr; // Will be deleted by QML engine
}

bool VirtualOutput::initialize()
{
    try
    {
        createWindow();
        loadQmlView();
        connectSignals();

        qDebug() << "VirtualOutput initialized successfully with resolution:"
                 << m_resolution.width() << "x" << m_resolution.height();
        return true;
    }
    catch (const std::exception &e)
    {
        qWarning() << "Failed to initialize VirtualOutput:" << e.what();
        return false;
    }
}

void VirtualOutput::createWindow()
{
    if (m_window)
    {
        return; // Already created
    }

    m_window = new QQuickView;

    // Set window properties for capturing by OBS
    m_window->setTitle("SoftProjector Virtual Output");
    m_window->setGeometry(0, 0, m_resolution.width(), m_resolution.height());
    m_window->setMinimumSize(m_resolution);
    m_window->setMaximumSize(m_resolution);

    // Create and register image provider
    m_imageProvider = new SpImageProvider;
    m_window->engine()->addImageProvider(QLatin1String("improvider"), m_imageProvider);

    // Set black background by default
    m_window->setColor(QColor(0, 0, 0));
}

void VirtualOutput::loadQmlView()
{
    if (!m_window)
    {
        throw std::runtime_error("Window not created");
    }

    // Load the VirtualDisplayArea.qml file
    // This file will be created in Phase 1.2
    m_window->setSource(QUrl("qrc:/qml/qml/VirtualDisplayArea.qml"));

    if (m_window->status() == QQuickView::Error)
    {
        QStringList errors;
        for (const auto &error : m_window->errors())
        {
            errors << error.toString();
        }
        throw std::runtime_error("Failed to load VirtualDisplayArea.qml: " + errors.join("; ").toStdString());
    }
}

void VirtualOutput::connectSignals()
{
    if (!m_window)
    {
        throw std::runtime_error("Window not created");
    }

    QObject *rootObject = m_window->rootObject();
    if (!rootObject)
    {
        throw std::runtime_error("Failed to get QML root object");
    }

    // Connect display control signals from QML
    connect(rootObject, SIGNAL(exitClicked()), this, SLOT(exitSlideClicked()));
    connect(rootObject, SIGNAL(nextClicked()), this, SLOT(nextSlideClicked()));
    connect(rootObject, SIGNAL(prevClicked()), this, SLOT(prevSlideClicked()));

    // Connect video player signals from QML
    connect(rootObject, SIGNAL(positionChanged(int)), this, SLOT(videoPositionChanged(int)));
    connect(rootObject, SIGNAL(durationChanged(int)), this, SLOT(videoDurationChanged(int)));
    connect(rootObject, SIGNAL(playbackStateChanged(int)), this, SLOT(videoPlaybackStateChanged(int)));
    connect(rootObject, SIGNAL(playbackStopped()), this, SLOT(playbackStopped()));

    // Connect our rendering slots
    connect(this, SIGNAL(updateDisplay()), this, SLOT(updateDisplay()), Qt::QueuedConnection);
}

void VirtualOutput::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
    {
        return;
    }

    m_enabled = enabled;

    if (m_enabled)
    {
        if (!m_window)
        {
            initialize();
        }
        m_window->show();
    }
    else
    {
        if (m_window)
        {
            m_window->hide();
        }
    }

    emit enabledChanged(m_enabled);
}

void VirtualOutput::setResolution(ResolutionPreset preset)
{
    m_resolutionPreset = preset;

    switch (preset)
    {
    case RES_720P:
        m_resolution = QSize(1280, 720);
        break;
    case RES_1080P:
        m_resolution = QSize(1920, 1080);
        break;
    case RES_1440P:
        m_resolution = QSize(2560, 1440);
        break;
    case RES_CUSTOM:
        // Resolution should be set via setCustomResolution
        break;
    }

    updateWindowSize();
    m_imageGenerator.setScreenSize(m_resolution);

    emit resolutionChanged(m_resolution);
}

void VirtualOutput::setCustomResolution(int width, int height)
{
    m_resolutionPreset = RES_CUSTOM;
    m_resolution = QSize(width, height);

    updateWindowSize();
    m_imageGenerator.setScreenSize(m_resolution);

    emit resolutionChanged(m_resolution);
}

void VirtualOutput::setTheme(bool mirrorDisplay1, int themeId)
{
    m_mirrorDisplay1 = mirrorDisplay1;
    m_themeId = themeId;

    emit themeChanged(m_mirrorDisplay1, m_themeId);

    qDebug() << "VirtualOutput theme changed - Mirror Display 1:" << m_mirrorDisplay1
             << "Theme ID:" << m_themeId;
}

void VirtualOutput::setLogoOverlay(const QString &imagePath)
{
    m_logoImagePath = imagePath;

    if (m_window && m_window->rootObject())
    {
        QObject *rootObject = m_window->rootObject();
        QMetaObject::invokeMethod(rootObject, "setLogoOverlay", Q_ARG(QString, imagePath));
    }

    qDebug() << "VirtualOutput logo overlay set to:" << imagePath;
}

void VirtualOutput::setLowerThirdConfig(bool show, const QString &text, const QFont &font,
                                        const QColor &bgColor, const QColor &textColor)
{
    m_lowerThirdEnabled = show;
    m_lowerThirdText = text;
    m_lowerThirdFont = font;
    m_lowerThirdBgColor = bgColor;
    m_lowerThirdTextColor = textColor;

    if (m_window && m_window->rootObject())
    {
        QObject *rootObject = m_window->rootObject();
        QMetaObject::invokeMethod(rootObject, "setLowerThirdConfig",
                                  Q_ARG(bool, show),
                                  Q_ARG(QString, text),
                                  Q_ARG(QFont, font),
                                  Q_ARG(QColor, bgColor),
                                  Q_ARG(QColor, textColor));
    }
}

void VirtualOutput::showLowerThird(const QString &text)
{
    m_lowerThirdText = text;
    m_lowerThirdEnabled = true;

    if (m_window && m_window->rootObject())
    {
        QObject *rootObject = m_window->rootObject();
        QMetaObject::invokeMethod(rootObject, "showLowerThird", Q_ARG(QString, text));
    }

    qDebug() << "VirtualOutput lower third shown:" << text;
}

void VirtualOutput::hideLowerThird()
{
    m_lowerThirdEnabled = false;

    if (m_window && m_window->rootObject())
    {
        QObject *rootObject = m_window->rootObject();
        QMetaObject::invokeMethod(rootObject, "hideLowerThird");
    }

    qDebug() << "VirtualOutput lower third hidden";
}

void VirtualOutput::updateDisplay()
{
    if (!m_enabled || !m_window)
    {
        return;
    }

    // This is called to refresh the display
    // The actual rendering happens through the render*Text slots
}

void VirtualOutput::renderNotText()
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // Clear the display (render black)
    QPixmap blackPixmap(m_resolution);
    blackPixmap.fill(Qt::black);
    m_imageProvider->setPixMap(blackPixmap);
}

void VirtualOutput::renderPassiveText(QPixmap &background, bool useBackground, TextSettings &pSets)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    if(pSets.backgroundType == B_VIDEO && !pSets.backgroundVideoPath.isEmpty())
    {
        setBackgroundVideo(pSets.backgroundVideoPath, pSets.backgroundVideoLoop, pSets.backgroundVideoFillMode);
        m_backType = B_VIDEO;
    }
    else
    {
        stopBackgroundVideo();
        if (useBackground && !background.isNull())
        {
            setBackPixmap(background, 1); // 1 = keep aspect ratio
            m_backType = B_PICTURE;
        }
        else
        {
            setBackPixmap(background, m_currentColor);
            m_backType = B_NONE;
        }
    }
}

void VirtualOutput::renderBibleText(Verse verse, BibleSettings &settings)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // TODO: Phase 1.2+ - Render Bible verse text with settings
    // This will be implemented after VirtualDisplayArea.qml is created
    qDebug() << "VirtualOutput rendering Bible text:" << verse.primary_caption;
}

void VirtualOutput::renderSongText(Stanza stanza, SongSettings &settings)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // TODO: Phase 1.2+ - Render song stanza with settings
    // This will be implemented after VirtualDisplayArea.qml is created
    qDebug() << "VirtualOutput rendering song text";
}

void VirtualOutput::renderAnnounceText(AnnounceSlide announce, TextSettings &settings)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // TODO: Phase 1.2+ - Render announcement with settings
    // This will be implemented after VirtualDisplayArea.qml is created
    qDebug() << "VirtualOutput rendering announcement text";
}

void VirtualOutput::renderSlideShow(QPixmap slide, SlideShowSettings &settings)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // TODO: Phase 1.2+ - Render slideshow
    // This will be implemented after VirtualDisplayArea.qml is created
    qDebug() << "VirtualOutput rendering slideshow slide";
}

void VirtualOutput::renderVideo(VideoInfo videoDetails)
{
    if (!m_enabled || !m_window || !m_imageProvider)
    {
        return;
    }

    // TODO: Phase 1.2+ - Set video playback
    // This will be implemented after VirtualDisplayArea.qml is created
    qDebug() << "VirtualOutput rendering video:" << videoDetails.fileName;
}

void VirtualOutput::playVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "playVideo");
}

void VirtualOutput::pauseVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "pauseVideo");
}

void VirtualOutput::stopVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "stopVideo");
}

void VirtualOutput::setVideoVolume(int level)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setVideoVolume", Q_ARG(int, level));
}

void VirtualOutput::setVideoMuted(bool muted)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setVideoMuted", Q_ARG(bool, muted));
}

void VirtualOutput::setVideoPosition(qint64 position)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setVideoPosition", Q_ARG(qint64, position));
}

void VirtualOutput::positionControls(DisplayControlsSettings &settings)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    // TODO: Phase 1.2+ - Position display controls
    qDebug() << "VirtualOutput positioning controls";
}

void VirtualOutput::setControlsVisible(bool visible)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setControlsVisible", Q_ARG(bool, visible));
}

void VirtualOutput::updateWindowSize()
{
    if (m_window)
    {
        m_window->setGeometry(0, 0, m_resolution.width(), m_resolution.height());
        m_window->setMinimumSize(m_resolution);
        m_window->setMaximumSize(m_resolution);
    }
}

void VirtualOutput::setBackPixmap(QPixmap pixmap, int fillMode)
{
    if (!m_imageProvider)
    {
        return;
    }

    // Check if pixmap has changed
    if (m_currentBackground.cacheKey() == pixmap.cacheKey())
    {
        m_isNewBackground = false;
        return;
    }

    m_currentBackground = pixmap;
    m_isNewBackground = true;

    // Apply fill mode scaling
    switch (fillMode)
    {
    case 0:
        pixmap = pixmap.scaled(m_resolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    case 1:
        pixmap = pixmap.scaled(m_resolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case 2:
        pixmap = pixmap.scaled(m_resolution, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        break;
    default:
        // No scaling
        break;
    }

    m_imageProvider->setPixMap(pixmap);
    m_back1to2 = (!m_back1to2);

    // Update the QML image display
    if (m_window && m_window->rootObject())
    {
        QObject *backImage1 = m_window->rootObject()->findChild<QObject *>("backImage1");
        QObject *backImage2 = m_window->rootObject()->findChild<QObject *>("backImage2");

        if (backImage1 && backImage2)
        {
            if (m_back1to2)
            {
                backImage2->setProperty("visible", true);
                backImage1->setProperty("visible", false);
            }
            else
            {
                backImage1->setProperty("visible", true);
                backImage2->setProperty("visible", false);
            }
        }
    }
}

void VirtualOutput::setBackPixmap(QPixmap pixmap, QColor color)
{
    setBackPixmap(pixmap, 0); // Use stretch fill mode
}

void VirtualOutput::setTextPixmap(QPixmap pixmap)
{
    if (!m_imageProvider)
    {
        return;
    }

    m_imageProvider->setPixMap(pixmap);
    m_text1to2 = (!m_text1to2);

    // Update the QML text display
    if (m_window && m_window->rootObject())
    {
        QObject *textImage1 = m_window->rootObject()->findChild<QObject *>("textImage1");
        QObject *textImage2 = m_window->rootObject()->findChild<QObject *>("textImage2");

        if (textImage1 && textImage2)
        {
            if (m_text1to2)
            {
                textImage2->setProperty("visible", true);
                textImage1->setProperty("visible", false);
            }
            else
            {
                textImage1->setProperty("visible", true);
                textImage2->setProperty("visible", false);
            }
        }
    }
}

void VirtualOutput::setBackVideo(QString path)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setBackgroundVideo", Q_ARG(QString, path));
}

void VirtualOutput::setVideoSource(QObject *playerObject, QUrl path)
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QObject *player = m_window->rootObject()->findChild<QObject *>("player");
    if (player)
    {
        player->setProperty("source", path);
    }
}

void VirtualOutput::setBackgroundVideo(const QString &path, bool loop, int fillMode)
{
    if(path == m_currentBackgroundVideoPath)
        return;

    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "setBackgroundVideo",
                              Q_ARG(QVariant, QVariant(path)),
                              Q_ARG(QVariant, QVariant(loop)),
                              Q_ARG(QVariant, QVariant(fillMode)));
    m_currentBackgroundVideoPath = path;
}

void VirtualOutput::stopBackgroundVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "stopBackgroundVideo");
    m_currentBackgroundVideoPath.clear();
}

void VirtualOutput::pauseBackgroundVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "pauseBackgroundVideo");
}

void VirtualOutput::resumeBackgroundVideo()
{
    if (!m_window || !m_window->rootObject())
    {
        return;
    }

    QMetaObject::invokeMethod(m_window->rootObject(), "resumeBackgroundVideo");
}

void VirtualOutput::exitSlideClicked()
{
    emit exitSlide();
}

void VirtualOutput::nextSlideClicked()
{
    emit nextSlide();
}

void VirtualOutput::prevSlideClicked()
{
    emit prevSlide();
}

void VirtualOutput::videoPositionChanged(int position)
{
    emit videoPositionChanged(static_cast<qint64>(position));
}

void VirtualOutput::videoDurationChanged(int duration)
{
    emit videoDurationChanged(static_cast<qint64>(duration));
}

void VirtualOutput::videoPlaybackStateChanged(int state)
{
    emit videoPlaybackStateChanged(static_cast<QMediaPlayer::PlaybackState>(state));
}

void VirtualOutput::playbackStopped()
{
    emit videoStopped();
}

void VirtualOutput::keyReleaseEvent(QKeyEvent *event)
{
    // This will be implemented in Phase 1.2+ when VirtualDisplayArea.qml is created
    // For now, this is a placeholder for key handling
    Q_UNUSED(event);
}
