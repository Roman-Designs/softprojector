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

#include <QMediaMetaData>

#include "../headers/mediawidget.hpp"
#include "ui_mediawidget.h"

MediaWidget::MediaWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaWidget)//,
{
    ui->setupUi(this);
    isReadyToPlay = false;
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(player);
    player->setAudioOutput(audioOutput);

    mediaControls = new MediaControl(this);
     ui->horizontalLayoutControls->addWidget(mediaControls);

    connect(player, &QMediaPlayer::metaDataChanged, this, &MediaWidget::updateInfo);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MediaWidget::statusChanged);
//    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, &QMediaPlayer::hasVideoChanged, this, &MediaWidget::hasVideoChanged);
//    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));


    connect(mediaControls, &MediaControl::muted, audioOutput, &QAudioOutput::setMuted);
    connect(mediaControls, SIGNAL(play()),player,SLOT(play()));
    connect(mediaControls, SIGNAL(pause()),player,SLOT(pause()));
    connect(mediaControls, SIGNAL(stop()),player,SLOT(stop()));
    connect(mediaControls, SIGNAL(timeChanged(qint64)),player,SLOT(setPosition(qint64)));
    connect(mediaControls, &MediaControl::volumeChanged, this,
            [this](int level) { audioOutput->setVolume(level / 100.0); });


    connect(player, &QMediaPlayer::playbackStateChanged, mediaControls, &MediaControl::updatePlayerState);
    connect(player, &QMediaPlayer::errorOccurred, this, &MediaWidget::displayErrorMessage);
    connect(player, SIGNAL(durationChanged(qint64)), mediaControls, SLOT(setMaximumTime(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), mediaControls, SLOT(updateTime(qint64)));
  videoWidget = new VideoPlayerWidget(this);
    player->setVideoOutput(videoWidget);

    mediaControls->setVolume(100);


       ui->verticalLayoutMedia->addWidget(videoWidget);
        videoWidget->setVisible(false);




    /**********************************************/

    ui->pushButtonGoLive->setEnabled(false);

    playIcon = QIcon(":icons/icons/play.png");
    pauseIcon = QIcon(":icons/icons/pause.png");
    muteIcon = QIcon(":icons/icons/speakerMute.png");
    unmuteIcon = QIcon(":icons/icons/speaker.png");

    QPalette palette;
    palette.setBrush(QPalette::WindowText, Qt::white);

    ui->labelInfo->setStyleSheet("border-image:url(:icons/icons/playerScreen.png)");
    ui->labelInfo->setPalette(palette);
    ui->labelInfo->setText(tr("<center>No media</center>"));


//    connect(&mediaPlayer, SIGNAL(metaDataChanged()), this, SLOT(updateInfo()));
//    connect(videoWidget, SIGNAL(handleDrops(QDropEvent*)), this, SLOT(handleDrop(QDropEvent*)));

    setAcceptDrops(true);

    audioExt = "*.mp3 *.acc *.ogg *.oga *.wma *.wav *.asf *.mka";
    videoExt = "*.wmv *.avi *.mkv *.flv *.mp4 *.mpg *.mpeg *.mov *.ogv *.ts";
    loadMediaLibrary();
}

MediaWidget::~MediaWidget()
{
    delete player;
    delete videoWidget;
    delete mediaControls;
//    delete timeSlider;
//    delete volumeSlider;
    delete ui;
}

void MediaWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MediaWidget::loadMediaLibrary()
{
    QSqlQuery sq;
    sq.exec("SELECT * FROM Media");
    while(sq.next())
    {
        mediaFilePaths.append(QUrl::fromLocalFile(sq.value(0).toString()));
        mediaFileNames.append(sq.value(1).toString());
        ui->listWidgetMediaFiles->clear();
        ui->listWidgetMediaFiles->addItems(mediaFileNames);
    }
}

void MediaWidget::statusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug()<<"statusChanged"<<status;
    switch (status) {
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::LoadingMedia:
        ui->labelInfo->setText(QString("<center><strong><font color=#49fff9>%1</font>")
                               .arg(tr("Loading...")));
        break;
    case QMediaPlayer::StalledMedia:
        ui->labelInfo->setText(QString("<center><strong><font color=#49fff9>%1</font>")
                               .arg(tr("Media Stalled")));
        break;
    case QMediaPlayer::InvalidMedia:
        schedulePlaybackPending = false;
        displayErrorMessage();
        break;
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferedMedia:
        isReadyToPlay = true;
        if (schedulePlaybackPending) {
            schedulePlaybackPending = false;
            goLiveFromSchedule();
        }
        break;
    }
}

