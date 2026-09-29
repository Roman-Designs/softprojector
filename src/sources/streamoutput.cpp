#include "../headers/streamoutput.hpp"

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSqlQuery>

StreamSettings::StreamSettings()
{
    layouts[StreamBible].rect = QRectF(.10, .68, .80, .27);
    layouts[StreamSong].rect = QRectF(.05, .05, .90, .90);
    layouts[StreamSong].showDetails = false;
    layouts[StreamAnnouncement].rect = QRectF(.10, .67, .80, .28);
    layouts[StreamPicture].rect = QRectF(.66, .07, .30, .32);
    layouts[StreamVideo].rect = QRectF(.05, .05, .90, .90);
    layouts[StreamVideo].visible = false;
}

StreamSettings StreamSettings::load()
{
    StreamSettings settings;
    QSqlQuery query;
    query.prepare("SELECT sets FROM Settings WHERE type = 'stream'");
    if (!query.exec() || !query.first())
        return settings;

    const QJsonObject object = QJsonDocument::fromJson(query.value(0).toByteArray()).object();
    settings.enabled = object.value("enabled").toBool(false);
    settings.port = quint16(qBound(1024, object.value("port").toInt(8765), 65535));
    settings.canvas = object.value("canvas").toInt() == 2160 ? QSize(3840, 2160) : QSize(1920, 1080);
    settings.bibleId = object.value("bibleId").toString();
    const QJsonArray layouts = object.value("layouts").toArray();
    for (int i = 0; i < StreamContentCount && i < layouts.size(); ++i) {
        const QJsonObject value = layouts.at(i).toObject();
        StreamLayout &layout = settings.layouts[i];
        layout.visible = value.value("visible").toBool(layout.visible);
        const QJsonArray rect = value.value("rect").toArray();
        if (rect.size() == 4) {
            const double x = qBound(0.0, rect.at(0).toDouble(), .95);
            const double y = qBound(0.0, rect.at(1).toDouble(), .95);
            layout.rect = QRectF(x, y,
                                 qBound(.05, rect.at(2).toDouble(), 1.0 - x),
                                 qBound(.05, rect.at(3).toDouble(), 1.0 - y));
        }
        const QString font = value.value("font").toString();
        if (!font.isEmpty())
            layout.font.fromString(font);
        const QColor text(value.value("textColor").toString());
        if (text.isValid()) layout.textColor = text;
        const QColor panel(value.value("panel").toString());
        if (panel.isValid()) layout.panel = panel;
        layout.shadow = value.value("shadow").toBool(layout.shadow);
        layout.showDetails = value.value("showDetails").toBool(layout.showDetails);
        layout.crop = value.value("crop").toBool(layout.crop);
    }
    return settings;
}

void StreamSettings::save() const
{
    QJsonArray list;
    for (const StreamLayout &layout : layouts) {
        list.append(QJsonObject{
            {"visible", layout.visible},
            {"rect", QJsonArray{layout.rect.x(), layout.rect.y(), layout.rect.width(), layout.rect.height()}},
            {"font", layout.font.toString()},
            {"textColor", layout.textColor.name(QColor::HexArgb)},
            {"panel", layout.panel.name(QColor::HexArgb)},
            {"shadow", layout.shadow},
            {"showDetails", layout.showDetails},
            {"crop", layout.crop}
        });
    }
    const QByteArray json = QJsonDocument(QJsonObject{
        {"enabled", enabled}, {"port", int(port)}, {"canvas", canvas.height()},
        {"bibleId", bibleId}, {"layouts", list}
    }).toJson(QJsonDocument::Compact);
    QSqlQuery query;
    query.prepare("SELECT 1 FROM Settings WHERE type = 'stream'");
    const bool exists = query.exec() && query.first();
    query.prepare(exists ? "UPDATE Settings SET sets = ? WHERE type = 'stream'"
                         : "INSERT INTO Settings (sets, type) VALUES (?, 'stream')");
    query.addBindValue(QString::fromUtf8(json));
    query.exec();
}

StreamOutput::StreamOutput(QObject *parent) : QObject(parent), m_tcp(new QTcpServer(this))
{
    static const QByteArray page = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>SoftProjector Stream</title>
<style>html,body{margin:0;width:100%;height:100%;overflow:hidden;background:transparent}img{display:block;width:100%;height:100%}</style>
</head><body><img alt=""><script>
let revision = -1;
async function refresh() {
  try {
    const response = await fetch('/stream/state', {cache:'no-store'});
    if (!response.ok) throw new Error('Stream disconnected');
    const state = await response.json();
    if (state.revision !== revision) {
      revision = state.revision;
      document.querySelector('img').src = '/stream/image?v=' + revision;
    }
  } catch (_) {
    document.querySelector('img').removeAttribute('src');
    revision = -1;
  }
}
refresh(); setInterval(refresh, 250);
</script></body></html>)HTML";
    m_http.route("/stream", [] { return QHttpServerResponse("text/html; charset=utf-8", page); });
    m_http.route("/stream/state", [this] {
        return QHttpServerResponse(QJsonObject{{"revision", QString::number(m_revision)},
                                              {"width", m_settings.canvas.width()},
                                              {"height", m_settings.canvas.height()}});
    });
    m_http.route("/stream/image", [this] { return QHttpServerResponse("image/png", m_png); });
}

