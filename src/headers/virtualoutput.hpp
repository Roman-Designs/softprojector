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

#ifndef VIRTUALOUTPUT_HPP
#define VIRTUALOUTPUT_HPP

#include <QWindow>
#include <QQuickView>
#include <QQuickItem>
#include <QtQml>
#include <QMediaPlayer>
#include <QSize>
#include <QString>
#include <QColor>
#include "spimageprovider.hpp"
#include "imagegenerator.hpp"
#include "settings.hpp"
#include "bible.hpp"
#include "song.hpp"
#include "announcement.hpp"
#include "videoinfo.hpp"

/**
 * @brief The VirtualOutput class manages a separate streaming/output window
 *
 * This class creates and manages a virtual output window that can be captured
 * by streaming software like OBS. It supports:
 * - Configurable resolution (720p, 1080p, custom)
 * - Independent theme or mirror of Display 1
 * - Overlay layers (logo, lower third graphics)
 * - QML-based rendering for consistency with main display
 */
class VirtualOutput : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Resolution presets for virtual output
     */
    enum ResolutionPreset
    {
        RES_720P,      // 1280x720
        RES_1080P,     // 1920x1080
        RES_1440P,     // 2560x1440
        RES_CUSTOM     // User-defined
    };

    explicit VirtualOutput(QObject *parent = nullptr);
    ~VirtualOutput();

    // Initialization and control
    bool initialize();
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    // Resolution configuration
    void setResolution(ResolutionPreset preset);
    void setCustomResolution(int width, int height);
    QSize getResolution() const { return m_resolution; }

    // Theme configuration
    void setTheme(bool mirrorDisplay1, int themeId = -1);
    bool isMirroringDisplay1() const { return m_mirrorDisplay1; }
    int getThemeId() const { return m_themeId; }

    // Overlay configuration
    void setLogoOverlay(const QString &imagePath);
    void setLowerThirdConfig(bool show, const QString &text, const QFont &font,
                            const QColor &bgColor, const QColor &textColor);

    // Display update methods
    void updateDisplay();
    void renderNotText();

public slots:
    // Render content from main display
    void renderPassiveText(QPixmap &background, bool useBackground, TextSettings &pSets);
    void renderBibleText(Verse verse, BibleSettings &settings);
    void renderSongText(Stanza stanza, SongSettings &settings);
    void renderAnnounceText(AnnounceSlide announce, TextSettings &settings);
    void renderSlideShow(QPixmap slide, SlideShowSettings &settings);
    void renderVideo(VideoInfo videoDetails);

    // Video background control
    void setBackgroundVideo(const QString &path, bool loop, int fillMode);
    void stopBackgroundVideo();
    void pauseBackgroundVideo();
    void resumeBackgroundVideo();

    // Video playback control
    void playVideo();
    void pauseVideo();
    void stopVideo();
    void setVideoVolume(int level);
    void setVideoMuted(bool muted);
    void setVideoPosition(qint64 position);

    // Display control positioning
    void positionControls(DisplayControlsSettings &settings);
    void setControlsVisible(bool visible);

signals:
    // State change signals
    void enabledChanged(bool enabled);
    void resolutionChanged(QSize newResolution);
    void themeChanged(bool isMirror, int themeId);

    // Display control signals
    void exitSlide();
    void nextSlide();
    void prevSlide();

    // Video control signals
    void videoPositionChanged(qint64 position);
    void videoDurationChanged(qint64 duration);
    void videoPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void videoStopped();

protected:
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    // Internal QML rendering slots
    void setBackPixmap(QPixmap pixmap, int fillMode);
    void setBackPixmap(QPixmap pixmap, QColor color);
    void setTextPixmap(QPixmap pixmap);
    void setBackVideo(QString path);
    void setVideoSource(QObject *playerObject, QUrl path);

    // Control signals from QML
    void exitSlideClicked();
    void nextSlideClicked();
    void prevSlideClicked();

    // Video player signals
    void videoPositionChanged(int position);
    void videoDurationChanged(int duration);
    void videoPlaybackStateChanged(int state);
    void playbackStopped();

private:
    // Helper methods
    void createWindow();
    void loadQmlView();
    void connectSignals();
    void updateWindowSize();

    // Configuration members
    bool m_enabled;
    bool m_mirrorDisplay1;
    int m_themeId;
    ResolutionPreset m_resolutionPreset;
    QSize m_resolution;

    // Overlay configuration
    QString m_logoImagePath;

    // QML and rendering components
    QQuickView *m_window;
    SpImageProvider *m_imageProvider;
    ImageGenerator m_imageGenerator;

    // State tracking for alternating buffers (like ProjectorDisplayScreen)
    bool m_backImSwitch1, m_backImSwitch2;
    bool m_textImSwitch1, m_textImSwitch2;
    bool m_isNewBackground;
    bool m_back1to2, m_text1to2;
    QPixmap m_currentBackground;
    QColor m_currentColor;
    QString m_currentBackgroundVideoPath;
    int m_backType;
};

#endif // VIRTUALOUTPUT_HPP
