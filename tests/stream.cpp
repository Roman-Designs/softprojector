#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QPainter>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTcpSocket>
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

    widget.setSettings(saved);
    auto *preset = widget.findChild<QComboBox *>("streamPlacementPreset");
    auto *preview = widget.findChild<QWidget *>("streamPlacementPreview");
    auto *exact = widget.findChild<QWidget *>("streamExactPlacement");
    auto *fineTune = widget.findChild<QPushButton *>("streamFineTunePlacement");
    assert(preset && preview && exact && fineTune);
    assert(preset->currentIndex() == 1 && exact->isHidden());
    fineTune->click();
    assert(!exact->isHidden());
    fineTune->click();
    assert(exact->isHidden());
    const auto selectPreset = [&](int index) {
        preset->setCurrentIndex(index);
        preset->activated(index);
    };
    selectPreset(2);
    assert(widget.getSettings().layouts[StreamBible].rect == QRectF(0, 0, 1, 1));
    selectPreset(3);
    assert(widget.getSettings().layouts[StreamBible].rect == QRectF(.66, .07, .30, .32));
    QKeyEvent move(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(preview, &move);
    assert(qAbs(widget.getSettings().layouts[StreamBible].rect.x() - .65) < .0001);
    assert(preset->currentIndex() == 0);
    QKeyEvent resize(QEvent::KeyPress, Qt::Key_Right, Qt::ShiftModifier);
    QApplication::sendEvent(preview, &resize);
    assert(qAbs(widget.getSettings().layouts[StreamBible].rect.width() - .31) < .0001);

    selectPreset(5);
    preview->resize(480, 270);
    const QPointF center(preview->width() / 2.0, preview->height() / 2.0);
    QMouseEvent press(QEvent::MouseButtonPress, center, center, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(preview, &press);
    QMouseEvent drag(QEvent::MouseMove, center + QPointF(2000, 2000), center + QPointF(2000, 2000),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(preview, &drag);
    QMouseEvent release(QEvent::MouseButtonRelease, center, center, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(preview, &release);
    QRectF moved = widget.getSettings().layouts[StreamBible].rect;
    assert(qAbs(moved.x() - .30) < .0001 && qAbs(moved.y() - .50) < .0001);
    assert(qAbs(moved.width() - .70) < .0001 && qAbs(moved.height() - .50) < .0001);

    // Resizing through the preview must not overflow the canvas or lose its origin.
    selectPreset(3);
    const double screenWidth = qMin(double(preview->width() - 16), (preview->height() - 16) * 16.0 / 9);
    const double screenHeight = screenWidth * 9 / 16;
    const QPointF handle((preview->width() - screenWidth) / 2 + .96 * screenWidth - 6,
                         (preview->height() - screenHeight) / 2 + .39 * screenHeight - 6);
    QMouseEvent resizePress(QEvent::MouseButtonPress, handle, handle, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(preview, &resizePress);
    QApplication::sendEvent(preview, &drag);
    QApplication::sendEvent(preview, &release);
    moved = widget.getSettings().layouts[StreamBible].rect;
    assert(qAbs(moved.x() - .66) < .0001 && qAbs(moved.y() - .07) < .0001);
    assert(qAbs(moved.right() - 1) < .0001 && qAbs(moved.bottom() - 1) < .0001);
    const StreamSettings custom = widget.getSettings();
    widget.setSettings(custom);
    assert(widget.getSettings().layouts[StreamBible].rect == moved && preset->currentIndex() == 0);
    widget.setSettings(saved);
    assert(preset->currentIndex() == 1);

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

    StreamSettings backgrounds = saved;
    QImage background(80, 40, QImage::Format_ARGB32);
    background.fill(Qt::green);
    for (int content = StreamBible; content <= StreamPicture; ++content) {
        auto &layout = backgrounds.layouts[content];
        layout.background = background;
        layout.backgroundName = "test background.png";
        layout.useBackground = true;
        layout.backgroundFullCanvas = false;
    }
    backgrounds.save();
    backgrounds = StreamSettings::load();
    for (int content = StreamBible; content <= StreamPicture; ++content) {
        const auto &layout = backgrounds.layouts[content];
        assert(layout.useBackground && !layout.backgroundFullCanvas);
        assert(layout.background == background && layout.backgroundName == "test background.png");
    }

    widget.setSettings(backgrounds);
    auto *backgroundEnabled = widget.findChild<QCheckBox *>("streamBackgroundEnabled");
    auto *backgroundScope = widget.findChild<QComboBox *>("streamBackgroundScope");
    assert(backgroundEnabled && backgroundScope && backgroundEnabled->isChecked());
    backgroundScope->setCurrentIndex(1);
    for (auto *combo : widget.findChildren<QComboBox *>())
        if (combo->findText("Photos") >= 0) combo->setCurrentIndex(StreamPicture);
    assert(backgroundScope->currentIndex() == 0 && backgroundEnabled->isChecked());
    backgroundEnabled->setChecked(false);
    const StreamSettings edited = widget.getSettings();
    assert(edited.layouts[StreamBible].backgroundFullCanvas);
    assert(!edited.layouts[StreamPicture].useBackground);
    assert(edited.layouts[StreamBible].background == background);

    const auto render = [&](int content) {
        if (content == StreamBible) output.renderBible(verse);
        else if (content == StreamSong) output.renderSong(stanza);
        else if (content == StreamAnnouncement) output.renderAnnouncement(announcement);
        else output.renderPicture(photo);
        return QImage::fromData(fetch("/stream/image"), "PNG");
    };
    assert(output.configure(backgrounds).isEmpty());
    for (int content = StreamBible; content <= StreamPicture; ++content) {
        image = render(content);
        assert(image.pixelColor(0, 0).alpha() == 0);
        // A point inside the region but outside the centered text or square photo.
        const QRectF r = backgrounds.layouts[content].rect;
        const QPoint inside(qRound((r.x() + .002) * image.width()),
                            qRound((r.y() + .002) * image.height()));
        assert(image.pixelColor(inside) == QColor(Qt::green));

        backgrounds.layouts[content].backgroundFullCanvas = true;
        assert(output.configure(backgrounds).isEmpty());
        image = render(content);
        assert(image.pixelColor(0, 0) == QColor(Qt::green));

        backgrounds.layouts[content].visible = false;
        assert(output.configure(backgrounds).isEmpty());
        image = render(content);
        assert(image.pixelColor(0, 0).alpha() == 0 && image.pixelColor(inside).alpha() == 0);
        backgrounds.layouts[content].visible = true;
    }
    backgrounds.canvas = QSize(3840, 2160);
    assert(output.configure(backgrounds).isEmpty());
    image = render(StreamBible);
    assert(image.size() == backgrounds.canvas && image.pixelColor(0, 0) == QColor(Qt::green));
    output.clear();
    image = QImage::fromData(fetch("/stream/image"), "PNG");
    assert(image.pixelColor(0, 0).alpha() == 0 && image.pixelColor(2000, 1800).alpha() == 0);

    // Cover scaling must crop the sides of a wide image rather than stretch it.
    StreamLayout cover;
    cover.useBackground = true;
    cover.background = QImage(300, 100, QImage::Format_ARGB32);
    cover.background.fill(Qt::red);
    { QPainter painter(&cover.background); painter.fillRect(100, 0, 100, 100, Qt::blue); }
    QImage square(100, 100, QImage::Format_ARGB32);
    square.fill(Qt::transparent);
    { QPainter painter(&square); cover.paintBackground(painter, square.rect(), square.rect()); }
    assert(square.pixelColor(5, 50) == QColor(Qt::blue));
    assert(square.pixelColor(95, 50) == QColor(Qt::blue));

    // Older saved settings with no background fields retain transparent defaults.
    assert(QSqlQuery("UPDATE Settings SET sets = '{\"layouts\":[{}]}' WHERE type = 'stream'").isActive());
    const StreamSettings legacy = StreamSettings::load();
    assert(!legacy.layouts[StreamBible].useBackground && legacy.layouts[StreamBible].background.isNull());

    QTcpSocket heldConnection;
    heldConnection.connectToHost(QHostAddress::LocalHost, saved.port);
    QEventLoop accepted;
    QTimer::singleShot(100, &accepted, &QEventLoop::quit);
    accepted.exec();
    assert(heldConnection.state() == QAbstractSocket::ConnectedState);
    heldConnection.write("GET /stream/state HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n");
    QEventLoop response;
    QObject::connect(&heldConnection, &QTcpSocket::readyRead, &response, &QEventLoop::quit);
    QTimer::singleShot(2000, &response, &QEventLoop::quit);
    response.exec();
    assert(heldConnection.readAll().contains("200 OK"));
    output.stop();
    QEventLoop disconnected;
    QObject::connect(&heldConnection, &QTcpSocket::disconnected, &disconnected, &QEventLoop::quit);
    QTimer::singleShot(2000, &disconnected, &QEventLoop::quit);
    disconnected.exec();
    assert(heldConnection.state() == QAbstractSocket::UnconnectedState);
    assert(probe.listen(QHostAddress::LocalHost, saved.port));
    probe.close();
    assert(output.configure(saved).isEmpty());
    output.stop();
    output.stop(); // Shutdown is safe when called repeatedly.
    assert(probe.listen(QHostAddress::LocalHost, saved.port));
    qInfo("Stream placement, rendering, backgrounds, transparency, and connection shutdown: OK");
}