QString StreamOutput::configure(const StreamSettings &settings)
{
    const bool wasListening = m_tcp->isListening();
    if (wasListening && !settings.enabled)
        clear();
    if (m_tcp->isListening() && (!settings.enabled || settings.port != m_settings.port))
        m_tcp->close();
    if (settings.enabled && !m_tcp->isListening()) {
        if (!m_tcp->listen(QHostAddress::AnyIPv4, settings.port)) {
            const QString error = m_tcp->errorString();
            if (wasListening) m_tcp->listen(QHostAddress::AnyIPv4, m_settings.port);
            return error;
        }
        if (!m_bound) {
            m_bound = m_http.bind(m_tcp);
            if (!m_bound) {
                m_tcp->close();
                return tr("Could not start the stream server");
            }
        }
    }
    const bool resized = settings.canvas != m_settings.canvas;
    m_settings = settings;
    if (m_png.isEmpty() || resized)
        clear();
    return {};
}

QRect StreamOutput::area(StreamContent content) const
{
    const QRectF r = m_settings.layouts[content].rect;
    const int w = m_settings.canvas.width(), h = m_settings.canvas.height();
    return QRect(qRound(r.x()*w), qRound(r.y()*h), qRound(r.width()*w), qRound(r.height()*h))
            .intersected(QRect(QPoint(0, 0), m_settings.canvas));
}

QImage StreamOutput::canvas(StreamContent content) const
{
    QImage image(m_settings.canvas, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    if (m_settings.layouts[content].visible) {
        QPainter painter(&image);
        painter.fillRect(area(content), m_settings.layouts[content].panel);
    }
    return image;
}

QFont StreamOutput::scaledFont(const QFont &font) const
{
    QFont scaled = font;
    scaled.setPointSize(qMax(1, font.pointSize()*m_settings.canvas.height()/1080));
    return scaled;
}

void StreamOutput::publish(const QImage &image)
{
    m_png.clear();
    QBuffer buffer(&m_png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    ++m_revision;
}

void StreamOutput::clear()
{
    QImage image(m_settings.canvas, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    publish(image);
}

void StreamOutput::renderBible(Verse verse)
{
    if (verse.primary_text.isEmpty()) { clear(); return; }
    QImage image = canvas(StreamBible);
    const auto &layout = m_settings.layouts[StreamBible];
    if (layout.visible) {
        const QRect target = area(StreamBible);
        BibleSettings settings;
        settings.versions.primaryBible = "stream";
        settings.textFont = scaledFont(layout.font);
        settings.captionFont = settings.textFont;
        settings.captionFont.setPointSize(qMax(1, settings.textFont.pointSize()*2/3));
        settings.textColor = settings.captionColor = layout.textColor;
        settings.textAlignmentV = 1;
        settings.textAlignmentH = settings.captionAlignment = 1;
        settings.useShadow = layout.shadow;
        settings.bibleAddBKColorToText = false;
        if (!layout.showDetails) verse.primary_caption.clear();
        m_generator.setScreenSize(target.size());
        QPainter(&image).drawPixmap(target.topLeft(), m_generator.generateBibleImage(verse, settings));
    }
    publish(image);
}

void StreamOutput::renderSong(Stanza stanza)
{
    QImage image = canvas(StreamSong);
    const auto &layout = m_settings.layouts[StreamSong];
    if (layout.visible) {
        const QRect target = area(StreamSong);
        SongSettings settings;
        settings.textFont = scaledFont(layout.font);
        settings.infoFont = settings.textFont;
        settings.textColor = settings.infoColor = layout.textColor;
        settings.showStanzaTitle = layout.showDetails;
        settings.showSongNumber = settings.showSongKey = settings.showSongEnding = false;
        settings.useShadow = layout.shadow;
        settings.songAddBKColorToText = false;
        m_generator.setScreenSize(target.size());
        QPainter(&image).drawPixmap(target.topLeft(), m_generator.generateSongImage(stanza, settings));
    }
    publish(image);
}

void StreamOutput::renderAnnouncement(AnnounceSlide slide)
{
    if (slide.text.isEmpty()) { clear(); return; }
    QImage image = canvas(StreamAnnouncement);
    const auto &layout = m_settings.layouts[StreamAnnouncement];
    if (layout.visible) {
        const QRect target = area(StreamAnnouncement);
        TextSettings settings;
        settings.textFont = scaledFont(layout.font);
        settings.textColor = layout.textColor;
        settings.textAlignmentV = settings.textAlignmentH = 1;
        settings.useShadow = layout.shadow;
        m_generator.setScreenSize(target.size());
        QPainter(&image).drawPixmap(target.topLeft(), m_generator.generateAnnounceImage(slide, settings));
    }
    publish(image);
}

void StreamOutput::renderPicture(const QPixmap &picture)
{
    if (picture.isNull()) { clear(); return; }
    QImage image = canvas(StreamPicture);
    const auto &layout = m_settings.layouts[StreamPicture];
    if (layout.visible) {
        const QRect target = area(StreamPicture);
        const QPixmap scaled = picture.scaled(target.size(), layout.crop ? Qt::KeepAspectRatioByExpanding
                                                                         : Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect dest(QPoint(), scaled.size());
        dest.moveCenter(target.center());
        QPainter painter(&image);
        painter.setClipRect(target);
        painter.drawPixmap(dest.topLeft(), scaled);
    }
    publish(image);
}
