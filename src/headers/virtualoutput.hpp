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

#include <QObject>
#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QSet>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMediaPlayer>
#include <QUrl>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include "imagegenerator.hpp"
#include "settings.hpp"
#include "bible.hpp"
#include "song.hpp"
#include "announcement.hpp"
#include "videoinfo.hpp"

class VirtualOutput : public QObject
{
    Q_OBJECT

public:
    enum ResolutionPreset
    {
        RES_720P,
        RES_1080P,
        RES_1440P,
        RES_CUSTOM
    };

    explicit VirtualOutput(QObject *parent = nullptr);
    ~VirtualOutput();

    bool initialize();
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    void setResolution(ResolutionPreset preset);
    void setCustomResolution(int width, int height);
    QSize getResolution() const { return m_resolution; }

    void setTheme(bool mirrorDisplay1, int themeId = -1);
    bool isMirroringDisplay1() const { return m_mirrorDisplay1; }
    int getThemeId() const { return m_themeId; }

    void setLogoOverlay(const QString &imagePath);
    void setLowerThirdConfig(bool show, const QString &text, const QFont &font,
                             const QColor &bgColor, const QColor &textColor);

    void updateDisplay();
    void renderNotText();

    static quint16 httpPort();
    static quint16 websocketPort();
    static QString browserUrl();

public slots:
    void renderPassiveText(QPixmap &background, bool useBackground, TextSettings &pSets);
    void renderBibleText(Verse verse, BibleSettings &settings);
    void renderSongText(Stanza stanza, SongSettings &settings);
    void renderAnnounceText(AnnounceSlide announce, TextSettings &settings);
    void renderSlideShow(QPixmap slide, SlideShowSettings &settings);
    void renderVideo(VideoInfo videoDetails);

    void setBackgroundVideo(const QString &path, bool loop, int fillMode);
    void stopBackgroundVideo();
    void pauseBackgroundVideo();
    void resumeBackgroundVideo();

    void playVideo();
    void pauseVideo();
    void stopVideo();
    void setVideoVolume(int level);
    void setVideoMuted(bool muted);
    void setVideoPosition(qint64 position);

    void positionControls(DisplayControlsSettings &settings);
    void setControlsVisible(bool visible);

signals:
    void enabledChanged(bool enabled);
    void resolutionChanged(QSize newResolution);
    void themeChanged(bool isMirror, int themeId);

    void exitSlide();
    void nextSlide();
    void prevSlide();

    void videoPositionChanged(qint64 position);
    void videoDurationChanged(qint64 duration);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void videoPlaybackStateChanged(QMediaPlayer::PlaybackState state);
#else
    void videoPlaybackStateChanged(QMediaPlayer::State state);
#endif
    void videoStopped();

protected:
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void onNewHttpConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onNewWebSocketConnection();
    void onWebSocketDisconnected();

private:
    struct AssetState
    {
        QByteArray data;
        QString contentType;
        quint64 version;
        bool available;

        AssetState() : version(0), available(false) {}
    };

    bool startServers();
    void stopServers();
    void resetHttpSocket(QTcpSocket *socket);
    void handleHttpRequest(QTcpSocket *socket, const QByteArray &requestData);
    void sendHttpResponse(QTcpSocket *socket, const QByteArray &statusLine,
                          const QByteArray &contentType, const QByteArray &body,
                          const QList<QByteArray> &extraHeaders = QList<QByteArray>(),
                          bool sendBody = true, qint64 contentLength = -1);
    void sendImageResponse(QTcpSocket *socket, const AssetState &asset, bool sendBody);
    void sendMediaResponse(QTcpSocket *socket, const QString &filePath, bool sendBody,
                           const QByteArray &rangeHeader = QByteArray());
    void sendNotFound(QTcpSocket *socket);
    void sendMethodNotAllowed(QTcpSocket *socket);
    void sendBadRequest(QTcpSocket *socket);

    void broadcastState();
    void sendState(QWebSocket *socket);
    QByteArray buildStateMessage() const;

    void setTransition(int transitionType);
    void setBackPixmap(const QPixmap &pixmap, int fillMode);
    void setTextPixmap(const QPixmap &pixmap);
    void clearMainVideo();
    void updateOverlayAsset();

    QByteArray pixmapToPng(const QPixmap &pixmap) const;
    QString contentTypeForFile(const QString &path) const;
    QString localFilePath(const QUrl &url) const;

    bool m_enabled;
    bool m_initialized;
    bool m_mirrorDisplay1;
    int m_themeId;
    ResolutionPreset m_resolutionPreset;
    QSize m_resolution;

    QString m_logoImagePath;
    QTcpServer *m_httpServer;
    QWebSocketServer *m_webSocketServer;
    QSet<QWebSocket*> m_clients;
    QSet<QTcpSocket*> m_httpSockets;
    ImageGenerator m_imageGenerator;

    AssetState m_backgroundImage;
    AssetState m_textImage;
    AssetState m_overlayImage;
    quint64 m_mediaVersion;
    int m_transitionType;

    QString m_backgroundVideoPath;
    bool m_backgroundVideoLoop;
    int m_backgroundVideoFillMode;
    bool m_backgroundVideoPaused;

    QString m_mainVideoPath;
    bool m_mainVideoPaused;
    qint64 m_mainVideoPosition;
    int m_mainVideoVolume;
    bool m_mainVideoMuted;
    QColor m_color;
};

#endif // VIRTUALOUTPUT_HPP