void MediaWidget::displayErrorMessage()
{
    QString errMsg1 = tr("Possible Fail reasons:");
    QString errMsg2 = tr(" - Unsupported media format");
    QString errMsg3 = tr(" - Media file no longer exists or invalid path to file");

    if(!player->errorString().isEmpty())
    {
        errMsg1 = player->errorString();
        errMsg2 = errMsg3 = "";
    }
    ui->labelInfo->setText(QString("<center><strong><font color=#ff5555>%1:</font></strong><br>"
                                   "<font color=#49fff9>%2</font><br>%3<br>%4<br>%5</center>")
                           .arg(tr("ERROR Playing media file"))
                           .arg(currentMediaUrl.fileName())
                           .arg(errMsg1)
                           .arg(errMsg2)
                           .arg(errMsg3));
}

void MediaWidget::handleDrop(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    QStringList fileList;
    QStringList fileNameList;

    // Make sure that only supported files get to media player
    QString fext = audioExt + " " + videoExt;
    fext.remove("*");
    fext.replace(" ","$|");
    fext.append("$");
    QRegularExpression rx(fext);
    rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    foreach(const QUrl &url, urls)
    {
        QString fp = url.toLocalFile();
        if(fp.contains(rx))
        {
            QFileInfo fn(fp);
            fileList.append(fp);
            fileNameList.append(fn.fileName());
        }
    }

    // Add files to library
    if(fileList.count()>0)
    {
        int mcount = ui->listWidgetMediaFiles->count(); // get total media count
        insertFiles(fileList);
        ui->listWidgetMediaFiles->setCurrentRow(mcount); // select first in list of just added and play it
    }
}

void MediaWidget::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls() && e->proposedAction() != Qt::LinkAction) {
        e->acceptProposedAction();
        handleDrop(e);
    } else {
        e->ignore();
    }
}

void MediaWidget::dragEnterEvent(QDragEnterEvent *e)
{
    dragMoveEvent(e);
}

void MediaWidget::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        if (e->proposedAction() == Qt::CopyAction || e->proposedAction() == Qt::MoveAction){
            e->acceptProposedAction();
        }
    }
}


void MediaWidget::playFile(QUrl filePath)
{
    isReadyToPlay = false;
    schedulePlaybackPending = false;
    player->stop();
    currentMediaUrl = filePath;
    QUrl m(filePath);
    player->setSource(m);
}

void MediaWidget::updateInfo()
{
    int maxLength = 50;
    QString font = "<font color=#49fff9>";

    QString fName = player->source().fileName();
    QMediaMetaData md = player->metaData();
    QString tAlbum = md.stringValue(QMediaMetaData::AlbumTitle);
    QString tTitle = md.stringValue(QMediaMetaData::Title);
    QString tArtist = md.stringValue(QMediaMetaData::AlbumArtist);
    int tBitrate = md.stringValue(QMediaMetaData::AudioBitRate).toInt();

    if (fName.length() > maxLength)
        fName = fName.left(maxLength) + "...";

    if (tAlbum.length() > maxLength)
        tAlbum = tAlbum.left(maxLength) + "...";

    if (tTitle.length() > maxLength)
        tTitle = tTitle.left(maxLength) + "...";

    if (tArtist.length() > maxLength)
        tArtist = tArtist.left(maxLength) + "...";

    QString file;
    if(!fName.isEmpty())
        file = "File: " + font + fName + "<br></font>";

    QString album;
    if(!tAlbum.isEmpty())
        album = "Album: " + font + tAlbum + "<br></font>";

    QString title;
    if(!tTitle.isEmpty())
    {
        title = "Title: " + font + tTitle + "<br></font>";
        file.clear();
    }

    QString artist;
    if (!tArtist.isEmpty())
        artist = "Artist: " + font + tArtist + "<br></font>";

    QString bitrate;
    if (tBitrate != 0)
        bitrate = "Bitrate: " + font + QString::number(tBitrate/1000) + "kbit</font>";

    ui->labelInfo->setText(file + album + title + artist + bitrate);

}

