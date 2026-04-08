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

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMimeDatabase>
#include <QTextStream>
#include <QUrl>

namespace
{
const quint16 kVirtualOutputHttpPort = 15171;
const quint16 kVirtualOutputWebSocketPort = 15172;
const char *kVirtualOutputPage = ":/web/virtualoutput.html";
const char *kVirtualOutputScript = ":/web/virtualoutput.js";
const char *kVirtualOutputStyle = ":/web/virtualoutput.css";
}

VirtualOutput::VirtualOutput(QObject *parent)
    : QObject(parent),
      m_enabled(false),
      m_initialized(false),
      m_mirrorDisplay1(true),
      m_themeId(-1),
      m_resolutionPreset(RES_1080P),
      m_resolution(1920, 1080),
      m_httpServer(nullptr),
      m_webSocketServer(nullptr),
      m_mediaVersion(0),
      m_transitionType(TR_NONE),
      m_backgroundVideoLoop(true),
      m_backgroundVideoFillMode(0),
      m_backgroundVideoPaused(false),
      m_mainVideoPaused(true),
      m_mainVideoPosition(0),
      m_mainVideoVolume(100),
      m_mainVideoMuted(false)
{
    m_color.setRgb(0, 0, 0, 0);
    m_imageGenerator.setScreenSize(m_resolution);
}

VirtualOutput::~VirtualOutput()
{
    stopServers();
}

quint16 VirtualOutput::httpPort()
{
    return kVirtualOutputHttpPort;
}

quint16 VirtualOutput::websocketPort()
{
    return kVirtualOutputWebSocketPort;
}

QString VirtualOutput::browserUrl()
{
    return QString("http://127.0.0.1:%1/").arg(httpPort());
}

bool VirtualOutput::initialize()
{
    if (m_initialized && m_httpServer && m_webSocketServer) {
        return true;
    }

    m_imageGenerator.setScreenSize(m_resolution);
    m_initialized = startServers();
    if (m_initialized) {
        qDebug() << "VirtualOutput browser source ready at" << browserUrl();
    }
    return m_initialized;
}

bool VirtualOutput::startServers()
{
    stopServers();

    m_httpServer = new QTcpServer(this);
    connect(m_httpServer, SIGNAL(newConnection()), this, SLOT(onNewHttpConnection()));

    if (!m_httpServer->listen(QHostAddress::LocalHost, httpPort())) {
        qWarning() << "Failed to start virtual output HTTP server:" << m_httpServer->errorString();
        delete m_httpServer;
        m_httpServer = nullptr;
        return false;
    }

    m_webSocketServer = new QWebSocketServer(QStringLiteral("softProjector Virtual Output"),
                                             QWebSocketServer::NonSecureMode, this);
    connect(m_webSocketServer, SIGNAL(newConnection()), this, SLOT(onNewWebSocketConnection()));

    if (!m_webSocketServer->listen(QHostAddress::LocalHost, websocketPort())) {
        qWarning() << "Failed to start virtual output WebSocket server:" << m_webSocketServer->errorString();
        m_httpServer->close();
        delete m_httpServer;
        m_httpServer = nullptr;
        delete m_webSocketServer;
        m_webSocketServer = nullptr;
        return false;
    }

    return true;
}

void VirtualOutput::stopServers()
{
    foreach (QWebSocket *client, m_clients) {
        if (client) {
            client->close();
            client->deleteLater();
        }
    }
    m_clients.clear();

    foreach (QTcpSocket *socket, m_httpSockets) {
        if (socket) {
            socket->disconnect(this);
            socket->close();
            socket->deleteLater();
        }
    }
    m_httpSockets.clear();

    if (m_httpServer) {
        m_httpServer->close();
        m_httpServer->deleteLater();
        m_httpServer = nullptr;
    }

    if (m_webSocketServer) {
        m_webSocketServer->close();
        m_webSocketServer->deleteLater();
        m_webSocketServer = nullptr;
    }

    m_initialized = false;
}

