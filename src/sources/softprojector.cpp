/***************************************************************************
//
//    SoftProjector - an open source media projection software
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

#include <QWidget>
#include "../headers/softprojector.hpp"
#include "ui_softprojector.h"
#include "../headers/aboutdialog.hpp"
#include "../headers/editannouncementdialog.hpp"
#include <QTemporaryDir>
#include <filesystem>

SoftProjector::SoftProjector(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SoftProjectorClass)
{
    // Load settings
    mySettings.loadSettings();
    theme.setThemeId(mySettings.general.currentThemeId);
    theme.loadTheme();
    // Reset current theme id if initial was 0
    mySettings.general.currentThemeId = theme.getThemeId();

    // Update Themes Bible Versions
    for (int i = 0; i < 4; ++i)
        theme.bible[i].versions = mySettings.bibleSets[i];

    //Setting up the Display Screen
    for (int i = 0; i < 4; ++i)
        pds[i] = new ProjectorDisplayScreen();
    const QString streamError = streamOutput.configure(StreamSettings::load());
    if (!streamError.isEmpty())
        qWarning() << "Stream server:" << streamError;

    bibleWidget = new BibleWidget;
    songWidget = new SongWidget;
    editWidget = new EditWidget;
    announceWidget = new AnnounceWidget;
    manageDialog = new ManageDataDialog(this);
    settingsDialog = new SettingsDialog(this);
    helpDialog = new HelpDialog();
    pictureWidget = new PictureWidget;
    mediaPlayer = new MediaWidget;
    mediaControls = new MediaControl(this);

    ui->setupUi(this);
    if (!streamError.isEmpty())
        statusBar()->showMessage(tr("Stream unavailable: %1").arg(streamError));

    // Create action group for language slections
    languagePath = qApp->applicationDirPath()+QString(QDir::separator())+"translations"+QString(QDir::separator());
    createLanguageActions();

    // Always place the "Settings" menu item under the application
    // menu, even if the item is translated (Mac OS X):
    ui->actionSettings->setMenuRole(QAction::PreferencesRole);
    // FIXME Make the Preferences menu appear in the menu bar even for the
    // display window (Mac OS X)

    // Apply Settings
    BibleVersionSettings bsets[4] = {mySettings.bibleSets[0], mySettings.bibleSets[1], mySettings.bibleSets[2], mySettings.bibleSets[3]};
    applySetting(mySettings.general, theme, mySettings.slideSets, bsets[0], bsets[1], bsets[2], bsets[3]);

    positionDisplayWindow();

    showing = false;

    ui->projectTab->clear();
    ui->projectTab->addTab(bibleWidget,QIcon(":/icons/icons/book.png"), tr("Bible (F6)"));
    ui->projectTab->addTab(songWidget,QIcon(":/icons/icons/song_tab.png"), tr("Songs (F7)"));
    ui->projectTab->addTab(pictureWidget,QIcon(":/icons/icons/photo.png"),tr("Pictures"));
    ui->projectTab->addTab(mediaPlayer,QIcon(":/icons/icons/video.png"),tr("Media"));
    ui->projectTab->addTab(announceWidget,QIcon(":/icons/icons/announce.png"), tr("Announcements (F8)"));
    ui->projectTab->setCurrentIndex(0);


    connect(bibleWidget, SIGNAL(goLive(QStringList, QString, QItemSelection)),
            this, SLOT(setChapterList(QStringList, QString, QItemSelection)));
    connect(bibleWidget, SIGNAL(setArrowCursor()), this, SLOT(setArrowCursor()));
    connect(bibleWidget, SIGNAL(setWaitCursor()), this, SLOT(setWaitCursor()));
    connect(songWidget, SIGNAL(sendSong(Song, int)), this, SLOT(setSongList(Song, int)));
    connect(songWidget, SIGNAL(setArrowCursor()), this, SLOT(setArrowCursor()));
    connect(songWidget, SIGNAL(setWaitCursor()), this, SLOT(setWaitCursor()));
    connect(announceWidget,SIGNAL(sendAnnounce(Announcement,int)), this, SLOT(setAnnounceText(Announcement,int)));
    connect(pictureWidget, SIGNAL(sendSlideShow(QList<SlideShowItem>&,int,QString)),
            this, SLOT(setPictureList(QList<SlideShowItem>&,int,QString)));
    connect(pictureWidget, SIGNAL(sendToSchedule(SlideShow&)),this,SLOT(addToShcedule(SlideShow&)));
    connect(mediaPlayer, SIGNAL(toProjector(VideoInfo&)), this, SLOT(setVideo(VideoInfo&)));
    connect(editWidget, SIGNAL(updateSongFromDatabase(int,int)), songWidget, SLOT(updateSongFromDatabase(int,int)));
    connect(editWidget, SIGNAL(addedNew(Song,int)), songWidget,SLOT(addNewSong(Song,int)));
    connect(manageDialog, SIGNAL(setMainArrowCursor()), this, SLOT(setArrowCursor()));
    connect(manageDialog, SIGNAL(setMainWaitCursor()), this, SLOT(setWaitCursor()));
    connect(languageGroup, SIGNAL(triggered(QAction*)), this, SLOT(switchLanguage(QAction*)));
    connect(pds[0], SIGNAL(exitSlide()), this, SLOT(on_actionHide_triggered()));
    connect(pds[0], SIGNAL(nextSlide()), this, SLOT(nextSlide()));
    connect(pds[0], SIGNAL(prevSlide()), this, SLOT(prevSlide()));
    connect(settingsDialog, &SettingsDialog::updateSettings, this, &SoftProjector::updateSetting);
    connect(settingsDialog, &SettingsDialog::updateStreamSettings, this, &SoftProjector::applyStreamSettings);
    connect(settingsDialog,SIGNAL(positionsDisplayWindow()),this,SLOT(positionDisplayWindow()));
    connect(settingsDialog,SIGNAL(updateScreen()),this,SLOT(updateScreen()));
    connect(songWidget,SIGNAL(addToSchedule(Song&)),this,SLOT(addToShcedule(Song&)));
    connect(announceWidget,SIGNAL(addToSchedule(Announcement&)),this,SLOT(addToShcedule(Announcement&)));

    // Add tool bar actions
    ui->toolBarFile->addAction(ui->actionNewSchedule);
    ui->toolBarFile->addAction(ui->actionOpenSchedule);
    ui->toolBarFile->addAction(ui->actionSaveSchedule);
    ui->toolBarFile->addSeparator();
    ui->toolBarFile->addAction(ui->actionPrint);

    ui->toolBarSchedule->addAction(ui->actionMoveScheduleTop);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleUp);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleDown);
    ui->toolBarSchedule->addAction(ui->actionMoveScheduleBottom);
    ui->toolBarSchedule->addSeparator();
    ui->toolBarSchedule->addAction(ui->actionScheduleAdd);
    ui->toolBarSchedule->addAction(ui->actionScheduleRemove);
    ui->toolBarSchedule->addAction(ui->actionScheduleClear);

    ui->toolBarEdit->addAction(ui->actionNew);
    ui->toolBarEdit->addAction(ui->actionEdit);
    ui->toolBarEdit->addAction(ui->actionCopy);
    ui->toolBarEdit->addAction(ui->actionDelete);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionSettings);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->actionSong_Counter);
    ui->toolBarEdit->addSeparator();
    ui->toolBarEdit->addAction(ui->action_Help);

    ui->toolBarShow->addAction(ui->actionShow);
    ui->toolBarShow->addAction(ui->actionClear);
    ui->toolBarShow->addAction(ui->actionHide);
    ui->toolBarShow->addAction(ui->actionCloseDisplay);

    ui->actionShow->setEnabled(false);
    ui->actionHide->setEnabled(false);
    ui->actionClear->setEnabled(false);

    // Create and connect shortcuts
    shpgUP = new QShortcut(Qt::Key_PageUp,this);
    shpgDwn = new QShortcut(Qt::Key_PageDown,this);
    shSart1 = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_B),this);
    shSart2 = new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5),this);
    connect(shpgUP,SIGNAL(activated()),this,SLOT(prevSlide()));
    connect(shpgDwn,SIGNAL(activated()),this,SLOT(nextSlide()));
    connect(shSart1,SIGNAL(activated()),this,SLOT(on_actionShow_triggered()));
    connect(shSart2,SIGNAL(activated()),this,SLOT(on_actionShow_triggered()));

    // Hide Multi verse selection, only visible to be when showing bible
    ui->widgetMultiVerse->setVisible(false);

    // Set up video controls
    ui->verticalLayoutDisplayControls->insertWidget(1,mediaControls);
    mediaControls->setVisible(false);
    mediaControls->setVolume(100);
    connect(pds[0], SIGNAL(videoPositionChanged(qint64)),
            mediaControls, SLOT(updateTime(qint64)));
    connect(pds[0], SIGNAL(videoDurationChanged(qint64)),
            mediaControls, SLOT(setMaximumTime(qint64)));
    connect(pds[0], &ProjectorDisplayScreen::videoPlaybackStateChanged,
            mediaControls, &MediaControl::updatePlayerState);
    connect(pds[0], SIGNAL(videoStopped()), this, SLOT(videoStopped()));
    connect(mediaControls, SIGNAL(play()), this, SLOT(playVideo()));
    connect(mediaControls, SIGNAL(pause()), this, SLOT(pauseVideo()));
    connect(mediaControls, SIGNAL(stop()), this, SLOT(stopVideo()));
    connect(mediaControls, SIGNAL(volumeChanged(int)), pds[0], SLOT(setVideoVolume(int)));
    connect(mediaControls, SIGNAL(muted(bool)), pds[0], SLOT(setVideoMuted(bool)));
    connect(mediaControls, SIGNAL(timeChanged(qint64)), this, SLOT(setVideoPosition(qint64)));

    version_string = "2.2";
    this->setWindowTitle("SoftProjector " + version_string);
}

SoftProjector::~SoftProjector()
{
    saveSettings();
    delete songWidget;
    delete editWidget;
    delete bibleWidget;
    delete announceWidget;
    delete manageDialog;
    delete mediaPlayer;
    for (int i = 0; i < 4; ++i)
        delete pds[i];
    delete languageGroup;
    delete settingsDialog;
    delete shpgUP;
    delete shpgDwn;
    delete shSart1;
    delete shSart2;
    delete helpDialog;
    delete ui;
}

void SoftProjector::positionDisplayWindow()
{
    // Position the display window as needed (including setting "always on top" flag,
    // showing full screen / normal mode, and positioning it on the right screen)

    if (mySettings.general.displayIsOnTop)
    {
        for (int i = 0; i < 4; ++i)
            pds[i]->setWindowFlags(Qt::WindowStaysOnTopHint);
    }

    qDebug()<< "Screen Count: " << QApplication::primaryScreen()->virtualSiblings().count();

    if(QApplication::primaryScreen()->virtualSiblings().count() > 1)
    {
        // Primary display
        pds[0]->setGeometry(QApplication::primaryScreen()->virtualSiblings().at(mySettings.general.displayScreen)->geometry());
        pds[0]->setCursor(Qt::BlankCursor);
        pds[0]->resetImGenSize();
        pds[0]->renderPassiveText(theme.passive[0].backgroundPix, theme.passive[0].useBackground);
        pds[0]->setControlsVisible(false);

        if(mySettings.general.displayOnStartUp)
        {
            pds[0]->showFullScreen();
            ui->actionCloseDisplay->setChecked(true);
            updateCloseDisplayButtons(true);
        }

        // Additional displays
        int displayScreens[4] = {mySettings.general.displayScreen, mySettings.general.displayScreen2,
                                 mySettings.general.displayScreen3, mySettings.general.displayScreen4};
        for (int i = 1; i < 4; ++i)
        {
            if (displayScreens[i] >= 0)
            {
                hasDisplayScreen[i] = true;
                pds[i]->setGeometry(QApplication::primaryScreen()->virtualSiblings().at(displayScreens[i])->geometry());
                pds[i]->resetImGenSize();
                pds[i]->setCursor(Qt::BlankCursor);
                pds[i]->renderPassiveText(theme.passive[i].backgroundPix, theme.passive[i].useBackground);
                pds[i]->setControlsVisible(false);
                if(mySettings.general.displayOnStartUp)
                    pds[i]->showFullScreen();
            }
            else
            {
                hasDisplayScreen[i] = false;
                pds[i]->hide();
            }
        }

        isSingleScreen = false;
    }
    else
    {
        // Single monitor only: Do not show on strat up.
        // Will be shown only when items were sent to the projector.
        qDebug()<< "Setting Primary screen";
        pds[0]->setGeometry(QApplication::primaryScreen()->virtualSiblings().at(0)->geometry());
        pds[0]->resetImGenSize();
        showDisplayScreen(false);
        isSingleScreen = true;
        for (int i = 1; i < 4; ++i)
            hasDisplayScreen[i] = false;
    }
}

void SoftProjector::showDisplayScreen(bool show)
{
    if(show)
    {
        pds[0]->showFullScreen();
        pds[0]->positionControls(mySettings.general.displayControls);
        pds[0]->setControlsVisible(true);
    }
    else
    {
        pds[0]->hide();
        QPixmap p;
        pds[0]->renderPassiveText(p,false);
        ui->actionCloseDisplay->setEnabled(false);
    }

}

void SoftProjector::saveSettings()
{
    // Save splitter states
    mySettings.spMain.spSplitter = ui->splitter->saveState();
    mySettings.spMain.bibleHiddenSplitter = bibleWidget->getHiddenSplitterState();
    mySettings.spMain.bibleShowSplitter = bibleWidget->getShownSplitterState();
    mySettings.spMain.songSplitter = songWidget->getSplitterState();

    // Save window maximized state
    mySettings.spMain.isWindowMaximized = this->isMaximized();

    // save translation settings
    QList<QAction*> languageActions = ui->menuLanguage->actions();

    for(int i(0);i < languageActions.count();++i)
    {
        if(languageActions.at(i)->isChecked())
        {
            if(i < languageActions.count())
                mySettings.spMain.uiTranslation = languageActions.at(i)->data().toString();
            else
                mySettings.spMain.uiTranslation = "en";
        }
    }

    // save settings
    mySettings.saveSettings();
    theme.saveThemeUpdate();
}

void SoftProjector::updateSetting(GeneralSettings &g, Theme &t, SlideShowSettings &ssets,
                                  BibleVersionSettings bsets[4])
{
    mySettings.general = g;
    mySettings.slideSets = ssets;
    for (int i = 0; i < 4; ++i)
        mySettings.bibleSets[i] = bsets[i];
    mySettings.saveSettings();
    theme = t;
    bibleWidget->setSettings(mySettings.bibleSets[0]);
    pictureWidget->setSettings(mySettings.slideSets);

    for (int i = 0; i < 4; ++i)
        theme.bible[i].versions = mySettings.bibleSets[i];
}

void SoftProjector::applyStreamSettings(const StreamSettings &settings)
{
    const QString error = streamOutput.configure(settings);
    if (!error.isEmpty())
        QMessageBox::warning(this, tr("Stream output"), tr("Could not start the OBS browser source: %1").arg(error));
    else
        settings.save();
}

void SoftProjector::applySetting(GeneralSettings &g, Theme &t, SlideShowSettings &s,
                                 BibleVersionSettings &b1, BibleVersionSettings &b2,
                                 BibleVersionSettings &b3, BibleVersionSettings &b4)
{
    BibleVersionSettings bsets[4] = {b1, b2, b3, b4};
    updateSetting(g,t,s,bsets);

    // Apply splitter states
    ui->splitter->restoreState(mySettings.spMain.spSplitter);
    bibleWidget->setHiddenSplitterState(mySettings.spMain.bibleHiddenSplitter);
    bibleWidget->setShownSplitterState(mySettings.spMain.bibleShowSplitter);
    songWidget->setSplitterState(mySettings.spMain.songSplitter);

    // Apply window maximized
    if(mySettings.spMain.isWindowMaximized)
        this->setWindowState(Qt::WindowMaximized);

    // Apply current translation
    QList<QAction*> la = ui->menuLanguage->actions();
    QString splocale;
    for(int i(0);i < la.count(); ++i)
    {
        if(la.at(i)->data().toString() == mySettings.spMain.uiTranslation)
        {
            if(i < la.count())
            {
                ui->menuLanguage->actions().at(i)->setChecked(true);
                splocale = mySettings.spMain.uiTranslation;
            }
            else
            {
                ui->menuLanguage->actions().at(0)->setChecked(true);//default
                splocale = "en";
            }
        }
    }
    cur_locale = splocale;
    retranslateUis();
}

void SoftProjector::closeEvent(QCloseEvent *event)
{
    if (!is_schedule_saved && !confirmSaveSchedule(tr("Do you want to save current schedule?")))
        event->ignore();
    else
        event->accept();
}

void SoftProjector::keyPressEvent(QKeyEvent *event)
{
    // Will get called when a key is pressed
    int key = event->key();
    if(key == Qt::Key_F6)
    {
        ui->projectTab->setCurrentWidget(bibleWidget);
        bibleWidget->setBibleBookActive();
    }
    else if(key == Qt::Key_F7)
    {
        ui->projectTab->setCurrentWidget(songWidget);
        songWidget->setSearchActive();
    }
    else if(key == Qt::Key_F8)
        ui->projectTab->setCurrentWidget(announceWidget);
	else if(key == Qt::Key_F9)
    {
        ui->projectTab->setCurrentWidget(bibleWidget);
        bibleWidget->setBibleSearchActive();
    }
    else if(key == Qt::Key_Left)
        prevSlide();
    else if(key == Qt::Key_Back)
        prevSlide();
    else if(key == Qt::Key_Right)
        nextSlide();
    else if(key == Qt::Key_Forward)
        nextSlide();
    else if(key == Qt::Key_Return)
        nextSlide();
    else if(key == Qt::Key_Enter)
        nextSlide();
    else
        QMainWindow::keyPressEvent(event);
}

void SoftProjector::on_actionClose_triggered()
{
    close();
}

void SoftProjector::setAnnounceText(Announcement announce, int row)
{
    currentAnnounce = announce;
    pType = ANNOUCEMENT;
    ui->widgetMultiVerse->setVisible(false);
    ui->rbMultiVerse->setChecked(false);
    mediaControls->setVisible(false);
    showing = true;
    new_list = true;
    ui->labelIcon->setPixmap(QPixmap(":/icons/icons/announce.png").scaled(16,16,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    ui->labelShow->setText(currentAnnounce.title);
    ui->labelSongNotes->setVisible(false);
    ui->listShow->clear();
    ui->listShow->setSpacing(5); // ?
    ui->listShow->setWordWrap(true);
    ui->listShow->addItems(currentAnnounce.getAnnounceList());
    ui->listShow->setCurrentRow(row);
    ui->listShow->setFocus();
    new_list = false;
    updateScreen();
}

void SoftProjector::setSongList(Song song, int row)
{
    QStringList song_list = song.getSongTextList();
    current_song = song;
    current_song_verse = row;

    // Display the specified song text in the right-most column of softProjector
    pType = SONG;
    ui->widgetMultiVerse->setVisible(false);
    ui->rbMultiVerse->setChecked(false);
    mediaControls->setVisible(false);
    showing = true;
    new_list = true;
    ui->listShow->clear();
    ui->labelIcon->setPixmap(QPixmap(":/icons/icons/song_tab.png").scaled(16,16,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    ui->labelShow->setText(song.title);
    if(song.notes.isEmpty())
        ui->labelSongNotes->setVisible(false);
    else
    {
        ui->labelSongNotes->setText(QString("%1\n%2").arg(tr("Notes:","Notes to songs")).arg(song.notes));
        ui->labelSongNotes->setVisible(true);
    }
    ui->listShow->setSpacing(5);
    ui->listShow->setWordWrap(false);
    ui->listShow->addItems(song_list);
    ui->listShow->setCurrentRow(row);
    ui->listShow->setFocus();
    new_list = false;
    updateScreen();
}

void SoftProjector::setChapterList(QStringList chapter_list, QString caption, QItemSelection selectedItems)
{
    // Called to show a bible verse from a chapter in the preview list
    pType = BIBLE;
    ui->widgetMultiVerse->setVisible(true);
    mediaControls->setVisible(false);
    showing = true;
    new_list = true;
    ui->labelIcon->setPixmap(QPixmap(":/icons/icons/book.png").scaled(16,16,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    ui->labelShow->setText(caption);
    ui->labelSongNotes->setVisible(false);
    ui->listShow->clear();
    ui->listShow->setSpacing(2);
    ui->listShow->setWordWrap(true);
    ui->listShow->addItems(chapter_list);
    if(selectedItems.indexes().count()>1)
        ui->rbMultiVerse->setChecked(true);
    else
        ui->rbMultiVerse->setChecked(false);

    ui->listShow->setCurrentRow(selectedItems.first().top());
    ui->listShow->selectionModel()->select(selectedItems,QItemSelectionModel::Select);
    ui->listShow->setFocus();
    new_list = false;
    updateScreen();
}

void SoftProjector::setPictureList(QList<SlideShowItem> &image_list,int row,QString name)
{
    // Called to show picture list
    pType = PICTURE;
    showing = true;
    ui->widgetMultiVerse->setVisible(false);
    ui->rbMultiVerse->setChecked(false);
    mediaControls->setVisible(false);
    new_list = true;
    pictureShowList = image_list;
    ui->labelIcon->setPixmap(QPixmap(":/icons/icons/photo.png").scaled(16,16,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    ui->labelShow->setText(name);
    ui->labelSongNotes->setVisible(false);
    ui->listShow->clear();
    ui->listShow->setSpacing(1);
    ui->listShow->setIconSize(QSize(100,100));

    foreach(const SlideShowItem &p, pictureShowList)
    {
        QListWidgetItem *itm = new QListWidgetItem;
        QIcon ico(p.imageSmall);
        itm->setIcon(ico);
        ui->listShow->addItem(itm);
    }

    ui->listShow->setCurrentRow(row);
    ui->listShow->setFocus();
    new_list = false;
    updateScreen();
}

void SoftProjector::setVideo(VideoInfo &video)
{
    // Called when showing video
    pType = VIDEO;
    currentVideo = video;
    showing = true;
    ui->widgetMultiVerse->setVisible(false);
    ui->rbMultiVerse->setChecked(false);
    if(!mediaControls->isVisible())
    {
        mediaControls->setVisible(true);
    }
    new_list = true;
    ui->listShow->clear();
    ui->labelSongNotes->setVisible(false);
    ui->labelIcon->setPixmap(QPixmap(":/icons/icons/video.png").scaled(16,16,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
    ui->labelShow->setText(currentVideo.fileName);
    new_list = false;
    updateScreen();
}

void SoftProjector::playVideo()
{
    pds[0]->playVideo();
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->playVideo();
}

void SoftProjector::pauseVideo()
{
    pds[0]->pauseVideo();
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->pauseVideo();
}

void SoftProjector::stopVideo()
{
    pds[0]->stopVideo();
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->stopVideo();
}

void SoftProjector::setVideoPosition(qint64 position)
{
    pds[0]->setVideoPosition(position);
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->setVideoPosition(position);
}

void SoftProjector::videoStopped()
{
    showing = false;
    updateScreen();
}

void SoftProjector::on_listShow_currentRowChanged(int currentRow)
{
    // Called when the user selects a different row in the show (right-most) list.
    //    updateScreen();
}

void SoftProjector::on_listShow_itemSelectionChanged()
{
    // Called when the user selects a different row in the show (right-most) list.
    // First check if ratio button "Multi Verse" is check. If so, make button "Show"
    // enable and update screen only after show_botton is clicked.
    if(ui->rbMultiVerse->isChecked())
        ui->actionShow->setEnabled(true);
    else
        updateScreen();
}

void SoftProjector::updateScreen()
{
    // Display the specified row of the show (rightmost) table to
    // the display
    int currentRow = ui->listShow->currentRow();

    if(!showing)
    {
        // Do not display any text:
        if (streamOutput.settings().enabled)
            streamOutput.clear();
        pds[0]->renderPassiveText(theme.passive[0].backgroundPix, theme.passive[0].useBackground);

        if(isSingleScreen)
            showDisplayScreen(false);

        for (int i = 1; i < 4; ++i) {
            if (hasDisplayScreen[i]) {
                if (!theme.passive[i].useDisp1settings) {
                    pds[i]->renderPassiveText(theme.passive[i].backgroundPix, theme.passive[i].useBackground);
                } else {
                    pds[i]->renderPassiveText(theme.passive[0].backgroundPix, theme.passive[0].useBackground);
                }
            }
        }
        stopVideo();
        ui->actionShow->setEnabled(true);
        ui->actionHide->setEnabled(false);
        ui->actionClear->setEnabled(false);
    }
    else if ((currentRow >=0 || pType == VIDEO)  && !new_list)
    {
        if(isSingleScreen)
        {
            if(pds[0]->isHidden())
                showDisplayScreen(true);
        }
        else
        {
            if(pds[0]->isHidden() || !ui->actionCloseDisplay->isChecked())
            {
                ui->actionCloseDisplay->trigger();
            }
        }

        ui->actionShow->setEnabled(false);
        ui->actionHide->setEnabled(true);
        switch (pType)
        {
        case BIBLE:
        case SONG:
        case ANNOUCEMENT:
            ui->actionClear->setEnabled(true);
            break;
        default:
            ui->actionClear->setEnabled(false);
            break;
        }

        switch (pType)
        {
        case BIBLE:
            showBible();
            break;
        case SONG:
            showSong(currentRow);
            break;
        case ANNOUCEMENT:
            showAnnounce(currentRow);
            break;
        case PICTURE:
            showPicture(currentRow);
            break;
        case VIDEO:
            showVideo();
            break;
        default:
            // ERROR Unown type to show.
            break;
        }
    }
    else if (!new_list && streamOutput.settings().enabled)
        streamOutput.clear();
}

void SoftProjector::showBible()
{
    int srows(ui->listShow->count());
    QList<int> currentRows;
    for(int i(0); i<srows; ++i)
    {
        if(ui->listShow->item(i)->isSelected())
            currentRows.append(i);
    }
    pds[0]->renderBibleText(bibleWidget->bible.getCurrentVerseAndCaption(
                              currentRows,theme.bible[0],mySettings.bibleSets[0]),
                          theme.bible[0]);
    for (int i = 1; i < 4; ++i) {
        if (hasDisplayScreen[i]) {
            if (!theme.bible[i].useDisp1settings) {
                pds[i]->renderBibleText(bibleWidget->bible.
                                      getCurrentVerseAndCaption(currentRows,theme.bible[i],
                                                                mySettings.bibleSets[i]),theme.bible[i]);
            } else {
                pds[i]->renderBibleText(bibleWidget->bible.
                                      getCurrentVerseAndCaption(currentRows,theme.bible[0],
                                                                mySettings.bibleSets[0]),theme.bible[0]);
            }
        }
    }
    if (streamOutput.settings().enabled) {
        BibleSettings streamBible;
        BibleVersionSettings versions;
        versions.primaryBible = streamOutput.settings().bibleId.isEmpty()
                ? mySettings.bibleSets[0].primaryBible : streamOutput.settings().bibleId;
        streamOutput.renderBible(bibleWidget->bible.getCurrentVerseAndCaption(
                currentRows, streamBible, versions));
    }
}

void SoftProjector::showSong(int currentRow)
{
    // Get Song Settings
    SongSettings s[4] = {theme.song[0], theme.song[1], theme.song[2], theme.song[3]};

    // Apply Song specific settings if there is one
    if(current_song.usePrivateSettings)
    {
        for (int i = 0; i < 4; ++i)
            current_song.getSettings(s[i]);
    }

    pds[0]->renderSongText(current_song.getStanza(currentRow),s[0]);
    for (int i = 1; i < 4; ++i) {
        if (hasDisplayScreen[i]) {
            if (!theme.song[i].useDisp1settings) {
                pds[i]->renderSongText(current_song.getStanza(currentRow),s[i]);
            } else {
                pds[i]->renderSongText(current_song.getStanza(currentRow),s[0]);
            }
        }
    }
    if (streamOutput.settings().enabled)
        streamOutput.renderSong(current_song.getStanza(currentRow));
}

void SoftProjector::showAnnounce(int currentRow)
{
    pds[0]->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce[0]);
    for (int i = 1; i < 4; ++i) {
        if (hasDisplayScreen[i]) {
            if (!theme.announce[i].useDisp1settings) {
                pds[i]->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce[i]);
            } else {
                pds[i]->renderAnnounceText(currentAnnounce.getAnnounceSlide(currentRow),theme.announce[0]);
            }
        }
    }
    if (streamOutput.settings().enabled)
        streamOutput.renderAnnouncement(currentAnnounce.getAnnounceSlide(currentRow));
}

void SoftProjector::showPicture(int currentRow)
{
    pds[0]->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->renderSlideShow(pictureShowList.at(currentRow).image,mySettings.slideSets);
    if (streamOutput.settings().enabled) {
        const SlideShowItem &slide = pictureShowList.at(currentRow);
        QPixmap original(slide.path);
        streamOutput.renderPicture(original.isNull() ? slide.image : original);
    }
}

void SoftProjector::showVideo()
{
    if (streamOutput.settings().enabled)
        streamOutput.clear();
    pds[0]->renderVideo(currentVideo);
    pds[0]->setVideoVolume(100);
    for (int i = 1; i < 4; ++i) {
        if (hasDisplayScreen[i]) {
            pds[i]->setVideoVolume(0);
            pds[i]->setVideoMuted(true);
            pds[i]->renderVideo(currentVideo);
        }
    }
}

void SoftProjector::on_actionShow_triggered()
{
    showing = true;
    updateScreen();
}

void SoftProjector::on_actionHide_triggered()
{
    showing = false;
    updateScreen();
}

void SoftProjector::on_actionClear_triggered()
{
    if (streamOutput.settings().enabled)
        streamOutput.clear();
    pds[0]->renderNotText();
    for (int i = 1; i < 4; ++i)
        if (hasDisplayScreen[i])
            pds[i]->renderNotText();
    ui->actionClear->setEnabled(false);
    ui->actionShow->setEnabled(true);
//    ui->actionHide->setEnabled(false);
}

void SoftProjector::on_actionCloseDisplay_triggered()
{
    if(ui->actionCloseDisplay->isChecked())
    {
        pds[0]->showFullScreen();
        for (int i = 1; i < 4; ++i)
            if (hasDisplayScreen[i])
                pds[i]->showFullScreen();
    }
    else
    {
        pds[0]->hide();
        for (int i = 1; i < 4; ++i)
            if (hasDisplayScreen[i])
                pds[i]->hide();
        showing = false;
        if (streamOutput.settings().enabled)
            streamOutput.clear();
    }

    updateCloseDisplayButtons(ui->actionCloseDisplay->isChecked());
}

void SoftProjector::updateCloseDisplayButtons(bool isOn)
{
    if(isOn)
        ui->actionCloseDisplay->setIcon(QIcon(":/icons/icons/display_on.png"));
    else
        ui->actionCloseDisplay->setIcon(QIcon(":/icons/icons/display_off.png"));
}

void SoftProjector::on_actionSettings_triggered()
{
    settingsDialog->loadSettings(mySettings.general,theme,mySettings.slideSets,
                                 mySettings.bibleSets.data(), streamOutput.settings());
    settingsDialog->exec();
}

void SoftProjector::on_listShow_doubleClicked(QModelIndex index)
{
    // Called when the user double clicks on a row in the preview table.
    showing = true;
    updateScreen();
}

void SoftProjector::on_projectTab_currentChanged(int index)
{
    updateEditActions();
}

void SoftProjector::updateEditActions()
{
    int ctab = ui->projectTab->currentIndex();
    // ctab - 0=bible, 1=songs, 2=pix, 3=media, 4=annouce
    if(ctab == 0)
    {
        ui->actionNew->setText("");
        ui->actionEdit->setText("");
        ui->actionCopy->setText("");
        ui->actionDelete->setText(tr("&Clear Bible History List"));
        ui->actionNew->setIcon(QIcon());
        ui->actionEdit->setIcon(QIcon());
        ui->actionCopy->setIcon(QIcon());
        ui->actionDelete->setIcon(QIcon(":/icons/icons/bibleHistoryDelete.png"));
    }
    else if(ctab == 1) // Song Tab
    {
        ui->actionNew->setText(tr("&New Song..."));
        ui->actionEdit->setText(tr("&Edit Song..."));
        ui->actionCopy->setText(tr("&Copy Song..."));
        ui->actionDelete->setText(tr("&Delete Song"));
        ui->actionNew->setIcon(QIcon(":/icons/icons/song_new.png"));
        ui->actionEdit->setIcon(QIcon(":/icons/icons/song_edit.png"));
        ui->actionCopy->setIcon(QIcon(":/icons/icons/song_copy.png"));
        ui->actionDelete->setIcon(QIcon(":/icons/icons/song_delete.png"));
    }
    else if (ctab == 2) // Picture Tab
    {
        ui->actionNew->setText(tr("&New Slide Show..."));
        ui->actionEdit->setText(tr("&Edit Slide Show..."));
        ui->actionCopy->setText("");
        ui->actionDelete->setText(tr("&Delete Slide Show"));
        ui->actionNew->setIcon(QIcon(":/icons/icons/slideshow_new.png"));
        ui->actionEdit->setIcon(QIcon(":/icons/icons/slideshow_edit.png"));
        ui->actionCopy->setIcon(QIcon());
        ui->actionDelete->setIcon(QIcon(":/icons/icons/slideshow_delete.png"));
    }
    else if (ctab == 3) // Media Tab
    {
        ui->actionNew->setText(tr("&Add Media Files..."));
        ui->actionEdit->setText("");
        ui->actionCopy->setText("");
        ui->actionDelete->setText(tr("&Remove Media Files"));
        ui->actionNew->setIcon(QIcon(":/icons/icons/video_add.png"));
        ui->actionEdit->setIcon(QIcon());
        ui->actionCopy->setIcon(QIcon());
        ui->actionDelete->setIcon(QIcon(":/icons/icons/video_remove.png"));
    }
    else if (ctab == 4) // Announcement Tab
    {
        ui->actionNew->setText(tr("&New Announcement..."));
        ui->actionEdit->setText(tr("&Edit Announcement..."));
        ui->actionCopy->setText(tr("&Copy Announcement..."));
        ui->actionDelete->setText(tr("&Delete Announcement"));
        ui->actionNew->setIcon(QIcon(":/icons/icons/announce_new.png"));
        ui->actionEdit->setIcon(QIcon(":/icons/icons/announce_edit.png"));
        ui->actionCopy->setIcon(QIcon(":/icons/icons/announce_copy.png"));
        ui->actionDelete->setIcon(QIcon(":/icons/icons/announce_delete.png"));
    }
    else
    {
        ui->actionNew->setText("");
        ui->actionEdit->setText("");
        ui->actionCopy->setText("");
        ui->actionDelete->setText("");
        ui->actionNew->setIcon(QIcon());
        ui->actionEdit->setIcon(QIcon());
        ui->actionCopy->setIcon(QIcon());
        ui->actionDelete->setIcon(QIcon());
    }


    // Set Edit Action Menu Visibility
    ui->actionNew->setVisible(ctab == 1 || ctab == 2 || ctab == 3 || ctab == 4);
    ui->actionEdit->setVisible(ctab == 1 || ctab == 2 || ctab == 4);
    ui->actionCopy->setVisible(ctab == 1 || ctab == 4);
    ui->actionDelete->setVisible(ctab == 0 || ctab == 1 || ctab == 2 || ctab == 3 || ctab == 4);

    // Set Edit Action Menu enabled
    ui->actionNew->setEnabled(ctab == 1 || ctab == 2 || ctab == 3 || ctab == 4);
    ui->actionEdit->setEnabled(ctab == 1 || ctab == 2 || ctab == 4);
    ui->actionCopy->setEnabled(ctab == 1 || ctab == 4);
    ui->actionDelete->setEnabled(ctab == 0 || ctab == 1 || ctab == 2 || ctab == 3 || ctab == 4);

    /////////////////////////////////////////
    // Set Print Action Menu enabled
    ui->actionPrint->setEnabled(ctab == 0 || ctab == 1 || ctab == 4);
}

void SoftProjector::on_actionNew_triggered()
{
    int ctab = ui->projectTab->currentIndex();
    if(ctab == 1)
        newSong();
    else if(ctab == 2)
        newSlideShow();
    else if(ctab == 3)
        addMediaToLibrary();
    else if(ctab == 4)
        newAnnouncement();
}

void SoftProjector::on_actionEdit_triggered()
{
    int ctab = ui->projectTab->currentIndex();
    if(ctab == 1)
        editSong();
    else if(ctab == 2)
        editSlideShow();
    else if(ctab == 4)
        editAnnouncement();
}

void SoftProjector::on_actionCopy_triggered()
{
    int ctab = ui->projectTab->currentIndex();
    if(ctab == 1)
        copySong();
    else if(ctab == 4)
        copyAnnouncement();
}

void SoftProjector::on_actionDelete_triggered()
{
    int ctab = ui->projectTab->currentIndex();
    if(ctab == 0)
        bibleWidget->clearHistory();
    else if(ctab == 1)
        deleteSong();
    else if(ctab == 2)
        deleteSlideShow();
    else if(ctab == 3)
        removeMediaFromLibrary();
    else if(ctab == 4)
        deleteAnnoucement();
}

void SoftProjector::on_actionManage_Database_triggered()
{
    QSqlQuery sq;

    manageDialog->loadThemes();
    manageDialog->load_songbooks();
    manageDialog->setDataDir(appDataDir);
    manageDialog->exec();

    // Reload songbooks if Songbook has been added, edited, or deleted
    if (manageDialog->reload_songbook)
        songWidget->updateSongbooks();

    // Relaod themes if a theme has been deleted
    if (manageDialog->reloadThemes)
    {
        // Check if current theme has been deleted
        sq.exec("SELECT * FROM Themes WHERE id = " + QString::number(theme.getThemeId()));
        if(!sq.first())
        {
            GeneralSettings g = mySettings.general;
            Theme t;
            sq.exec("SELECT id FROM Themes");
            sq.first();
            t.setThemeId(sq.value(0).toInt());
            t.loadTheme();
            g.currentThemeId = t.getThemeId();
            BibleVersionSettings bsets[4] = {mySettings.bibleSets[0], mySettings.bibleSets[1], mySettings.bibleSets[2], mySettings.bibleSets[3]};
            updateSetting(g,t,mySettings.slideSets,bsets);
            updateScreen();
        }
    }

    // Reload Bibles if Bible has been deleted
    if (manageDialog->reload_bible)
    {
        // check if Primary bible has been removed
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets[0].primaryBible);
        if (!sq.first())
        {
            // If original primary bible has been removed, set first bible in the list to be primary
            sq.clear();
            sq.exec("SELECT id FROM BibleVersions");
            sq.first();
            mySettings.bibleSets[0].primaryBible = sq.value(0).toString();
        }
        sq.clear();

        // check if secondary bible has been removed, if yes, set secondary to "none"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets[0].secondaryBible);
        if (!sq.first())
            mySettings.bibleSets[0].secondaryBible = "none";
        sq.clear();

        // check if trinary bible has been removed, if yes, set secondary to "none"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets[0].trinaryBible);
        if (!sq.first())
            mySettings.bibleSets[0].trinaryBible = "none";
        sq.clear();

        // check if operator bible has been removed, if yes, set secondary to "same"
        sq.exec("SELECT * FROM BibleVersions WHERE id = " + mySettings.bibleSets[0].operatorBible);
        if (!sq.first())
            mySettings.bibleSets[0].operatorBible = "same";
        bibleWidget->setSettings(mySettings.bibleSets[0]);
    }
}

void SoftProjector::on_actionDonate_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=FPCLPXFMH9XP4"));
}

void SoftProjector::on_actionAbout_triggered()
{
    AboutDialog *aboutDialog;
    aboutDialog = new AboutDialog(this, version_string);
    aboutDialog->exec();
    delete aboutDialog;
}

void SoftProjector::on_action_Help_triggered()
{
    //helpDialog->show();
     QDesktopServices::openUrl(QUrl("http://softprojector.org/help/index.html"));
}

void SoftProjector::newSong()
{
    if (!editWidget->isHidden()) //Prohibits editing a song when a different song already been edited.
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Cannot create a new song"));
        ms.setText(tr("Another song is already been edited."));
        ms.setInformativeText(tr("Please save and/or close current edited song before edited a different song."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
    else
    {
        editWidget->show();
        editWidget->setNew();
        editWidget->activateWindow();
    }
}

void SoftProjector::editSong()
{
    if (songWidget->isSongSelected())
    {
        if(!editWidget->isHidden()) //Prohibits editing a song when a different song already been edited.
        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("Cannot start new edit"));
            ms.setText(tr("Another song is already been edited."));
            ms.setInformativeText(tr("Please save and/or close current edited song before edited a different song."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
        else
        {
            editWidget->show();
            editWidget->setEdit(songWidget->getSongToEdit());
            editWidget->activateWindow();
        }
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No song selected"));
        ms.setText(tr("No song has been selected to edit."));
        ms.setInformativeText(tr("Please select a song to edit."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::copySong()
{
    if (songWidget->isSongSelected())
    {
        if (!editWidget->isHidden()) //Prohibits editing a song when a different song already been edited.
        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("Cannot copy this song"));
            ms.setText(tr("Another song is already been edited."));
            ms.setInformativeText(tr("Please save and/or close current edited song before edited a different song."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
        else
        {
            editWidget->show();
            editWidget->setCopy(songWidget->getSongToEdit());
            editWidget->activateWindow();
        }
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No song selected"));
        ms.setText(tr("No song has been selected to copy"));
        ms.setInformativeText(tr("Please select a song to copy"));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::deleteSong()
{
    if (songWidget->isSongSelected())
    {
        QString song_title = songWidget->currentSong().title;
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Delete song?"));
        ms.setText(tr("Delete song \"") + song_title + "\"?");
        ms.setInformativeText(tr("This action will permanentrly delete this song"));
        ms.setIcon(QMessageBox::Question);
        ms.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        ms.setDefaultButton(QMessageBox::Yes);
        int ret = ms.exec();

        switch (ret)
        {
        case QMessageBox::Yes:
            // Delete a song
            songWidget->deleteSong();
            break;
        case QMessageBox::No:
            // Cancel was clicked
            break;
        default:
            // should never be reached
            break;
        }
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No song selected"));
        ms.setText(tr("No song has been selected to delete"));
        ms.setInformativeText(tr("Please select a song to delete"));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::newSlideShow()
{
    SlideShowEditor * sse = new SlideShowEditor;
    sse->setSettings(mySettings.slideSets);
    sse->exec();
    pictureWidget->loadSlideShows();
    delete sse;
}

void SoftProjector::editSlideShow()
{
    if(pictureWidget->isSlideShowSelected())
    {
        SlideShowEditor * sse = new SlideShowEditor;
        sse->setSettings(mySettings.slideSets);
        sse->setSlideShow(pictureWidget->getCurrentSlideshow());
        sse->exec();
        pictureWidget->loadSlideShows();
        delete sse;
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No slideshow selected"));
        ms.setText(tr("No slideshow has been selected to edit."));
        ms.setInformativeText(tr("Please select a slideshow to edit."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::deleteSlideShow()
{
    if(pictureWidget->isSlideShowSelected())
    {
        QString title = pictureWidget->getCurrentSlideshow().name;
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Delete slideshow?"));
        ms.setText(tr("Delete slideshow: \"") + title + "\"?");
        ms.setInformativeText(tr("This action will permanentrly delete this slideshow"));
        ms.setIcon(QMessageBox::Question);
        ms.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        ms.setDefaultButton(QMessageBox::Yes);
        int ret = ms.exec();

        switch (ret) {
        case QMessageBox::Yes:
            // Delete a slideshow
            pictureWidget->deleteSlideShow();
            break;
        case QMessageBox::No:
            // Cancel was clicked
            break;
        default:
            // should never be reached
            break;
        }
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No slideshow selected"));
        ms.setText(tr("No slideshow has been selected to delete."));
        ms.setInformativeText(tr("Please select a slideshow to delete."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::addMediaToLibrary()
{
    mediaPlayer->addToLibrary();
}

void SoftProjector::removeMediaFromLibrary()
{
    if(mediaPlayer->isValidMedia())
    mediaPlayer->removeFromLibrary();
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No media selected"));
        ms.setText(tr("No media item has been selected to delete."));
        ms.setInformativeText(tr("Please select a media item to delete."));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::newAnnouncement()
{
    announceWidget->newAnnouncement();
}

void SoftProjector::editAnnouncement()
{
    if(announceWidget->isAnnounceValid())
        announceWidget->editAnnouncement();
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No Announcement Selected"));
        ms.setText(tr("No announcement has been selected to edit"));
        ms.setInformativeText(tr("Please select an announcement to edit"));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::copyAnnouncement()
{
    if(announceWidget->isAnnounceValid())
        announceWidget->copyAnnouncement();
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No Announcement Selected"));
        ms.setText(tr("No announcement has been selected to copy"));
        ms.setInformativeText(tr("Please select an announcement to copy"));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::deleteAnnoucement()
{
    if(announceWidget->isAnnounceValid())
    {
        QString title = announceWidget->getAnnouncement().title;
        QMessageBox ms(this);
        ms.setWindowTitle(tr("Delete Announcement?"));
        ms.setText(tr("Delete announcement: \"") + title + "\"?");
        ms.setInformativeText(tr("This action will permanentrly delete this announcement"));
        ms.setIcon(QMessageBox::Question);
        ms.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        ms.setDefaultButton(QMessageBox::Yes);
        int ret = ms.exec();

        switch (ret) {
        case QMessageBox::Yes:
            // Delete a announce
            announceWidget->deleteAnnouncement();
            break;
        case QMessageBox::No:
            // Cancel was clicked
            break;
        default:
            // should never be reached
            break;
        }
    }
    else
    {
        QMessageBox ms(this);
        ms.setWindowTitle(tr("No Announcement Selected"));
        ms.setText(tr("No announcement has been selected to delete"));
        ms.setInformativeText(tr("Please select an announcement to delete"));
        ms.setIcon(QMessageBox::Information);
        ms.exec();
    }
}

void SoftProjector::setArrowCursor()
{
    this->setCursor(Qt::ArrowCursor);
}

void SoftProjector::setWaitCursor()
{
    this->setCursor(Qt::WaitCursor);
}

void SoftProjector::createLanguageActions()
{
    // find all *.qm files at language folder
     // and create coresponding action in language menu

        languageGroup = new QActionGroup(this);
        //default language and flag
        QAction *englishAction = new QAction(QIcon(":/icons/icons/flag_uk.png"), "English", this);
        englishAction->setCheckable(true);
        englishAction->setChecked(true);
        englishAction->setIconVisibleInMenu(true);
        languageGroup->addAction(englishAction);
        ui->menuLanguage->addAction(englishAction);

        QDir languageDir(languagePath);
        //all available languages
        QStringList languagesList = languageDir.entryList(QStringList("softpro_*.qm"), QDir::Files);
        //all available flags
        QStringList flagsList = languageDir.entryList(QStringList("flag_*.png"), QDir::Files);

        foreach(QString agent, languagesList)
        {
            // local translator for taken original language's name
            QTranslator tmpTranslator;
            tmpTranslator.load(agent, languageDir.absolutePath());
            // this string are used for detection language' name
            // this is one of translated strings

            QString fullLanguageName = tmpTranslator.translate("Native language name", "English","Do not change");
            QAction *tmpAction = new QAction(fullLanguageName, this);

            QString splocale = agent.remove(0, agent.indexOf('_')+1);
            splocale.chop(3);

            // flag's file name
            QString flagFileName = "flag_"+splocale+".png";
            if(flagsList.contains(flagFileName))//  if flag is available
            {
                tmpAction->setIcon(QIcon(languageDir.absolutePath() + QDir::separator() + flagFileName));
                tmpAction->setIconVisibleInMenu(true);
            }

            tmpAction->setData(splocale);// information abount localization
            tmpAction->setCheckable(true);
            languageGroup->addAction(tmpAction);
            ui->menuLanguage->addAction(tmpAction);
        }
}

void SoftProjector::switchLanguage(QAction *action)
{
    cur_locale = action->data().toString();
    retranslateUis();
}

void SoftProjector::retranslateUis()
{
    qApp->removeTranslator(&translator);
    if(cur_locale != "en")
    {
        translator.load("softpro_"+cur_locale+".qm", QDir(languagePath).absolutePath());
        // qt libs translator must be add there,
        // but where are qt_locale.qm files?
        qApp->installTranslator(&translator);
    }

    ui->retranslateUi(this);
    ui->projectTab->setTabText(0, tr("Bible (F6)"));
    ui->projectTab->setTabText(1, tr("Songs (F7)"));
    ui->projectTab->setTabText(2, tr("Pictures"));
//    ui->projectTab->setTabText(3, tr("Media"));
    ui->projectTab->setTabText(3, tr("Announcements (F8)"));
    updateEditActions();
    songWidget->retranslateUis();
    editWidget->retranslateUis();
}

void SoftProjector::on_actionSong_Counter_triggered()
{
    SongCounter *songCounter;
    songCounter = new SongCounter(this, cur_locale);
    songCounter->exec();
    delete songCounter;
}

void SoftProjector::updateWindowText()
{
    QFileInfo fi(schedule_file_path);
    QString file = fi.fileName();

    if(!file.isEmpty())
    {
        file.remove(".spsc");
        if(is_schedule_saved)
            this->setWindowTitle(file + " - SoftProjector " + version_string);
        else
            this->setWindowTitle(file + "* - SoftProjector " + version_string);
    }
    else
    {
        this->setWindowTitle("SoftProjector " + version_string);
    }
}

void SoftProjector::on_rbMultiVerse_toggled(bool checked)
{
    if(checked)
        ui->listShow->setSelectionMode(QAbstractItemView::ContiguousSelection);
    else
        ui->listShow->setSelectionMode(QAbstractItemView::SingleSelection);
}

void SoftProjector::on_actionPrint_triggered()
{
    PrintPreviewDialog* p;
    p = new PrintPreviewDialog(this);
    if(ui->projectTab->currentIndex() == 0)
    {
        p->setText(mySettings.bibleSets[0].operatorBible + "," + mySettings.bibleSets[0].primaryBible,
                   bibleWidget->getCurrentBook(),bibleWidget->getCurrentChapter());
        p->exec();
    }
    else if (ui->projectTab->currentIndex() == 1)
    {
        if (songWidget->isSongSelected())
        {
            p->setText(songWidget->getSongToEdit());
            p->exec();

        }
        else
        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("No song selected"));
            ms.setText(tr("No song has been selected to be printed."));
            ms.setInformativeText(tr("Please select a song to be printed."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
    }
    else if (ui->projectTab->currentIndex() == 4)
    {
        if(announceWidget->isAnnounceValid())
        {
            p->setText(announceWidget->getAnnouncement());
            p->exec();
        }
        else
        {
            QMessageBox ms(this);
            ms.setWindowTitle(tr("No announcement selected"));
            ms.setText(tr("No announcement has been selected to be printed."));
            ms.setInformativeText(tr("Please select a announcement to be printed."));
            ms.setIcon(QMessageBox::Information);
            ms.exec();
        }
    }
    delete p;
}

void SoftProjector::on_actionPrintSchedule_triggered()
{
    PrintPreviewDialog* p = new PrintPreviewDialog(this);
    p->setSchedule(schedule_file_path,schedule,false);
    p->exec();
    delete p;
}

void SoftProjector::nextSlide()
{
    // selects next item in the show list
    int current = ui->listShow->currentRow();
    if(ui->rbMultiVerse->isChecked())
    {
        // if multiple is selected, select last one
        for (int i(0);i<ui->listShow->count();++i)
        {
            if(ui->listShow->item(i)->isSelected())
                current = i;
        }
        if(current < ui->listShow->count()-1)
            ui->rbMultiVerse->setChecked(false);
    }
    if(current < ui->listShow->count()-1)
        ui->listShow->setCurrentRow(current+1);
}

void SoftProjector::prevSlide()
{
    // selects previous item in the show list
    int current = ui->listShow->currentRow();
    if(ui->rbMultiVerse->isChecked())
    {
        // if multiple is selected, select first one
        for (int i(0);i<ui->listShow->count();++i)
        {
            if(ui->listShow->item(i)->isSelected())
            {
                current = i;
                break;
            }
        }
        if(current>0)
            ui->rbMultiVerse->setChecked(false);
    }
    if(current>0)
        ui->listShow->setCurrentRow(current-1);
}

//void SoftProjector::on_pushButtonPlay_clicked()
//{
    /*
     * if(pds1->videoPlayer->state() == Phonon::PlayingState)
    {
        pds1->videoPlayer->pause();
        if(hasDisplayScreen2)
            pds2->videoPlayer->pause();
    }
    else
    {
        if(pds1->videoPlayer->currentTime() == pds1->videoPlayer->totalTime())
        {
            pds1->videoPlayer->seek(0);
            if(hasDisplayScreen2)
                pds2->videoPlayer->seek(0);
        }
        pds1->videoPlayer->play();
        if(hasDisplayScreen2)
            pds2->videoPlayer->play();
    }*/
