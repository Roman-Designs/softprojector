#include <QApplication>
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "../src/headers/biblesettingwidget.hpp"

static void check(bool passed, const char *message)
{
    if (!passed)
        qFatal("%s", message);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    auto database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(":memory:");
    check(database.open(), "Cannot open test database");
    QSqlQuery query;
    check(query.exec("CREATE TABLE BibleVersions (id TEXT, bible_name TEXT)"),
          "Cannot create BibleVersions");

    BibleSettingWidget widget;
    BibleVersionSettings versions[4];
    auto load = [&] {
        widget.setBibleVersions(versions[0], versions[1], versions[2], versions[3]);
        widget.getBibleVersions(versions[0], versions[1], versions[2], versions[3]);
    };

    // Opening Settings on a fresh database must also allow applying settings.
    load();
    for (const auto &version : versions)
        check(version.primaryBible == "none", "Empty database must have no primary Bible");
    load(); // Reopening the same dialog.

    check(query.exec("INSERT INTO BibleVersions VALUES ('1', 'Bible one'), ('2', 'Bible two')"),
          "Cannot insert test Bibles");
    for (auto &version : versions)
        version.primaryBible = "removed-bible";
    load();
    for (const auto &version : versions)
        check(version.primaryBible == "none", "Missing primary Bible must be handled safely");

    for (auto &version : versions) {
        version.primaryBible = "1";
        version.secondaryBible = "2";
        version.trinaryBible = "none";
        version.operatorBible = "2";
    }
    load();
    for (const auto &version : versions) {
        check(version.primaryBible == "1", "Primary Bible selection changed");
        check(version.secondaryBible == "2", "Secondary Bible selection changed");
    }
    check(versions[0].operatorBible == "2", "Operator Bible selection changed");

    check(query.exec("INSERT INTO BibleVersions VALUES ('3', 'Bible three')"),
          "Cannot insert third Bible");
    for (auto &version : versions) {
        version.primaryBible = "1";
        version.secondaryBible = "none";
    }
    load();
    for (const QString &suffix : {QString(), QString("2"), QString("3"), QString("4")}) {
        auto *primary = widget.findChild<QComboBox *>("comboBoxPrimaryBible" + suffix);
        auto *secondary = widget.findChild<QComboBox *>("comboBoxSecondaryBible" + suffix);
        auto *tertiary = widget.findChild<QComboBox *>("comboBoxTrinaryBible" + suffix);
        check(primary && secondary && tertiary, "Bible dropdown missing");
        check(!tertiary->isEnabled(), "Third Bible must wait for a second selection");
        primary->setCurrentIndex(primary->findText("Bible two"));
        // Emit the actual Qt 6 signal to verify the automatic slot connection.
        primary->activated(primary->currentIndex());
        check(secondary->findText("Bible one") > 0, "Previous primary must return to dropdown 2");
        check(secondary->findText("Bible two") == -1, "Current primary must be excluded");
        secondary->setCurrentIndex(secondary->findText("Bible one"));
        secondary->activated(secondary->currentIndex());
        check(tertiary->isEnabled(), "Selecting Bible 2 must enable Bible 3");
        check(tertiary->findText("Bible three") > 0, "Remaining translation must be available");
        check(tertiary->findText("Bible one") == -1 && tertiary->findText("Bible two") == -1,
              "Third dropdown must exclude the first two translations");
        tertiary->setCurrentIndex(tertiary->findText("Bible three"));
    }
    widget.getBibleVersions(versions[0], versions[1], versions[2], versions[3]);
    for (const auto &version : versions)
        check(version.primaryBible == "2" && version.secondaryBible == "1" && version.trinaryBible == "3",
              "Switched selections must retain correct translation IDs");
    load();
    for (const auto &version : versions)
        check(version.primaryBible == "2" && version.secondaryBible == "1" && version.trinaryBible == "3",
              "Reopening must retain all three selections");

    // An empty secondary dropdown has index -1, not the 'None' index 0.
    for (const QString &suffix : {QString(), QString("2"), QString("3"), QString("4")}) {
        auto *secondary = widget.findChild<QComboBox *>("comboBoxSecondaryBible" + suffix);
        check(secondary != nullptr, "Secondary Bible dropdown missing");
        secondary->clear();
        const QByteArray method = ("updateTrinaryBibleMenu" + suffix).toLatin1();
        check(QMetaObject::invokeMethod(&widget, method.constData(), Qt::DirectConnection),
              "Cannot update tertiary Bible dropdown");
    }
    qInfo("Settings initialization: empty database, missing Bibles, valid selections, and empty dropdowns OK");
}