void VirtualOutput::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    if (enabled) {
        if (!initialize()) {
            return;
        }
    } else {
        stopServers();
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);

    if (m_enabled) {
        broadcastState();
    }
}

void VirtualOutput::setResolution(ResolutionPreset preset)
{
    m_resolutionPreset = preset;

    switch (preset) {
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
        break;
    }

    m_imageGenerator.setScreenSize(m_resolution);
    emit resolutionChanged(m_resolution);
    broadcastState();
}

void VirtualOutput::setCustomResolution(int width, int height)
{
    m_resolutionPreset = RES_CUSTOM;
    m_resolution = QSize(width, height);
    m_imageGenerator.setScreenSize(m_resolution);
    emit resolutionChanged(m_resolution);
    broadcastState();
}

void VirtualOutput::setTheme(bool mirrorDisplay1, int themeId)
{
    m_mirrorDisplay1 = mirrorDisplay1;
    m_themeId = themeId;
    emit themeChanged(m_mirrorDisplay1, m_themeId);
}

void VirtualOutput::setLogoOverlay(const QString &imagePath)
{
    m_logoImagePath = imagePath;
    updateOverlayAsset();
    broadcastState();
}

void VirtualOutput::setLowerThirdConfig(bool show, const QString &text, const QFont &font,
                                        const QColor &bgColor, const QColor &textColor)
{
    Q_UNUSED(show)
    Q_UNUSED(text)
    Q_UNUSED(font)
    Q_UNUSED(bgColor)
    Q_UNUSED(textColor)
}

void VirtualOutput::updateDisplay()
{
    broadcastState();
}

void VirtualOutput::renderNotText()
{
    if (!m_enabled) {
        return;
    }

    setTransition(TR_NONE);
    setTextPixmap(m_imageGenerator.generateEmptyImage());
    clearMainVideo();
    updateDisplay();
}

void VirtualOutput::setTransition(int transitionType)
{
    m_transitionType = transitionType;
}