//}

void SoftProjector::on_actionScheduleAdd_triggered()
{
    int ctab = ui->projectTab->currentIndex();
    if(ctab == 0) // Bible
    {
        if(bibleWidget->isVerseSelected())
        {
            BibleHistory b = bibleWidget->getCurrentVerse();
            addToShcedule(b);
        }
    }
    else if(ctab == 1) // Song
    {
        if(songWidget->isSongSelected())
        {
            Song s = songWidget->getSongToEdit();
            addToShcedule(s);
        }
    }
    else if(ctab == 2) // Slide Show
    {
        if(pictureWidget->isSlideShowSelected())
        {
            SlideShow ssi = pictureWidget->getCurrentSlideshow();
            addToShcedule(ssi);
        }
    }
    else if(ctab == 3) // Multimedia
    {
        if(mediaPlayer->isValidMedia())
        {
            VideoInfo v = mediaPlayer->getMedia();
            addToShcedule(v);
        }
    }
    else if(ctab == 4)
    {
        if(announceWidget->isAnnounceValid())
        {
            Announcement a = announceWidget->getAnnouncement();
            addToShcedule(a);
        }
    }
    int row_count = ui->listWidgetSchedule->count();
    ui->listWidgetSchedule->setCurrentRow(row_count-1);
}

