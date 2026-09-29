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

#include <QtSql>
#include "../headers/settingsdialog.hpp"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    generalSettingswidget = new GeneralSettingWidget;
    passiveSettingwidget = new PassiveSettingWidget;
    bibleSettingswidget = new BibleSettingWidget;
    songSettingswidget = new SongSettingWidget;
    pictureSettingWidget = new PictureSettingWidget;
    announcementSettingswidget = new AnnouncementSettingWidget;
    streamSettingswidget = new StreamSettingsWidget;
    ui->listWidget->addItem(new QListWidgetItem(QIcon(":/icons/icons/display.png"), tr("Stream")));
    ui->stackedWidget->addWidget(streamSettingswidget);

    ui->scrollAreaGeneralSettings->setWidget(generalSettingswidget);
    ui->scrollAreaPassiveSettings->setWidget(passiveSettingwidget);
    ui->scrollAreaBibleSettings->setWidget(bibleSettingswidget);
    ui->scrollAreaSongSettings->setWidget(songSettingswidget);
    ui->scrollAreaPicture->setWidget(pictureSettingWidget);
    ui->scrollAreaAnnouncementSettings->setWidget(announcementSettingswidget);

    btnOk = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnApply = new QPushButton(tr("Apply"));

    ui->buttonBox->addButton(btnOk,QDialogButtonBox::AcceptRole);
    ui->buttonBox->addButton(btnCancel,QDialogButtonBox::RejectRole);
    ui->buttonBox->addButton(btnApply,QDialogButtonBox::ApplyRole);

    // Connect display screen slot
    connect(generalSettingswidget,SIGNAL(setDisp2Use(bool)),this,SLOT(setUseDispScreen2(bool)));
    connect(generalSettingswidget,SIGNAL(setDisp3Use(bool)),this,SLOT(setUseDispScreen3(bool)));
    connect(generalSettingswidget,SIGNAL(setDisp4Use(bool)),this,SLOT(setUseDispScreen4(bool)));
    connect(generalSettingswidget,SIGNAL(themeChanged(int)),this,SLOT(changeTheme(int)));

    // Connect Apply to all
    connect(bibleSettingswidget,SIGNAL(applyBackToAll(int,QString,QPixmap)),this,SLOT(applyToAllActive(int,QString,QPixmap)));
    connect(songSettingswidget,SIGNAL(applyBackToAll(int,QString,QPixmap)),this,SLOT(applyToAllActive(int,QString,QPixmap)));
    connect(announcementSettingswidget,SIGNAL(applyBackToAll(int,QString,QPixmap)),this,SLOT(applyToAllActive(int,QString,QPixmap)));

}

void SettingsDialog::loadSettings(GeneralSettings &sets, Theme &thm, SlideShowSettings &ssets,
                                   BibleVersionSettings bsets[4], const StreamSettings &stream)
{
    gsettings = sets;
    theme = thm;
    for (int i = 0; i < 4; ++i)
        bsettings[i] = bsets[i];
    ssettings = ssets;

    // remember main display window setting if they will be changed
    is_always_on_top = gsettings.displayIsOnTop;
    current_display_screen = gsettings.displayScreen;
    currentDisplayScreen2 = gsettings.displayScreen2;
    currentDisplayScreen3 = gsettings.displayScreen3;
    currentDisplayScreen4 = gsettings.displayScreen4;

    // Set individual items
    generalSettingswidget->setSettings(gsettings);
    bibleSettingswidget->setBibleVersions(bsettings[0], bsettings[1], bsettings[2], bsettings[3]);
    pictureSettingWidget->setSettings(ssettings);
    streamSettingswidget->setSettings(stream);
    setThemes();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;

    delete generalSettingswidget;
    delete passiveSettingwidget;
    delete bibleSettingswidget;
    delete songSettingswidget;
    delete announcementSettingswidget;

    delete btnOk;
    delete btnCancel;
    delete btnApply;
}

void SettingsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch ( e->type() ) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SettingsDialog::on_listWidget_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void SettingsDialog::setUseDispScreen2(bool toUse)
{
    passiveSettingwidget->setDispScreen2Visible(toUse);
    bibleSettingswidget->setDispScreen2Visible(toUse);
    songSettingswidget->setDispScreen2Visible(toUse);
    announcementSettingswidget->setDispScreen2Visible(toUse);
}