void VirtualOutput::setBackPixmap(const QPixmap &pixmap, int fillMode)
{
    QPixmap scaled = pixmap;

    switch (fillMode) {
    case 0:
        scaled = pixmap.scaled(m_resolution, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        break;
    case 1:
        scaled = pixmap.scaled(m_resolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        break;
    case 2:
        scaled = pixmap.scaled(m_resolution, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        break;
    default:
        break;
    }

    m_backgroundImage.data = pixmapToPng(scaled);
    m_backgroundImage.contentType = QStringLiteral("image/png");
    m_backgroundImage.available = !m_backgroundImage.data.isEmpty();
    ++m_backgroundImage.version;
}

void VirtualOutput::setTextPixmap(const QPixmap &pixmap)
{
    m_textImage.data = pixmapToPng(pixmap);
    m_textImage.contentType = QStringLiteral("image/png");
    m_textImage.available = !m_textImage.data.isEmpty();
    ++m_textImage.version;
}

void VirtualOutput::renderPassiveText(QPixmap &background, bool useBackground, TextSettings &pSets)
{
    if (!m_enabled) {
        return;
    }

    setTransition(TR_NONE);
    setTextPixmap(m_imageGenerator.generateEmptyImage());
    clearMainVideo();

    if (pSets.backgroundType == B_VIDEO && !pSets.backgroundVideoPath.isEmpty()) {
        setBackgroundVideo(pSets.backgroundVideoPath, pSets.backgroundVideoLoop, pSets.backgroundVideoFillMode);
    } else {
        stopBackgroundVideo();
        if (useBackground) {
            setBackPixmap(background, 0);
        } else {
            setBackPixmap(m_imageGenerator.generateColorImage(m_color), 0);
        }
    }

    updateDisplay();
}

void VirtualOutput::renderBibleText(Verse verse, BibleSettings &settings)
{
    if (!m_enabled) {
        return;
    }

    setTransition(settings.useFading ? TR_FADE : TR_NONE);
    clearMainVideo();

    if (settings.backgroundType == B_VIDEO && !settings.backgroundVideoPath.isEmpty()) {
        setBackgroundVideo(settings.backgroundVideoPath, settings.backgroundVideoLoop, settings.backgroundVideoFillMode);
    } else {
        stopBackgroundVideo();
        if (settings.useBackground) {
            setBackPixmap(settings.backgroundPix, 0);
        } else {
            setBackPixmap(m_imageGenerator.generateColorImage(m_color), 0);
        }
    }

    setTextPixmap(m_imageGenerator.generateBibleImage(verse, settings));
    updateDisplay();
}

void VirtualOutput::renderSongText(Stanza stanza, SongSettings &settings)
{
    if (!m_enabled) {
        return;
    }

    setTransition(settings.useFading ? TR_FADE : TR_NONE);
    clearMainVideo();

    if (settings.backgroundType == B_VIDEO && !settings.backgroundVideoPath.isEmpty()) {
        setBackgroundVideo(settings.backgroundVideoPath, settings.backgroundVideoLoop, settings.backgroundVideoFillMode);
    } else {
        stopBackgroundVideo();
        if (settings.useBackground) {
            setBackPixmap(settings.backgroundPix, 0);
        } else {
            setBackPixmap(m_imageGenerator.generateColorImage(m_color), 0);
        }
    }

    setTextPixmap(m_imageGenerator.generateSongImage(stanza, settings));
    updateDisplay();
}

void VirtualOutput::renderAnnounceText(AnnounceSlide announce, TextSettings &settings)
{
    if (!m_enabled) {
        return;
    }

    setTransition(settings.useFading ? TR_FADE : TR_NONE);
    clearMainVideo();

    if (settings.backgroundType == B_VIDEO && !settings.backgroundVideoPath.isEmpty()) {
        setBackgroundVideo(settings.backgroundVideoPath, settings.backgroundVideoLoop, settings.backgroundVideoFillMode);
    } else {
        stopBackgroundVideo();
        if (settings.useBackground) {
            setBackPixmap(settings.backgroundPix, 0);
        } else {
            setBackPixmap(m_imageGenerator.generateColorImage(m_color), 0);
        }
    }

    setTextPixmap(m_imageGenerator.generateAnnounceImage(announce, settings));
    updateDisplay();
}

void VirtualOutput::renderSlideShow(QPixmap slide, SlideShowSettings &settings)
{
    if (!m_enabled) {
        return;
    }

    bool expand = true;
    if (slide.width() < m_imageGenerator.width() && slide.height() < m_imageGenerator.height()) {
        expand = settings.expandSmall;
    }

    setTransition(TR_FADE);
    stopBackgroundVideo();
    clearMainVideo();
    setTextPixmap(m_imageGenerator.generateEmptyImage());
    setBackPixmap(slide, expand ? settings.fitType + 1 : 3);
    updateDisplay();
}

void VirtualOutput::renderVideo(VideoInfo videoDetails)
{
    if (!m_enabled) {
        return;
    }

    stopBackgroundVideo();
    setTransition(TR_NONE);
    setTextPixmap(m_imageGenerator.generateEmptyImage());
    setBackPixmap(m_imageGenerator.generateColorImage(m_color), 0);

    const QString path = localFilePath(videoDetails.filePath);
    if (path != m_mainVideoPath) {
        ++m_mediaVersion;
    }

    m_mainVideoPath = path;
    m_mainVideoPaused = false;
    m_mainVideoPosition = 0;
    updateDisplay();
}

void VirtualOutput::setBackgroundVideo(const QString &path, bool loop, int fillMode)
{
    const QString localPath = localFilePath(QUrl::fromUserInput(path));
    if (localPath != m_backgroundVideoPath) {
        ++m_mediaVersion;
    }

    m_backgroundVideoPath = localPath;
    m_backgroundVideoLoop = loop;
    m_backgroundVideoFillMode = fillMode;
    m_backgroundVideoPaused = false;
    broadcastState();
}

void VirtualOutput::stopBackgroundVideo()
{
    if (m_backgroundVideoPath.isEmpty()) {
        return;
    }

    m_backgroundVideoPath.clear();
    m_backgroundVideoPaused = false;
    ++m_mediaVersion;
    broadcastState();
}

void VirtualOutput::pauseBackgroundVideo()
{
    if (m_backgroundVideoPath.isEmpty()) {
        return;
    }

    m_backgroundVideoPaused = true;
    broadcastState();
}

void VirtualOutput::resumeBackgroundVideo()
{
    if (m_backgroundVideoPath.isEmpty()) {
        return;
    }

    m_backgroundVideoPaused = false;
    broadcastState();
}

void VirtualOutput::playVideo()
{
    if (m_mainVideoPath.isEmpty()) {
        return;
    }

    m_mainVideoPaused = false;
    broadcastState();
}

void VirtualOutput::pauseVideo()
{
    if (m_mainVideoPath.isEmpty()) {
        return;
    }

    m_mainVideoPaused = true;
    broadcastState();
}

void VirtualOutput::stopVideo()
{
    if (m_mainVideoPath.isEmpty()) {
        return;
    }

    m_mainVideoPaused = true;
    m_mainVideoPosition = 0;
    broadcastState();
}

void VirtualOutput::setVideoVolume(int level)
{
    m_mainVideoVolume = level;
    broadcastState();
}

void VirtualOutput::setVideoMuted(bool muted)
{
    m_mainVideoMuted = muted;
    broadcastState();
}

void VirtualOutput::setVideoPosition(qint64 position)
{
    if (m_mainVideoPath.isEmpty()) {
        return;
    }

    m_mainVideoPosition = position;
    emit videoPositionChanged(position);
    broadcastState();
}

void VirtualOutput::positionControls(DisplayControlsSettings &settings)
{
    Q_UNUSED(settings)
}

void VirtualOutput::setControlsVisible(bool visible)
{
    Q_UNUSED(visible)
}

void VirtualOutput::clearMainVideo()
{
    if (m_mainVideoPath.isEmpty()) {
        return;
    }

    m_mainVideoPath.clear();
    m_mainVideoPaused = true;
    m_mainVideoPosition = 0;
    ++m_mediaVersion;
}

void VirtualOutput::updateOverlayAsset()
{
    m_overlayImage.available = false;
    m_overlayImage.data.clear();
    m_overlayImage.contentType.clear();
    ++m_overlayImage.version;

    if (m_logoImagePath.isEmpty()) {
        return;
    }

    QFile file(m_logoImagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    m_overlayImage.data = file.readAll();
    m_overlayImage.contentType = contentTypeForFile(m_logoImagePath);
    if (m_overlayImage.contentType.isEmpty()) {
        m_overlayImage.contentType = QStringLiteral("application/octet-stream");
    }
    m_overlayImage.available = !m_overlayImage.data.isEmpty();
}

QByteArray VirtualOutput::pixmapToPng(const QPixmap &pixmap) const
{
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return png;
}

QString VirtualOutput::contentTypeForFile(const QString &path) const
{
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(path);
    return mime.isValid() ? mime.name() : QString();
}

QString VirtualOutput::localFilePath(const QUrl &url) const
{
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }
    if (url.scheme().isEmpty()) {
        return url.toString();
    }
    return url.toLocalFile();
}

void VirtualOutput::broadcastState()
{
    if (!m_enabled) {
        return;
    }

    const QByteArray state = buildStateMessage();
    foreach (QWebSocket *client, m_clients) {
        if (client && client->isValid()) {
            client->sendTextMessage(QString::fromUtf8(state));
        }
    }
}

void VirtualOutput::sendState(QWebSocket *socket)
{
    if (!socket || !socket->isValid() || !m_enabled) {
        return;
    }
    socket->sendTextMessage(QString::fromUtf8(buildStateMessage()));
}

QByteArray VirtualOutput::buildStateMessage() const
{
    QJsonObject root;
    root.insert(QStringLiteral("type"), QStringLiteral("state"));
    root.insert(QStringLiteral("width"), m_resolution.width());
    root.insert(QStringLiteral("height"), m_resolution.height());
    root.insert(QStringLiteral("transition"), m_transitionType == TR_FADE ? QStringLiteral("fade") : QStringLiteral("none"));

    QJsonObject background;
    background.insert(QStringLiteral("enabled"), m_backgroundImage.available);
    background.insert(QStringLiteral("version"), QString::number(m_backgroundImage.version));
    background.insert(QStringLiteral("url"), QString("/assets/background.png?v=%1").arg(m_backgroundImage.version));
    root.insert(QStringLiteral("backgroundImage"), background);

    QJsonObject text;
    text.insert(QStringLiteral("enabled"), m_textImage.available);
    text.insert(QStringLiteral("version"), QString::number(m_textImage.version));
    text.insert(QStringLiteral("url"), QString("/assets/text.png?v=%1").arg(m_textImage.version));
    root.insert(QStringLiteral("textImage"), text);

    QJsonObject overlay;
    overlay.insert(QStringLiteral("enabled"), m_overlayImage.available);
    overlay.insert(QStringLiteral("version"), QString::number(m_overlayImage.version));
    overlay.insert(QStringLiteral("url"), QString("/assets/overlay?v=%1").arg(m_overlayImage.version));
    root.insert(QStringLiteral("overlay"), overlay);

    QJsonObject backgroundVideo;
    backgroundVideo.insert(QStringLiteral("active"), !m_backgroundVideoPath.isEmpty());
    backgroundVideo.insert(QStringLiteral("url"), QString("/media/background?v=%1").arg(m_mediaVersion));
    backgroundVideo.insert(QStringLiteral("loop"), m_backgroundVideoLoop);
    backgroundVideo.insert(QStringLiteral("fillMode"), m_backgroundVideoFillMode);
    backgroundVideo.insert(QStringLiteral("paused"), m_backgroundVideoPaused);
    root.insert(QStringLiteral("backgroundVideo"), backgroundVideo);

    QJsonObject mainVideo;
    mainVideo.insert(QStringLiteral("active"), !m_mainVideoPath.isEmpty());
    mainVideo.insert(QStringLiteral("url"), QString("/media/main?v=%1").arg(m_mediaVersion));
    mainVideo.insert(QStringLiteral("paused"), m_mainVideoPaused);
    mainVideo.insert(QStringLiteral("positionMs"), QString::number(m_mainVideoPosition));
    mainVideo.insert(QStringLiteral("volume"), m_mainVideoVolume);
    mainVideo.insert(QStringLiteral("muted"), m_mainVideoMuted);
    root.insert(QStringLiteral("mainVideo"), mainVideo);

    root.insert(QStringLiteral("browserUrl"), browserUrl());
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

void VirtualOutput::onNewHttpConnection()
{
    if (!m_httpServer) {
        return;
    }

    while (m_httpServer->hasPendingConnections()) {
        QTcpSocket *socket = m_httpServer->nextPendingConnection();
        m_httpSockets.insert(socket);
        connect(socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    }
}

void VirtualOutput::onSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    QByteArray requestData = socket->property("requestBuffer").toByteArray();
    requestData += socket->readAll();
    if (!requestData.contains("\r\n\r\n")) {
        socket->setProperty("requestBuffer", requestData);
        return;
    }

    socket->setProperty("requestBuffer", QByteArray());

    handleHttpRequest(socket, requestData);
}

void VirtualOutput::onSocketDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }
    m_httpSockets.remove(socket);
    socket->deleteLater();
}

void VirtualOutput::onNewWebSocketConnection()
{
    if (!m_webSocketServer) {
        return;
    }

    QWebSocket *socket = m_webSocketServer->nextPendingConnection();
    if (!socket) {
        return;
    }

    connect(socket, SIGNAL(disconnected()), this, SLOT(onWebSocketDisconnected()));
    m_clients.insert(socket);
    sendState(socket);
}

void VirtualOutput::onWebSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) {
        return;
    }
    m_clients.remove(socket);
    socket->deleteLater();
}