void MediaWidget::insertFiles(QStringList &files)
{
    // Insert filenames and path into appropriate lists and database
    QSqlQuery sq;
    foreach(const QString &file, files)
    {
        QFileInfo f(file);
        mediaFileNames.append(f.fileName());
        mediaFilePaths.append(QUrl::fromLocalFile(file));
        sq.prepare("INSERT INTO Media (long_path, short_path) VALUES (?, ?)");
        sq.addBindValue(file);
        sq.addBindValue(f.fileName());
        sq.exec();
        ui->listWidgetMediaFiles->addItem(f.fileName());
    }
}

void MediaWidget::hasVideoChanged(bool bHasVideo)
{
    qDebug()<<"hasVideoChanged"<<bHasVideo;
    if(!bHasVideo && videoWidget->isFullScreen())
        videoWidget->setFullScreen(false);
    ui->labelInfo->setVisible(!bHasVideo);
    ui->pushButtonGoLive->setEnabled(bHasVideo);
    videoWidget->setVisible(bHasVideo);
//    isReadyToPlay = true;
}

void MediaWidget::prepareForProjection()
{
    player->pause();
    VideoInfo v;
//    v.fileName = mediaFileNames.at(ui->listWidgetMediaFiles->currentRow());
//    v.filePath = mediaFilePaths.at(ui->listWidgetMediaFiles->currentRow());
    v.fileName = currentMediaUrl.fileName();
    v.filePath = currentMediaUrl.toString();
    emit toProjector(v);
}

void MediaWidget::on_pushButtonGoLive_clicked()
{
    prepareForProjection();
}

void MediaWidget::addToLibrary()
{
    QStringList mfp = QFileDialog::getOpenFileNames(this,tr("Select Music/Video Files to Open"),".",
                                                    tr("Media Files (%1);;Audio Files (%2);;Video Files (%3)")
                                                    .arg(audioExt + " " + videoExt) // media files
                                                    .arg(audioExt) // audio files
                                                    .arg(videoExt)); // video files

    if(mfp.count()>0)
        insertFiles(mfp);
}

void MediaWidget::removeFromLibrary()
{
    int cm = ui->listWidgetMediaFiles->currentRow();
    if(cm>=0)
    {
        QSqlQuery sq;
        sq.prepare("DELETE FROM Media WHERE long_path = ?");
        sq.addBindValue(mediaFilePaths.at(cm).toLocalFile());
        sq.exec();
        mediaFilePaths.removeAt(cm);
        mediaFileNames.removeAt(cm);

        ui->listWidgetMediaFiles->setCurrentRow(-1);
        ui->listWidgetMediaFiles->clear();
        if(mediaFileNames.count()>0)
            ui->listWidgetMediaFiles->addItems(mediaFileNames);
        player->stop();

        hasVideoChanged(false);

        ui->labelInfo->setText(tr("<center>No media</center>"));
    }
}

void MediaWidget::on_listWidgetMediaFiles_itemSelectionChanged()
{
    int cRow = ui->listWidgetMediaFiles->currentRow();
    if(cRow>=0)
    {
        playFile(mediaFilePaths.at(cRow));
    }
}

void MediaWidget::on_listWidgetMediaFiles_doubleClicked(const QModelIndex &index)
{
    if(ui->pushButtonGoLive->isEnabled())
        prepareForProjection();
}

VideoInfo MediaWidget::getMedia()
{
    VideoInfo v;
    int cm = ui->listWidgetMediaFiles->currentRow();
    v.fileName = mediaFileNames.at(cm);
    v.filePath = mediaFilePaths.at(cm);
    return v;
}

void MediaWidget::setMediaFromSchedule(VideoInfo &v)
{
    if(currentMediaUrl == v.filePath)
    {
        // Same current Media File, do not update.
        return;
    }
    ui->listWidgetMediaFiles->clearSelection();
    playFile(v.filePath.scheme().isEmpty() ? QUrl::fromLocalFile(v.filePath.toString()) : v.filePath);
    player->pause();
}

void MediaWidget::goLiveFromSchedule()
{
    if (!isReadyToPlay) {
        schedulePlaybackPending = true;
        return;
    }
    qDebug()<<videoWidget->isVisible()<<ui->pushButtonGoLive->isEnabled();
    if(ui->pushButtonGoLive->isEnabled())
    {
        qDebug()<<"Go Live is Enabled";
        prepareForProjection();
    }
    else
    {
        qDebug()<<"Start Playing";
        player->play();
    }
}

bool MediaWidget::isValidMedia()
{
    if(ui->listWidgetMediaFiles->selectionModel()->selectedRows().count() > 0)
        return true;
    else
        return false;
}