void SettingsDialog::setUseDispScreen3(bool toUse)
{
    passiveSettingwidget->setDispScreen3Visible(toUse);
    bibleSettingswidget->setDispScreen3Visible(toUse);
    songSettingswidget->setDispScreen3Visible(toUse);
    announcementSettingswidget->setDispScreen3Visible(toUse);
}

void SettingsDialog::setUseDispScreen4(bool toUse)
{
    passiveSettingwidget->setDispScreen4Visible(toUse);
    bibleSettingswidget->setDispScreen4Visible(toUse);
    songSettingswidget->setDispScreen4Visible(toUse);
    announcementSettingswidget->setDispScreen4Visible(toUse);
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if(button == btnOk)
    {
        applySettings();
        close();
    }
    else if(button == btnCancel)
        close();
    else if(button == btnApply)
        applySettings();
}

void SettingsDialog::applySettings()
{
    gsettings = generalSettingswidget->getSettings();
    bibleSettingswidget->getBibleVersions(bsettings[0], bsettings[1], bsettings[2], bsettings[3]);
    pictureSettingWidget->getSettings(ssettings);
    getThemes();

    // Apply settings
    emit updateSettings(gsettings,theme,ssettings,bsettings);
    emit updateStreamSettings(streamSettingswidget->getSettings());

    // Update <display_on_top> only when changed, or when screen location has been changed
    if(is_always_on_top!=gsettings.displayIsOnTop
            || current_display_screen!=gsettings.displayScreen
            || currentDisplayScreen2!=gsettings.displayScreen2
            || currentDisplayScreen3!=gsettings.displayScreen3
            || currentDisplayScreen4!=gsettings.displayScreen4)
    {
        emit positionsDisplayWindow();
    }

    // Redraw the screen:
    emit updateScreen();

    // Save Settings
    theme.saveThemeUpdate();

    // reset display holders
    is_always_on_top = gsettings.displayIsOnTop;
    current_display_screen = gsettings.displayScreen;
    currentDisplayScreen2 = gsettings.displayScreen2;
    currentDisplayScreen3 = gsettings.displayScreen3;
    currentDisplayScreen4 = gsettings.displayScreen4;
}

void SettingsDialog::getThemes()
{
    passiveSettingwidget->getSettings(theme.passive[0], theme.passive[1], theme.passive[2], theme.passive[3]);
    bibleSettingswidget->getSettings(theme.bible[0], theme.bible[1], theme.bible[2], theme.bible[3]);
    songSettingswidget->getSettings(theme.song[0], theme.song[1], theme.song[2], theme.song[3]);
    announcementSettingswidget->getSettings(theme.announce[0], theme.announce[1], theme.announce[2], theme.announce[3]);
}

void SettingsDialog::setThemes()
{
    passiveSettingwidget->setSetings(theme.passive[0], theme.passive[1], theme.passive[2], theme.passive[3]);
    bibleSettingswidget->setSettings(theme.bible[0], theme.bible[1], theme.bible[2], theme.bible[3]);
    songSettingswidget->setSettings(theme.song[0], theme.song[1], theme.song[2], theme.song[3]);
    announcementSettingswidget->setSettings(theme.announce[0], theme.announce[1], theme.announce[2], theme.announce[3]);
}

void SettingsDialog::changeTheme(int theme_id)
{
    // First save existing changes to the theme
    getThemes();
    theme.saveThemeUpdate();

    // Then load changed theme
    theme.setThemeId(theme_id);
    theme.loadTheme();
    setThemes();
}

void SettingsDialog::applyToAllActive(int t, QString backName, QPixmap background)
{
    switch (t)
    {
    case 1:
        songSettingswidget->setBackgroungds(backName,background);
        announcementSettingswidget->setBackgroungds(backName,background);
        break;
    case 2:
        bibleSettingswidget->setBackgroungds(backName,background);
        announcementSettingswidget->setBackgroungds(backName,background);
        break;
    case 3:
        bibleSettingswidget->setBackgroungds(backName,background);
        songSettingswidget->setBackgroungds(backName,background);
        break;
    default:
        bibleSettingswidget->setBackgroungds(backName,background);
        songSettingswidget->setBackgroungds(backName,background);
        announcementSettingswidget->setBackgroungds(backName,background);
    }
}