void VirtualOutput::resetHttpSocket(QTcpSocket *socket)
{
    if (!socket) {
        return;
    }
    socket->disconnectFromHost();
}

void VirtualOutput::handleHttpRequest(QTcpSocket *socket, const QByteArray &requestData)
{
    const QList<QByteArray> sections = requestData.split('\n');
    if (sections.isEmpty()) {
        sendBadRequest(socket);
        return;
    }

    const QByteArray requestLine = sections.first().trimmed();
    const QList<QByteArray> requestParts = requestLine.split(' ');
    if (requestParts.size() < 2) {
        sendBadRequest(socket);
        return;
    }

    const QByteArray method = requestParts.at(0);
    const bool sendBody = (method != "HEAD");
    if (method != "GET" && method != "HEAD") {
        sendMethodNotAllowed(socket);
        return;
    }

    QByteArray rangeHeader;
    for (int i = 1; i < sections.count(); ++i) {
        const QByteArray line = sections.at(i).trimmed();
        if (line.startsWith("Range:")) {
            rangeHeader = line.mid(QByteArray("Range:").size()).trimmed();
        }
    }

    const QUrl url = QUrl::fromEncoded(requestParts.at(1));
    const QString path = url.path().isEmpty() ? QStringLiteral("/") : url.path();

    if (path == QStringLiteral("/") || path == QStringLiteral("/index.html")) {
        QFile file(QString::fromLatin1(kVirtualOutputPage));
        if (!file.open(QIODevice::ReadOnly)) {
            sendNotFound(socket);
            return;
        }
        const QByteArray body = file.readAll();
        sendHttpResponse(socket, "HTTP/1.1 200 OK", "text/html; charset=utf-8", body, QList<QByteArray>(), sendBody, body.size());
        return;
    }

    if (path == QStringLiteral("/virtualoutput.js")) {
        QFile file(QString::fromLatin1(kVirtualOutputScript));
        if (!file.open(QIODevice::ReadOnly)) {
            sendNotFound(socket);
            return;
        }
        const QByteArray body = file.readAll();
        sendHttpResponse(socket, "HTTP/1.1 200 OK", "application/javascript; charset=utf-8", body, QList<QByteArray>(), sendBody, body.size());
        return;
    }

    if (path == QStringLiteral("/virtualoutput.css")) {
        QFile file(QString::fromLatin1(kVirtualOutputStyle));
        if (!file.open(QIODevice::ReadOnly)) {
            sendNotFound(socket);
            return;
        }
        const QByteArray body = file.readAll();
        sendHttpResponse(socket, "HTTP/1.1 200 OK", "text/css; charset=utf-8", body, QList<QByteArray>(), sendBody, body.size());
        return;
    }

    if (path == QStringLiteral("/assets/background.png")) {
        sendImageResponse(socket, m_backgroundImage, sendBody);
        return;
    }

    if (path == QStringLiteral("/assets/text.png")) {
        sendImageResponse(socket, m_textImage, sendBody);
        return;
    }

    if (path == QStringLiteral("/assets/overlay")) {
        sendImageResponse(socket, m_overlayImage, sendBody);
        return;
    }

    if (path == QStringLiteral("/media/main")) {
        sendMediaResponse(socket, m_mainVideoPath, sendBody, rangeHeader);
        return;
    }

    if (path == QStringLiteral("/media/background")) {
        sendMediaResponse(socket, m_backgroundVideoPath, sendBody, rangeHeader);
        return;
    }

    sendNotFound(socket);
}