void SoftProjector::on_actionScheduleRemove_triggered()
{
    int cRow = ui->listWidgetSchedule->currentRow();
    if(cRow >=0)
    {
        schedule.takeAt(cRow);
        reloadShceduleList();
    }
}

void SoftProjector::on_actionScheduleClear_triggered()
{
    schedule.clear();
    reloadShceduleList();
}

void SoftProjector::addToShcedule(BibleHistory &b)
{
    Schedule d(b);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(Song &s)
{
    Schedule d(s);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(SlideShow &s)
{
    Schedule d(s);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(VideoInfo &v)
{
    Schedule d(v);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::addToShcedule(Announcement &a)
{
    Schedule d(a);
    schedule.append(d);
    reloadShceduleList();
}

void SoftProjector::reloadShceduleList()
{
    ui->listWidgetSchedule->setCurrentRow(-1);
    ui->listWidgetSchedule->clear();
    foreach (const Schedule &s, schedule)
    {
        QListWidgetItem *itm = new QListWidgetItem;
        itm->setIcon(s.icon);
        itm->setText(s.name);
        ui->listWidgetSchedule->addItem(itm);
    }
    is_schedule_saved = false;
    updateWindowText();
}

void SoftProjector::on_listWidgetSchedule_doubleClicked(const QModelIndex &index)
{
    Schedule s = schedule.at(index.row());
    if(s.stype == "bible")
    {
        ui->projectTab->setCurrentIndex(0);
        bibleWidget->setSelectedHistory(s.bible);
        bibleWidget->sendToProjector(true);
    }
    else if(s.stype == "song")
    {
        ui->projectTab->setCurrentIndex(1);
        songWidget->sendToPreviewFromSchedule(s.song);
        setSongList(s.song,0);
        songWidget->counter.addSongCount(s.song);
    }
    else if(s.stype == "slideshow")
    {
        ui->projectTab->setCurrentIndex(2);
        pictureWidget->sendToPreviewFromSchedule(s.slideshow);
        setPictureList(s.slideshow.slides,0,s.slideshow.name);
    }
    else if(s.stype == "media")
    {
        ui->projectTab->setCurrentIndex(3);
        mediaPlayer->setMediaFromSchedule(s.media);
        mediaPlayer->goLiveFromSchedule();
    }
    else if(s.stype == "announce")
    {
        ui->projectTab->setCurrentIndex(4);
        announceWidget->setAnnouncementFromHistory(s.announce);
        setAnnounceText(s.announce,0);
    }
}

void SoftProjector::on_listWidgetSchedule_itemSelectionChanged()
{
    int currentRow = ui->listWidgetSchedule->currentRow();
    if(currentRow>=0)
    {
        Schedule s = schedule.at(currentRow);
        if(s.stype == "bible")
        {
            ui->projectTab->setCurrentIndex(0);
            bibleWidget->setSelectedHistory(s.bible);
        }
        else if(s.stype == "song")
        {
            ui->projectTab->setCurrentIndex(1);
            songWidget->sendToPreviewFromSchedule(s.song);
        }
        else if(s.stype == "slideshow")
        {
            ui->projectTab->setCurrentIndex(2);
            pictureWidget->sendToPreviewFromSchedule(s.slideshow);
        }
        else if(s.stype == "media")
        {
            ui->projectTab->setCurrentIndex(3);
            mediaPlayer->setMediaFromSchedule(s.media);
        }
        else if(s.stype == "announce")
        {
            ui->projectTab->setCurrentIndex(4);
            announceWidget->setAnnouncementFromHistory(s.announce);
        }
    }
}

void SoftProjector::on_actionMoveScheduleTop_triggered()
{
    int cs = ui->listWidgetSchedule->currentRow();
    if(cs>0)
    {
        schedule.move(cs,0);
        reloadShceduleList();
        ui->listWidgetSchedule->setCurrentRow(0);
    }
}

void SoftProjector::on_actionMoveScheduleUp_triggered()
{
    int cs = ui->listWidgetSchedule->currentRow();
    if(cs>0)
    {
        schedule.move(cs,cs-1);
        reloadShceduleList();
        ui->listWidgetSchedule->setCurrentRow(cs-1);
    }
}

void SoftProjector::on_actionMoveScheduleDown_triggered()
{
    int cs = ui->listWidgetSchedule->currentRow();
    int max = schedule.count();
    if(cs>=0 && cs<max-1)
    {
        schedule.move(cs,cs+1);
        reloadShceduleList();
        ui->listWidgetSchedule->setCurrentRow(cs+1);
    }
}

void SoftProjector::on_actionMoveScheduleBottom_triggered()
{
    int cs = ui->listWidgetSchedule->currentRow();
    int max = schedule.count();
    if(cs>=0 && cs<max-1)
    {
        schedule.move(cs,max-1);
        reloadShceduleList();
        ui->listWidgetSchedule->setCurrentRow(max-1);
    }
}

void SoftProjector::on_actionNewSchedule_triggered()
{
    if (!is_schedule_saved && !confirmSaveSchedule(tr("Do you want to save current schedule before creating a new schedule?")))
        return;

    schedule_file_path = "untitled.spsc";
    schedule.clear();
    reloadShceduleList();
    is_schedule_saved = false;
    updateWindowText();
}

void SoftProjector::on_actionOpenSchedule_triggered()
{
    if (!is_schedule_saved && !confirmSaveSchedule(tr("Do you want to save current schedule before opening a new schedule?")))
        return;

    QString path = QFileDialog::getOpenFileName(this,tr("Open SoftProjector schedule:"),".",
                                                tr("SoftProjector schedule file ") + "(*.spsc)");
    if(!path.isEmpty())
    {
        if (openSchedule(path)) {
            schedule_file_path = path;
            is_schedule_saved = true;
            updateWindowText();
        }
    }
}

void SoftProjector::on_actionSaveSchedule_triggered()
{
    if(schedule_file_path.isEmpty() || schedule_file_path.startsWith("untitled"))
        on_actionSaveScheduleAs_triggered();
    else
        saveSchedule(schedule_file_path);
    updateWindowText();
}

void SoftProjector::on_actionSaveScheduleAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(this,tr("Save SoftProjector schedule as:"),".",
                                                tr("SoftProjector schedule file ") + "(*.spsc)");
    if(!path.isEmpty())
    {
        const QString target = path.endsWith(".spsc") ? path : path + ".spsc";
        if (saveSchedule(target))
            schedule_file_path = target;
    }

    updateWindowText();
}

void SoftProjector::on_actionCloseSchedule_triggered()
{
    if (!is_schedule_saved && !confirmSaveSchedule(tr("Do you want to save current schedule before closing it?")))
        return;

    schedule_file_path.clear();
    schedule.clear();
    reloadShceduleList();
    is_schedule_saved = true;
    updateWindowText();
}

bool SoftProjector::confirmSaveSchedule(const QString &question)
{
    const auto choice = QMessageBox::question(this, tr("Save Schedule?"), question,
                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                               QMessageBox::Save);
    if (choice == QMessageBox::Discard)
        return true;
    if (choice != QMessageBox::Save)
        return false;
    on_actionSaveSchedule_triggered();
    return is_schedule_saved;
}

bool SoftProjector::saveSchedule(const QString &path)
{
    // Build beside the destination so replacing it never destroys the old file on failure.
    QTemporaryDir tempDir(QFileInfo(path).absolutePath() + "/.softprojector-XXXXXX");
    const QString tempPath = tempDir.path() + "/schedule.spsc";
    const bool updating = path == schedule_file_path && QFile::exists(path);
    if (!tempDir.isValid() || (updating && !QFile::copy(path, tempPath))) {
        QMessageBox::warning(this, tr("Save Schedule"), tr("Could not prepare schedule file for saving."));
        return false;
    }

    const QList<Schedule> original = schedule;
    if (!updating)
        for (Schedule &item : schedule)
            item.scid = -1;

    bool saved = false;
    QString error;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","spsc");
        db.setDatabaseName(tempPath);
        if(db.open())
        {
            QSqlQuery sq(db);
            saved = db.transaction();
            saved = saved && sq.exec("PRAGMA user_version = 2");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'schedule' ('id' INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL, "
                    "'stype' TEXT, 'name' TEXT, 'sorder' INTEGER )");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'bible' ('scid' INTEGER, 'verseIds' TEXT, 'caption' TEXT, 'captionLong' TEXT)");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'song' ('scid' INTEGER, 'songid' INTEGER, 'sbid' INTEGER, 'sbName' TEXT, "
                    "'number' INTEGER, 'title' TEXT, 'category' INTEGER, 'tune' TEXT, 'wordsBy' TEXT, 'musicBy' TEXT, "
                    "'songText' TEXT, 'notes' TEXT, 'usePrivate' BOOL, 'alignV' INTEGER, 'alignH' INTEGER, 'color' INTEGER, "
                    "'font' TEXT, 'infoColor' INTEGER, 'infoFont' TEXT, 'endingColor' INTEGER, 'endingFont' TEXT, "
                    "'useBack' BOOL, 'backImage' BLOB, 'backName' TEXT)");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'slideshow' ('scid' INTEGER, 'ssid' INTEGER, 'name' TEXT, 'info' TEXT)");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'slides' ('scid' INTEGER, 'sid' INTEGER, 'name' TEXT, 'path' TEXT, "
                    "'porder' INTEGER, 'image' BLOB, 'imageSmall' BLOB, 'imagePreview' BLOB)");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'media' ('scid' INTEGER, 'name' TEXT, 'path' TEXT, 'aRatio' INTEGER)");
            saved = saved && sq.exec("CREATE TABLE IF NOT EXISTS 'announce' ('scid' INTEGER, 'aId' INTEGER, 'title' TEXT, 'aText' TEXT, "
                    "'usePrivate' BOOL, 'useAuto' BOOL, 'loop' BOOL, 'slideTimer' INTEGER, 'font' TEXT, 'color' INTEGER, "
                    "'useBack' BOOL, 'backImage' BLOB, 'backPath' TEXT, 'alignV' INTEGER, 'alignH' INTEGER)");
            saved = saved && saveScheduleUpdate(sq);
            if (saved)
                saved = db.commit();
            else
                db.rollback();
            error = sq.lastError().text();
        }
        else
            error = db.lastError().text();
        db.close();
    }
    QSqlDatabase::removeDatabase("spsc");
    if (saved) {
        std::error_code ec;
        std::filesystem::rename(std::filesystem::u8path(tempPath.toUtf8().constData()),
                                std::filesystem::u8path(QFileInfo(path).absoluteFilePath().toUtf8().constData()), ec);
        if (ec) {
            saved = false;
            error = QString::fromStdString(ec.message());
        }
    }
    if (!saved) {
        schedule = original;
        QMessageBox::warning(this, tr("Save Schedule"), tr("Could not save schedule: %1").arg(error));
        return false;
    }
    is_schedule_saved = true;
    return true;
}

