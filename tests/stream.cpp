#include <QApplication>
#include <QComboBox>
#include <QEventLoop>
#include <QHostAddress>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTimer>
#include <cassert>

#include "../src/headers/streamoutput.hpp"
#include "../src/headers/streamsettingswidget.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    assert(db.open());
    assert(QSqlQuery("CREATE TABLE Settings (type TEXT, sets TEXT)").isActive());
    assert(QSqlQuery("CREATE TABLE BibleVersions (id INTEGER, bible_name TEXT)").isActive());
    assert(QSqlQuery("INSERT INTO BibleVersions VALUES (7, 'Stream translation')").isActive());
    assert(QSqlQuery("CREATE TABLE BibleVerse (verse_id TEXT, bible_id TEXT, book TEXT, chapter INTEGER, verse INTEGER, verse_text TEXT)").isActive());
    assert(QSqlQuery("CREATE TABLE BibleBooks (bible_id TEXT, id TEXT, book_name TEXT)").isActive());
    assert(QSqlQuery("INSERT INTO BibleBooks VALUES ('7', '42', 'Луки')").isActive());
    assert(QSqlQuery("INSERT INTO BibleVerse VALUES ('B042C012V020', '7', '42', 12, 20, 'Нерозумний!')").isActive());

    QTcpServer probe;
    assert(probe.listen(QHostAddress::LocalHost, 0));
    StreamSettings settings;
    settings.enabled = true;
    settings.port = probe.serverPort();
    settings.bibleId = "7";
    settings.layouts[StreamBible].textColor = Qt::yellow;
    probe.close();
    settings.save();
    const StreamSettings saved = StreamSettings::load();
    assert(saved.bibleId == "7" && saved.port == settings.port);
    assert(saved.layouts[StreamBible].textColor == Qt::yellow);
    Bible bible;
    bible.currentIdList << "B042C012V020";
    BibleSettings bibleSettings;
    BibleVersionSettings versions;
    versions.primaryBible = saved.bibleId;
    assert(bible.getCurrentVerseAndCaption({0}, bibleSettings, versions).primary_text == QString::fromUtf8("Нерозумний!"));

    StreamSettingsWidget widget;
    widget.setSettings(saved);
    for (auto *combo : widget.findChildren<QComboBox*>())
        if (combo->findText("Photos") >= 0) combo->setCurrentIndex(StreamPicture);
    assert(widget.getSettings().bibleId == "7");
    assert(widget.getSettings().layouts[StreamBible].textColor == Qt::yellow);

    StreamOutput output;
    assert(output.configure(saved).isEmpty());
    QNetworkAccessManager network;
    const QString base = QString("http://127.0.0.1:%1").arg(saved.port);
    const auto fetch = [&](const QString &path) {
        auto *reply = network.get(QNetworkRequest(QUrl(base + path)));
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        loop.exec();
        assert(reply->isFinished() && reply->error() == QNetworkReply::NoError);
        const QByteArray result = reply->readAll();
        reply->deleteLater();
        return result;
    };

    assert(fetch("/stream").contains("background:transparent"));
    const auto revision = [&] {
        return QJsonDocument::fromJson(fetch("/stream/state")).object().value("revision").toString().toInt();
    };
    const int initial = revision();
    Verse verse;
    verse.primary_text = QString::fromUtf8("Нерозумний! Цієї ночі душу твою зажадають від тебе.");
    verse.primary_caption = QString::fromUtf8("Луки 12:20");
    output.renderBible(verse);
    assert(revision() == initial + 1);
    QImage image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.size() == saved.canvas && image.pixelColor(0, 0).alpha() == 0);
    bool hasText = false;
    for (int y = 0; y < image.height() && !hasText; ++y)
        for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y).alpha() > 0) { hasText = true; break; }
    assert(hasText);

    QPixmap photo(100, 100);
    photo.fill(Qt::red);
    output.renderPicture(photo);
    image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.pixelColor(0, 0).alpha() == 0);
    assert(image.pixelColor(1600, 200).red() == 255);
    output.clear();
    image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.pixelColor(1600, 200).alpha() == 0);

    // Even a user-configured tiny text area must return a frame instead of hanging.
    StreamSettings tiny = saved;
    tiny.layouts[StreamBible].rect = tiny.layouts[StreamSong].rect =
            tiny.layouts[StreamAnnouncement].rect = QRectF(0, 0, .05, .05);
    assert(output.configure(tiny).isEmpty());
    output.renderBible(verse);
    Stanza stanza{};
    stanza.stanza = verse.primary_text;
    output.renderSong(stanza);
    AnnounceSlide announcement{};
    announcement.text = verse.primary_text;
    output.renderAnnouncement(announcement);
    assert(!QImage::fromData(fetch("/stream/image"), "PNG").isNull());

    tiny.canvas = QSize(3840, 2160);
    assert(output.configure(tiny).isEmpty());
    output.renderBible(verse);
    image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.size() == tiny.canvas && image.pixelColor(0, 0).alpha() == 0);

    StreamSettings disabled = tiny;
    disabled.enabled = false;
    assert(output.configure(disabled).isEmpty());
    assert(output.configure(tiny).isEmpty());
    image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.size() == tiny.canvas && image.pixelColor(0, 0).alpha() == 0);
}
