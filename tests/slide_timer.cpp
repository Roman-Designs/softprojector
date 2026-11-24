// Timed slides: picture slideshows and announcements advance, loop and stop as configured.
// Reuse the application's database bootstrap with an isolated test database.
#define private public
#define main softprojectorApplicationMain
#include "../src/sources/main.cpp"
#undef main
#undef private
#include "ui_softprojector.h"
#include "ui_picturewidget.h"

#include <QTemporaryDir>
#include <QTimer>
#include <QElapsedTimer>

static void check(bool condition, const char *message)
{
    if (!condition) qFatal("%s", message);
}

static SlideShow makeShow(int count, bool autoNext, int seconds, bool loop)
{
    SlideShow show;
    show.name = "Welcome loop";
    show.useAutoNext = autoNext;
    show.slideTimer = seconds;
    show.loop = loop;
    for (int i = 0; i < count; ++i) {
        SlideShowItem item;
        QPixmap pix(64, 48);
        pix.fill(QColor::fromHsv(i * 60, 200, 200));
        item.image = item.imageSmall = item.imagePreview = pix;
        item.name = QString("slide%1.png").arg(i);
        show.slides.append(item);
    }
    return show;
}

static void checkDatabaseColumns()
{
    // A fresh database has the pre-Beta 3 SlideShows table, so this is the upgrade path.
    SlideShow::ensureTimingColumns();
    SlideShow::ensureTimingColumns(); // safe to run on every start
    QSqlQuery sq;
    QStringList columns;
    sq.exec("PRAGMA table_info(SlideShows)");
    while (sq.next())
        columns.append(sq.value(1).toString());
    check(columns.contains("use_auto") && columns.contains("loop") && columns.contains("slide_time"),
          "SlideShows table is missing timing columns");
    sq.exec("PRAGMA user_version");
    sq.first();
    check(sq.value(0).toInt() == dbVer, "Adding timing columns changed the database version");

    SlideShow show = makeShow(2, true, 7, true);
    show.saveSideShow("Saving", nullptr, {});
    SlideShow loaded;
    loaded.loadSlideShow(show.slideShowId);
    check(loaded.useAutoNext && loaded.loop && loaded.slideTimer == 7, "Saved slideshow timing did not reload");

    loaded.useAutoNext = false;
    loaded.loop = false;
    loaded.slideTimer = 3;
    loaded.saveSideShow("Saving", nullptr, {});
    SlideShow updated;
    updated.loadSlideShow(show.slideShowId);
    check(!updated.useAutoNext && !updated.loop && updated.slideTimer == 3, "Updated slideshow timing did not reload");
    qInfo("Slideshow timing is stored in the database without a version change: OK");
}

static void checkScheduleRoundTrip(SoftProjector &window, const QString &directory)
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "timing_schedule");
        db.setDatabaseName(directory + "/schedule.spsc");
        check(db.open(), "Cannot open test schedule");
        QSqlQuery q(db);
        // Old schedules have no timing columns: opening one keeps the defaults.
        q.exec("CREATE TABLE slideshow ('scid' INTEGER, 'ssid' INTEGER, 'name' TEXT, 'info' TEXT)");
        q.exec("CREATE TABLE slides ('scid' INTEGER, 'sid' INTEGER, 'name' TEXT, 'path' TEXT, "
               "'porder' INTEGER, 'image' BLOB, 'imageSmall' BLOB, 'imagePreview' BLOB)");
        q.exec("INSERT INTO slideshow VALUES (1, -1, 'Old', '')");
        SlideShow old;
        window.openScheduleItem(q, 1, old);
        check(!old.useAutoNext && !old.loop && old.slideTimer == 10, "Old schedule slideshow got unexpected timing");

        q.exec("ALTER TABLE slideshow ADD COLUMN 'useAuto' BOOL DEFAULT 0");
        q.exec("ALTER TABLE slideshow ADD COLUMN 'loop' BOOL DEFAULT 0");
        q.exec("ALTER TABLE slideshow ADD COLUMN 'slideTimer' INTEGER DEFAULT 10");
        check(window.saveScheduleItemNew(q, 2, makeShow(2, true, 4, true)), "Cannot save slideshow to schedule");
        SlideShow opened;
        window.openScheduleItem(q, 2, opened);
        check(opened.useAutoNext && opened.loop && opened.slideTimer == 4, "Schedule slideshow timing did not round-trip");
        check(opened.slides.count() == 2, "Schedule slideshow lost slides");
    }
    QSqlDatabase::removeDatabase("timing_schedule");
    qInfo("Schedule files keep slideshow timing and open older files: OK");
}