void VirtualOutput::sendHttpResponse(QTcpSocket *socket, const QByteArray &statusLine,
                                     const QByteArray &contentType, const QByteArray &body,
                                     const QList<QByteArray> &extraHeaders, bool sendBody,
                                     qint64 contentLength)
{
    if (!socket) {
        return;
    }

    QByteArray response;
    response += statusLine + "\r\n";
    response += "Connection: close\r\n";
    response += "Cache-Control: no-store, no-cache, must-revalidate\r\n";
    response += "Pragma: no-cache\r\n";
    if (!contentType.isEmpty()) {
        response += "Content-Type: " + contentType + "\r\n";
    }
    if (contentLength < 0) {
        contentLength = body.size();
    }
    response += "Content-Length: " + QByteArray::number(contentLength) + "\r\n";
    foreach (const QByteArray &header, extraHeaders) {
        response += header + "\r\n";
    }
    response += "\r\n";
    if (sendBody) {
        response += body;
    }

    socket->write(response);
    resetHttpSocket(socket);
}

void VirtualOutput::sendImageResponse(QTcpSocket *socket, const AssetState &asset, bool sendBody)
{
    if (!asset.available) {
        sendNotFound(socket);
        return;
    }

    sendHttpResponse(socket, "HTTP/1.1 200 OK", asset.contentType.toUtf8(), asset.data,
                     QList<QByteArray>(), sendBody, asset.data.size());
}