bool SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const BibleHistory &b)
{
    q.prepare("INSERT INTO bible (scid,verseIds,caption,captionLong) VALUES(?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(b.verseIds);
    q.addBindValue(b.caption);
    q.addBindValue(b.captionLong);
    return q.exec();
}

bool SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const Song &s)
{
    q.prepare("INSERT INTO song (scid,songid,sbid,sbName,number,title,category,tune,wordsBy,musicBy,"
              "songText,notes,usePrivate,alignV,alignH,color,font,infoColor,infoFont,endingColor,"
              "endingFont,useBack,backImage,backName) "
              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(s.songID);
    q.addBindValue(s.songbook_id);
    q.addBindValue(s.songbook_name);
    q.addBindValue(s.number);
    q.addBindValue(s.title);
    q.addBindValue(s.category);
    q.addBindValue(s.tune);
    q.addBindValue(s.wordsBy);
    q.addBindValue(s.musicBy);
    q.addBindValue(s.songText);
    q.addBindValue(s.notes);
    q.addBindValue(s.usePrivateSettings);
    q.addBindValue(s.alignmentV);
    q.addBindValue(s.alignmentH);
    q.addBindValue((unsigned int)(s.color.rgb()));
    q.addBindValue(s.font.toString());
    q.addBindValue((unsigned int)(s.infoColor.rgb()));
    q.addBindValue(s.infoFont.toString());
    q.addBindValue((unsigned int)(s.endingColor.rgb()));
    q.addBindValue(s.endingFont.toString());
    q.addBindValue(s.useBackground);
    q.addBindValue(pixToByte(s.background));
    q.addBindValue(s.backgroundName);
    return q.exec();
}

bool SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const SlideShow &s)
{
    q.prepare("INSERT INTO slideshow (scid,ssid,name,info) VALUES (?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(s.slideShowId);
    q.addBindValue(s.name);
    q.addBindValue(s.info);
    if (!q.exec())
        return false;

    foreach(const SlideShowItem & si,s.slides)
    {
        q.prepare("INSERT INTO slides (scid,sid,name,path,porder,image,imageSmall,imagePreview) VALUES(?,?,?,?,?,?,?,?)");
        q.addBindValue(scid);
        q.addBindValue(si.slideId);
        q.addBindValue(si.name);
        q.addBindValue(si.path);
        q.addBindValue(si.order);
        q.addBindValue(pixToByte(si.image));
        q.addBindValue(pixToByte(si.imageSmall));
        q.addBindValue(pixToByte(si.imagePreview));
        if (!q.exec())
            return false;
    }
    return true;
}

bool SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const VideoInfo &v)
{
    q.prepare("INSERT INTO media (scid,name,path,aRatio) VALUES(?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(v.fileName);
    q.addBindValue(v.filePath);
    q.addBindValue(v.aspectRatio);
    return q.exec();
}

bool SoftProjector::saveScheduleItemNew(QSqlQuery &q, int scid, const Announcement &a)
{
    q.prepare("INSERT INTO announce (scid,aId,title,aText,usePrivate,useAuto,loop,slideTimer,font,"
                        "color,useBack,backImage,backPath,alignV,alignH) "
                        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(scid);
    q.addBindValue(a.idNum);
    q.addBindValue(a.title);
    q.addBindValue(a.text);
    q.addBindValue(a.usePrivateSettings);
    q.addBindValue(a.useAutoNext);
    q.addBindValue(a.loop);
    q.addBindValue(a.slideTimer);
    q.addBindValue(a.font.toString());
    unsigned int tci = (unsigned int)(a.color.rgb());
    q.addBindValue(tci);
    q.addBindValue(a.useBackground);
    q.addBindValue(pixToByte(QPixmap()));
    q.addBindValue(a.backgroundPath);
    q.addBindValue(a.alignmentV);
    q.addBindValue(a.alignmentH);
    return q.exec();
}

bool SoftProjector::saveScheduleUpdate(QSqlQuery &q)
{
    for(int i(0);i < schedule.count();++i)
    {
        Schedule sc =  schedule.at(i);
        if(sc.scid == -1)   // Save new schedule item that was not saved yet
        {
            q.prepare("INSERT INTO schedule (stype,name,sorder) VALUES (?,?,?)");
            q.addBindValue(sc.stype);
            q.addBindValue(sc.name);
            q.addBindValue(i+1);
            if (!q.exec())
                return false;
            sc.scid = q.lastInsertId().toInt();
            bool ok = false;
            if(sc.stype == "bible")
                ok = saveScheduleItemNew(q,sc.scid,sc.bible);
            else if(sc.stype == "song")
                ok = saveScheduleItemNew(q,sc.scid,sc.song);
            else if(sc.stype == "slideshow")
                ok = saveScheduleItemNew(q,sc.scid,sc.slideshow);
            else if(sc.stype == "media")
                ok = saveScheduleItemNew(q,sc.scid,sc.media);
            else if(sc.stype == "announce")
                ok = saveScheduleItemNew(q,sc.scid,sc.announce);
            if (!ok)
                return false;
            schedule.replace(i,sc);
        }
        else    // Update existing schedule item
        {
            q.prepare("UPDATE schedule SET sorder = ? WHERE id = ?");
            q.addBindValue(i+1);
            q.addBindValue(sc.scid);
            if (!q.exec())
                return false;
        }
    }

    // Delete items removed from the schedule, including when it is now empty.
    if (!q.exec("SELECT id,stype FROM schedule"))
        return false;
    QSqlQuery sq(QSqlDatabase::database("spsc"));
    while(q.next())
    {
        int scid = q.value(0).toInt();
        bool toDelete = true;
        QString stype = q.value(1).toString();
        foreach(const Schedule &s,schedule)
        {
            if(scid == s.scid)
            {
                toDelete = false;
                break;
            }
        }

        if(toDelete)
        {
            // Table names come from our own schedule types, not from SQL input.
            if (stype != "bible" && stype != "song" && stype != "slideshow" &&
                stype != "media" && stype != "announce")
                return false;
            if (!sq.exec("DELETE FROM schedule WHERE id = " + QString::number(scid)) ||
                !sq.exec("DELETE FROM " + stype + " WHERE scid = " + QString::number(scid)) ||
                (stype == "slideshow" && !sq.exec("DELETE FROM slides WHERE scid = " + QString::number(scid))))
                return false;
        }
    }
    return true;
}

bool SoftProjector::openSchedule(const QString &path)
{
    QProgressDialog progress;
    progress.setMaximum(0);
    progress.setLabelText(tr("Opening schedule file..."));
    progress.show();
    bool loaded = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","spsc");
        db.setDatabaseName(path);
        if(db.open())
        {
            QSqlQuery sq(db);
            const int scVer = sq.exec("PRAGMA user_version") && sq.first() ? sq.value(0).toInt() : -1;
            if(scVer == 2 && sq.exec("SELECT id, stype, name FROM schedule ORDER BY sorder"))
            {
                schedule.clear();
                QSqlQuery sqsc(db);
                while(sq.next())
                {
                    int scid = sq.value(0).toInt();
                    QString stype = sq.value(1).toString();
                    QString name = sq.value(2).toString();

                    if(stype == "bible")
                    {
                        BibleHistory bib;
                        openScheduleItem(sqsc,scid,bib);
                        Schedule sc(bib);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "song")
                    {
                        Song song;
                        openScheduleItem(sqsc,scid,song);
                        Schedule sc(song);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "slideshow")
                    {
                        SlideShow ss;
                        openScheduleItem(sqsc,scid,ss);
                        Schedule sc(ss);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "media")
                    {
                        VideoInfo vi;
                        openScheduleItem(sqsc,scid,vi);
                        Schedule sc(vi);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                    else if(stype == "announce")
                    {
                        Announcement announce;
                        openScheduleItem(sqsc,scid,announce);
                        Schedule sc(announce);
                        sc.name = name;
                        sc.scid = scid;
                        schedule.append(sc);
                    }
                }
                reloadShceduleList();
                loaded = true;
            }
            else
            {
                QMessageBox mb(this);
                mb.setText(tr("The schedule file you are trying to open is of uncompatible version.\n"
                              "Compatible version: 2\n"
                              "This schedule file version: ") + QString::number(scVer));
                mb.setIcon(QMessageBox::Information);
                mb.setStandardButtons(QMessageBox::Ok);
                mb.exec();
            }
        }
        else
            QMessageBox::warning(this, tr("Open Schedule"), db.lastError().text());
        db.close();
    }
    QSqlDatabase::removeDatabase("spsc");
    progress.close();
    return loaded;
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, BibleHistory &b)
{
    q.exec("SELECT verseIds, caption, captionLong FROM bible WHERE scid = " + QString::number(scid));
    q.first();
    b.verseIds = q.value(0).toString();
    b.caption = q.value(1).toString();
    b.captionLong = q.value(2).toString();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, Song &s)
{
    q.exec("SELECT * FROM song WHERE scid = " + QString::number(scid));
    q.first();
    QSqlRecord r = q.record();
    s.songID = r.field("songid").value().toInt();
    s.songbook_id = r.field("sbid").value().toString();
    s.songbook_name = r.field("sbName").value().toString();
    s.number = r.field("number").value().toInt();
    s.title = r.field("title").value().toString();
    s.category = r.field("category").value().toInt();
    s.tune = r.field("tune").value().toString();
    s.wordsBy = r.field("wordsBy").value().toString();
    s.musicBy = r.field("musicBy").value().toString();
    s.songText = r.field("songText").value().toString();
    s.notes = r.field("notes").value().toString();
    s.usePrivateSettings = r.field("usePrivate").value().toBool();
    s.alignmentV = r.field("alignV").value().toInt();
    s.alignmentH = r.field("alignH").value().toInt();
    s.color = QColor::fromRgb(r.field("color").value().toUInt());
    s.font.fromString(r.field("font").value().toString());
    s.infoColor = QColor::fromRgb(r.field("infoColor").value().toUInt());
    s.infoFont.fromString(r.field("infoFont").value().toString());
    s.endingColor = QColor::fromRgb(r.field("endingColor").value().toUInt());
    s.endingFont.fromString(r.field("endingFont").value().toString());
    s.useBackground = r.field("useBack").value().toBool();
    s.background.loadFromData(r.field("backImage").value().toByteArray());
    s.backgroundName = r.field("backName").value().toString();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, SlideShow &s)
{
    q.exec("SELECT ssid, name, info FROM slideshow WHERE scid = " + QString::number(scid));
    q.first();
    s.slideShowId = q.value(0).toInt();
    s.name = q.value(1).toString();
    s.info = q.value(2).toString();

    q.exec("SELECT sid, name, path, porder, image, imageSmall, imagePreview FROM slides WHERE scid = " + QString::number(scid));
    while(q.next())
    {
        SlideShowItem si;
        si.slideId = q.value(0).toInt();
        si.name = q.value(1).toString();
        si.path = q.value(2).toString();
        si.order = q.value(3).toInt();
        si.image.loadFromData(q.value(4).toByteArray());
        si.imageSmall.loadFromData(q.value(5).toByteArray());
        si.imagePreview.loadFromData(q.value(6).toByteArray());
        s.slides.append(si);
    }
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, VideoInfo &v)
{
    q.exec("SELECT name, path, aRatio FROM media WHERE scid = " + QString::number(scid));
    q.first();
    v.fileName = q.value(0).toString();
    v.filePath = q.value(1).toString();
    v.aspectRatio = q.value(2).toInt();
}

void SoftProjector::openScheduleItem(QSqlQuery &q, const int scid, Announcement &a)
{
    q.exec("SELECT aId, title, aText, usePrivate, useAuto, loop, slideTimer, font, color, useBack, "
           "backImage, backPath, alignV, alignH FROM announce WHERE scid = " + QString::number(scid));
    q.first();
    a.idNum = q.value(0).toInt();
    a.title = q.value(1).toString();
    a.text = q.value(2).toString();
    a.usePrivateSettings = q.value(3).toBool();
    a.useAutoNext = q.value(4).toBool();
    a.loop = q.value(5).toBool();
    a.slideTimer = q.value(6).toInt();
    a.font.fromString(q.value(7).toString());
    a.color = QColor::fromRgb(q.value(8).toUInt());
    a.useBackground = q.value(9).toBool();
//    a.backImage = QPixmap
    a.backgroundPath = q.value(11).toString();
    a.alignmentV = q.value(12).toInt();
    a.alignmentH = q.value(13).toInt();
}