static void checkPictureTimer(SoftProjector &window, QApplication &app)
{
    SlideShow show = makeShow(3, true, 1, true);
    window.setPictureList(show, 0);
    QListWidget *list = window.ui->listShow;
    check(list->currentRow() == 0, "Slideshow did not start on the first slide");
    check(window.slideTimer->isActive() && window.slideTimer->interval() == 1000, "Timed slideshow did not start its timer");

    // One real tick through the event loop
    QElapsedTimer clock;
    clock.start();
    while (list->currentRow() == 0 && clock.elapsed() < 3000)
        app.processEvents(QEventLoop::AllEvents, 50);
    check(list->currentRow() == 1, "Timer did not advance to the next slide");
    check(clock.elapsed() >= 900, "Slide advanced before its time was up");

    window.advanceTimedSlide();
    check(list->currentRow() == 2, "Did not advance to the last slide");
    window.advanceTimedSlide();
    check(list->currentRow() == 0, "Loop did not return to the first slide");
    check(window.slideTimer->isActive(), "Timer stopped after looping");

    // A manual change restarts the countdown with the full time
    window.slideTimer->start(50);
    list->setCurrentRow(2);
    check(window.slideTimer->remainingTime() > 500, "Manual slide change did not restart the countdown");

    // Without loop the show stops on the last slide
    window.setSlideTiming(true, 1, false);
    window.advanceTimedSlide();
    check(list->currentRow() == 2, "Non-looping show moved past the last slide");
    check(!window.slideTimer->isActive(), "Timer kept running after the last slide");

    // Turning Loop back on while live restarts it
    window.setSlideTiming(true, 1, true);
    check(window.slideTimer->isActive(), "Turning Loop on while live did not restart the timer");

    // Hide and Clear stop it; Show resumes it
    window.on_actionHide_triggered();
    check(!window.slideTimer->isActive(), "Hide did not stop timed slides");
    window.on_actionShow_triggered();
    check(window.slideTimer->isActive(), "Show did not resume timed slides");
    window.on_actionClear_triggered();
    check(!window.slideTimer->isActive(), "Clear did not stop timed slides");

    // Auto advance off
    SlideShow manual = makeShow(3, false, 1, true);
    window.setPictureList(manual, 0);
    check(!window.slideTimer->isActive(), "Untimed slideshow started a timer");

    // The Pictures tab controls change the live show
    window.setPictureList(show, 0);
    window.pictureWidget->ui->checkBoxAutoAdvance->setChecked(true);
    window.pictureWidget->ui->checkBoxAutoAdvance->setChecked(false);
    check(!window.slideTimer->isActive(), "Pictures tab Auto advance did not stop the live show");
    window.pictureWidget->ui->spinBoxSlideTime->setValue(5);
    window.pictureWidget->ui->checkBoxAutoAdvance->setChecked(true);
    check(window.slideTimer->isActive() && window.slideTimer->interval() == 5000,
          "Pictures tab timing did not apply to the live show");
    check(window.pictureWidget->ui->spinBoxSlideTime->isEnabled() && window.pictureWidget->ui->checkBoxLoop->isEnabled(),
          "Pictures tab timing controls stayed disabled");
    qInfo("Picture slideshows advance, loop, and stop on Hide/Clear: OK");
}

static void checkAnnouncementTimer(SoftProjector &window)
{
    Announcement a;
    a.title = "Weekly";
    a.text = "Slide 1\nWelcome\n\nSlide 2\nCoffee after service\n\nSlide 3\nYouth tonight";
    a.useAutoNext = true;
    a.slideTimer = 2;
    a.loop = true;
    window.setAnnounceText(a, 0);
    check(window.ui->listShow->count() == 3, "Announcement did not split into three slides");
    check(window.slideTimer->isActive() && window.slideTimer->interval() == 2000, "Timed announcement did not start its timer");
    window.advanceTimedSlide();
    window.advanceTimedSlide();
    window.advanceTimedSlide();
    check(window.ui->listShow->currentRow() == 0, "Announcement loop did not return to the first slide");

    a.useAutoNext = false;
    window.setAnnounceText(a, 0);
    check(!window.slideTimer->isActive(), "Untimed announcement started a timer");

    // Switching to other content stops the timer
    a.useAutoNext = true;
    window.setAnnounceText(a, 0);
    Song song;
    song.title = "Test";
    song.songText = "Verse 1\nLine\n\nVerse 2\nLine";
    window.setSongList(song, 0);
    check(!window.slideTimer->isActive(), "Timer kept running after switching to a song");
    qInfo("Announcements use their Timed slides and Loop settings: OK");
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("SoftProjector slide timer test");
    QTemporaryDir directory;
    check(directory.isValid(), "Cannot create temporary database directory");
    check(::connect(directory.path() + QDir::separator()), "Cannot create test database");
    checkDatabaseColumns();
    {
        SoftProjector window;
        window.setAppDataDir(QDir(directory.path()));
        checkScheduleRoundTrip(window, directory.path());
        checkPictureTimer(window, app);
        checkAnnouncementTimer(window);
        window.is_schedule_saved = true;
        window.shutdownOutputs();
    }
    QSqlDatabase::database().close();
    return 0;
}