void VirtualOutput::sendMediaResponse(QTcpSocket *socket, const QString &filePath, bool sendBody,
                                      const QByteArray &rangeHeader)
{
    QFile file(filePath);
    if (filePath.isEmpty() || !file.exists() || !file.open(QIODevice::ReadOnly)) {
        sendNotFound(socket);
        return;
    }

    const qint64 totalSize = file.size();
    qint64 start = 0;
    qint64 end = totalSize > 0 ? totalSize - 1 : 0;
    bool partial = false;

    if (!rangeHeader.isEmpty() && rangeHeader.startsWith("bytes=")) {
        const QByteArray range = rangeHeader.mid(6);
        const QList<QByteArray> parts = range.split('-');
        if (!parts.isEmpty()) {
            const QByteArray startPart = parts.at(0).trimmed();
            const QByteArray endPart = parts.size() > 1 ? parts.at(1).trimmed() : QByteArray();
            if (!startPart.isEmpty()) {
                start = startPart.toLongLong();
            }
            if (!endPart.isEmpty()) {
                end = endPart.toLongLong();
            }
            if (end >= totalSize) {
                end = totalSize - 1;
            }
            if (start < 0) {
                start = 0;
            }
            if (start <= end && start < totalSize) {
                partial = true;
            }
        }
    }

    const qint64 length = partial ? (end - start + 1) : totalSize;
    if (partial) {
        file.seek(start);
    }
    const QByteArray body = sendBody ? file.read(length) : QByteArray();

    QList<QByteArray> headers;
    headers << "Accept-Ranges: bytes";
    if (partial) {
        headers << QByteArray("Content-Range: bytes ") + QByteArray::number(start) + "-" + QByteArray::number(end) + "/" + QByteArray::number(totalSize);
        sendHttpResponse(socket, "HTTP/1.1 206 Partial Content", contentTypeForFile(filePath).toUtf8(), body, headers, sendBody, length);
    } else {
        const QByteArray fullBody = sendBody ? file.readAll() : QByteArray();
        sendHttpResponse(socket, "HTTP/1.1 200 OK", contentTypeForFile(filePath).toUtf8(), fullBody, headers, sendBody, totalSize);
    }
}

void VirtualOutput::sendNotFound(QTcpSocket *socket)
{
    sendHttpResponse(socket, "HTTP/1.1 404 Not Found", "text/plain; charset=utf-8", "Not Found");
}

void VirtualOutput::sendMethodNotAllowed(QTcpSocket *socket)
{
    sendHttpResponse(socket, "HTTP/1.1 405 Method Not Allowed", "text/plain; charset=utf-8", "Method Not Allowed",
                     QList<QByteArray>() << "Allow: GET, HEAD");
}

void VirtualOutput::sendBadRequest(QTcpSocket *socket)
{
    sendHttpResponse(socket, "HTTP/1.1 400 Bad Request", "text/plain; charset=utf-8", "Bad Request");
}

void VirtualOutput::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event)
}
