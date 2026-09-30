#ifndef STREAMOUTPUT_HPP
#define STREAMOUTPUT_HPP

#include <QHttpServer>
#include <QTcpServer>
#include <QRectF>
#include "imagegenerator.hpp"

enum StreamContent { StreamBible, StreamSong, StreamAnnouncement, StreamPicture, StreamVideo, StreamContentCount };

class QPainter;

struct StreamLayout
{
    bool visible = true;
    QRectF rect;
    QFont font = QFont("Arial", 24);
    QColor textColor = Qt::white;
    QColor panel = Qt::transparent;
    bool shadow = true;
    bool showDetails = true;
    bool crop = false;
    bool useBackground = false;
    bool backgroundFullCanvas = false;
    QString backgroundName;
    QImage background;

    void paintBackground(QPainter &painter, const QRect &canvas, const QRect &content) const;
};

struct StreamSettings
{
    StreamSettings();
    static StreamSettings load();
    void save() const;

    bool enabled = false;
    quint16 port = 8765;
    QSize canvas = QSize(1920, 1080);
    QString bibleId; // Empty means use the primary projector translation.
    StreamLayout layouts[StreamContentCount];
};

class StreamOutput : public QObject
{
    Q_OBJECT
public:
    explicit StreamOutput(QObject *parent = nullptr);
    ~StreamOutput() override;
    const StreamSettings &settings() const { return m_settings; }
    QString configure(const StreamSettings &settings);
    void renderBible(Verse verse);
    void renderSong(Stanza stanza);
    void renderAnnouncement(AnnounceSlide slide);
    void renderPicture(const QPixmap &picture);
    void clear();
    void stop();

private:
    QRect area(StreamContent content) const;
    QImage canvas(StreamContent content) const;
    QFont scaledFont(const QFont &font) const;
    void publish(const QImage &image);

    StreamSettings m_settings;
    ImageGenerator m_generator;
    QTcpServer *m_tcp;
    QHttpServer m_http;
    bool m_bound = false;
    QByteArray m_png;
    quint64 m_revision = 0;
};

#endif // STREAMOUTPUT_HPP
