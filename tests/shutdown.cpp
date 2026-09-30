// Reuse the application's database bootstrap with an isolated test database.
#define main softprojectorApplicationMain
#include "../src/sources/main.cpp"
#undef main

#include <QTemporaryDir>
#include <QTcpServer>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

static void check(bool condition, const char *message)
{
    if (!condition) qFatal("%s", message);
}

static int exerciseShutdown(QApplication &app, const QDir &directory)
{
    SoftProjector window;
    window.setAppDataDir(directory);
    window.show();
    for (auto *display : window.pds)
        display->show();

    QTcpServer probe;
    check(probe.listen(QHostAddress::LocalHost, 0), "Cannot choose a stream port");
    StreamSettings stream;
    stream.enabled = true;
    stream.port = probe.serverPort();
    probe.close();
    check(window.streamOutput.configure(stream).isEmpty(), "Cannot start test stream");
    const auto streamAvailable = [&] {
        QNetworkAccessManager network;
        auto *reply = network.get(QNetworkRequest(QUrl(QString("http://127.0.0.1:%1/stream/state").arg(stream.port))));
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(2000, &loop, &QEventLoop::quit);
        loop.exec();
        return reply->isFinished() && reply->error() == QNetworkReply::NoError;
    };
    bool reachedQuit = false;
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &window, [&] {
        reachedQuit = true;
        for (auto *display : window.pds)
            check(!display->isVisible(), "Projector window survived application shutdown");
        check(probe.listen(QHostAddress::LocalHost, stream.port), "Stream listener survived application shutdown");
        probe.close();
    });
    QTimer::singleShot(0, &window, [&] {
        if (app.arguments().contains("--quit")) {
            app.quit();
            return;
        }
        check(QMetaObject::invokeMethod(&window, "on_actionNewSchedule_triggered"), "Cannot create unsaved schedule");
        const auto answer = [](QMessageBox::StandardButton button) {
            QTimer::singleShot(0, [button] {
                for (auto *widget : QApplication::topLevelWidgets())
                    if (auto *dialog = qobject_cast<QMessageBox *>(widget))
                        dialog->button(button)->click();
            });
        };
        answer(QMessageBox::Cancel);
        check(!window.close(), "Cancelled exit unexpectedly closed the main window");
        for (auto *display : window.pds)
            check(display->isVisible(), "Cancelled exit closed a projector window");
        check(streamAvailable(), "Cancelled exit stopped the stream");
        answer(QMessageBox::Discard);
        check(window.close(), "Accepted exit failed to close the main window");
    });
    QTimer::singleShot(10000, &app, [] { qFatal("Application remained alive after closing its main window"); });
    const int result = app.exec();
    check(reachedQuit && result == 0, "Application did not exit cleanly");
    check(!streamAvailable(), "Stream remained reachable after exit");
    // Runtime cleanup must not turn off the user's next-start stream preference.
    check(window.streamOutput.settings().enabled, "Shutdown changed the configured stream preference");
    qInfo("Application shutdown closes all four projectors and the stream: OK");
    return result;
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("SoftProjector shutdown test");
    QTemporaryDir directory;
    check(directory.isValid(), "Cannot create temporary database directory");
    check(::connect(directory.path() + QDir::separator()), "Cannot create test database");
    const int result = exerciseShutdown(app, QDir(directory.path()));
    QSqlDatabase::database().close();
    return result;
}
