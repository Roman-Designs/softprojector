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
//**************************************************************************/

#include "../headers/passivesettingwidget.hpp"
#include "ui_passivesettingwidget.h"
#include <QColorDialog>
#include <QFile>
#include "spfunctions.hpp"

PassiveSettingWidget::PassiveSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PassiveSettingWidget)
{
    ui->setupUi(this);
}

PassiveSettingWidget::~PassiveSettingWidget()
{
    delete ui;
}

void PassiveSettingWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
     switch ( e->type() ) {
     case QEvent::LanguageChange:
         ui->retranslateUi(this);
         break;
     default:
         break;
     }
}

void PassiveSettingWidget::setSetings(TextSettings &settings, TextSettings &settings2, TextSettings &settings3, TextSettings &settings4)
{
    mySettings = settings;
    mySettings2 = settings2;
    mySettings3 = settings3;
    mySettings4 = settings4;
    loadSettings();
}

void PassiveSettingWidget::getSettings(TextSettings &settings, TextSettings &settings2, TextSettings &settings3, TextSettings &settings4)
{
    mySettings.backgroundType = ui->groupBoxBackground->isChecked() ? mySettings.backgroundType : B_NONE;
    mySettings.backgroundName = ui->lineEditBackgroundPath->text();
    mySettings.backgroundVideoPath = ui->lineEditVideoPath->text();
    mySettings.backgroundVideoLoop = ui->checkBoxVideoLoop->isChecked();
    mySettings.backgroundVideoFillMode = ui->comboBoxVideoFillMode->currentIndex();

    mySettings2.useDisp1settings = !ui->groupBoxDisp2Sets->isChecked();
    mySettings2.useDisp2settings = ui->groupBoxDisp2Sets->isChecked();
    mySettings2.backgroundType = ui->groupBoxBackground2->isChecked() ? mySettings2.backgroundType : B_NONE;
    mySettings2.backgroundName = ui->lineEditBackgroundPath2->text();
    mySettings2.backgroundVideoPath = ui->lineEditVideoPath2->text();
    mySettings2.backgroundVideoLoop = ui->checkBoxVideoLoop2->isChecked();
    mySettings2.backgroundVideoFillMode = ui->comboBoxVideoFillMode2->currentIndex();

    mySettings3.useDisp1settings = !ui->groupBoxDisp3Sets->isChecked();
    mySettings3.useDisp3settings = ui->groupBoxDisp3Sets->isChecked();
    mySettings3.backgroundType = ui->groupBoxBackground3->isChecked() ? mySettings3.backgroundType : B_NONE;
    mySettings3.backgroundName = ui->lineEditBackgroundPath3->text();
    mySettings3.backgroundVideoPath = ui->lineEditVideoPath3->text();
    mySettings3.backgroundVideoLoop = ui->checkBoxVideoLoop3->isChecked();
    mySettings3.backgroundVideoFillMode = ui->comboBoxVideoFillMode3->currentIndex();

    mySettings4.useDisp1settings = !ui->groupBoxDisp4Sets->isChecked();
    mySettings4.useDisp4settings = ui->groupBoxDisp4Sets->isChecked();
    mySettings4.backgroundType = ui->groupBoxBackground4->isChecked() ? mySettings4.backgroundType : B_NONE;
    mySettings4.backgroundName = ui->lineEditBackgroundPath4->text();
    mySettings4.backgroundVideoPath = ui->lineEditVideoPath4->text();
    mySettings4.backgroundVideoLoop = ui->checkBoxVideoLoop4->isChecked();
    mySettings4.backgroundVideoFillMode = ui->comboBoxVideoFillMode4->currentIndex();

    settings = mySettings;
    settings2 = mySettings2;
    settings3 = mySettings3;
    settings4 = mySettings4;
}

void PassiveSettingWidget::loadSettings()
{
    ui->groupBoxBackground->setChecked(mySettings.backgroundType != B_NONE);
    ui->lineEditBackgroundPath->setText(mySettings.backgroundName);

    ui->groupBoxBackground2->setChecked(mySettings2.backgroundType != B_NONE);
    ui->lineEditBackgroundPath2->setText(mySettings2.backgroundName);

    ui->groupBoxBackground3->setChecked(mySettings3.backgroundType != B_NONE);
    ui->lineEditBackgroundPath3->setText(mySettings3.backgroundName);

    ui->groupBoxBackground4->setChecked(mySettings4.backgroundType != B_NONE);
    ui->lineEditBackgroundPath4->setText(mySettings4.backgroundName);

    setBackgroundTypeRadio(1, mySettings.backgroundType);
    setBackgroundTypeRadio(2, mySettings2.backgroundType);
    setBackgroundTypeRadio(3, mySettings3.backgroundType);
    setBackgroundTypeRadio(4, mySettings4.backgroundType);

    ui->lineEditVideoPath->setText(mySettings.backgroundVideoPath);
    ui->checkBoxVideoLoop->setChecked(mySettings.backgroundVideoLoop);
    ui->comboBoxVideoFillMode->setCurrentIndex(mySettings.backgroundVideoFillMode);

    ui->lineEditVideoPath2->setText(mySettings2.backgroundVideoPath);
    ui->checkBoxVideoLoop2->setChecked(mySettings2.backgroundVideoLoop);
    ui->comboBoxVideoFillMode2->setCurrentIndex(mySettings2.backgroundVideoFillMode);

    ui->lineEditVideoPath3->setText(mySettings3.backgroundVideoPath);
    ui->checkBoxVideoLoop3->setChecked(mySettings3.backgroundVideoLoop);
    ui->comboBoxVideoFillMode3->setCurrentIndex(mySettings3.backgroundVideoFillMode);

    ui->lineEditVideoPath4->setText(mySettings4.backgroundVideoPath);
    ui->checkBoxVideoLoop4->setChecked(mySettings4.backgroundVideoLoop);
    ui->comboBoxVideoFillMode4->setCurrentIndex(mySettings4.backgroundVideoFillMode);

    updateVideoPreview(1);
    updateVideoPreview(2);
    updateVideoPreview(3);
    updateVideoPreview(4);

    ui->groupBoxDisp2Sets->setChecked(!mySettings2.useDisp1settings);
    ui->groupBoxDisp3Sets->setChecked(!mySettings3.useDisp1settings);
    ui->groupBoxDisp4Sets->setChecked(!mySettings4.useDisp1settings);
}

void PassiveSettingWidget::setDispScreen2Visible(bool visible)
{
    ui->groupBoxDisp2Sets->setVisible(visible);
}

void PassiveSettingWidget::setDispScreen3Visible(bool visible)
{
    ui->groupBoxDisp3Sets->setVisible(visible);
}

void PassiveSettingWidget::setDispScreen4Visible(bool visible)
{
    ui->groupBoxDisp4Sets->setVisible(visible);
}

void PassiveSettingWidget::on_buttonBrowseBackgound_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for passive wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings.backgroundName = filename;
        ui->lineEditBackgroundPath->setText(filename);
    }
}

void PassiveSettingWidget::on_groupBoxDisp2Sets_toggled(bool arg1)
{
    ui->groupBoxBackground2->setVisible(arg1);
}

void PassiveSettingWidget::on_groupBoxDisp3Sets_toggled(bool arg1)
{
    ui->groupBoxBackground3->setVisible(arg1);
}

void PassiveSettingWidget::on_groupBoxDisp4Sets_toggled(bool arg1)
{
    ui->groupBoxBackground4->setVisible(arg1);
}

void PassiveSettingWidget::on_buttonBrowseBackgound2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for passive wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings2.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings2.backgroundName = filename;
        ui->lineEditBackgroundPath2->setText(filename);
    }
}

void PassiveSettingWidget::on_buttonBrowseBackgound3_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for passive wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings3.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings3.backgroundName = filename;
        ui->lineEditBackgroundPath3->setText(filename);
    }
}

void PassiveSettingWidget::on_buttonBrowseBackgound4_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for passive wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings4.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings4.backgroundName = filename;
        ui->lineEditBackgroundPath4->setText(filename);
    }
}

void PassiveSettingWidget::on_pushButtonDefault_clicked()
{
    TextSettings p;
    mySettings = p;
    mySettings2 = p;
    mySettings3 = p;
    mySettings4 = p;
    loadSettings();
}

void PassiveSettingWidget::on_radioButtonBgNone_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_NONE;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgSolid_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgImage_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground->setVisible(true);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgVideo_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(true);
    }
}

void PassiveSettingWidget::on_radioButtonBgNone2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_NONE;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgSolid2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgImage2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground2->setVisible(true);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgVideo2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(true);
    }
}

void PassiveSettingWidget::on_radioButtonBgNone3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_NONE;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgSolid3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgImage3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground3->setVisible(true);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgVideo3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(true);
    }
}

void PassiveSettingWidget::on_radioButtonBgNone4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_NONE;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgSolid4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgImage4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground4->setVisible(true);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void PassiveSettingWidget::on_radioButtonBgVideo4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(true);
    }
}

void PassiveSettingWidget::on_toolButtonBrowseVideo_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", tr("Videos(%1)").arg(getSupportedVideoFormats()));
    if(!filename.isNull())
    {
        mySettings.backgroundVideoPath = filename;
        ui->lineEditVideoPath->setText(filename);
        updateVideoPreview(1);
    }
}

void PassiveSettingWidget::on_toolButtonBrowseVideo2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", tr("Videos(%1)").arg(getSupportedVideoFormats()));
    if(!filename.isNull())
    {
        mySettings2.backgroundVideoPath = filename;
        ui->lineEditVideoPath2->setText(filename);
        updateVideoPreview(2);
    }
}

void PassiveSettingWidget::on_toolButtonBrowseVideo3_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", tr("Videos(%1)").arg(getSupportedVideoFormats()));
    if(!filename.isNull())
    {
        mySettings3.backgroundVideoPath = filename;
        ui->lineEditVideoPath3->setText(filename);
        updateVideoPreview(3);
    }
}

void PassiveSettingWidget::on_toolButtonBrowseVideo4_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", tr("Videos(%1)").arg(getSupportedVideoFormats()));
    if(!filename.isNull())
    {
        mySettings4.backgroundVideoPath = filename;
        ui->lineEditVideoPath4->setText(filename);
        updateVideoPreview(4);
    }
}

void PassiveSettingWidget::updateVideoPreview(int display)
{
    QLabel *thumbnail = nullptr;
    QString videoPath;

    switch(display)
    {
    case 1:
        thumbnail = ui->labelVideoThumbnail;
        videoPath = mySettings.backgroundVideoPath;
        break;
    case 2:
        thumbnail = ui->labelVideoThumbnail2;
        videoPath = mySettings2.backgroundVideoPath;
        break;
    case 3:
        thumbnail = ui->labelVideoThumbnail3;
        videoPath = mySettings3.backgroundVideoPath;
        break;
    case 4:
        thumbnail = ui->labelVideoThumbnail4;
        videoPath = mySettings4.backgroundVideoPath;
        break;
    }

    if(thumbnail && QFile::exists(videoPath))
    {
        QPixmap pix(videoPath);
        if(!pix.isNull())
        {
            thumbnail->setPixmap(pix.scaled(thumbnail->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            thumbnail->setText(tr("Cannot load video thumbnail"));
        }
    }
    else if(thumbnail)
    {
        thumbnail->clear();
    }
}

void PassiveSettingWidget::setBackgroundTypeRadio(int display, int type)
{
    QRadioButton *radioNone = nullptr;
    QRadioButton *radioSolid = nullptr;
    QRadioButton *radioImage = nullptr;
    QRadioButton *radioVideo = nullptr;
    QGroupBox *groupImage = nullptr;
    QGroupBox *groupVideo = nullptr;

    switch(display)
    {
    case 1:
        radioNone = ui->radioButtonBgNone;
        radioSolid = ui->radioButtonBgSolid;
        radioImage = ui->radioButtonBgImage;
        radioVideo = ui->radioButtonBgVideo;
        groupImage = ui->groupBoxImageBackground;
        groupVideo = ui->groupBoxVideoBackground;
        break;
    case 2:
        radioNone = ui->radioButtonBgNone2;
        radioSolid = ui->radioButtonBgSolid2;
        radioImage = ui->radioButtonBgImage2;
        radioVideo = ui->radioButtonBgVideo2;
        groupImage = ui->groupBoxImageBackground2;
        groupVideo = ui->groupBoxVideoBackground2;
        break;
    case 3:
        radioNone = ui->radioButtonBgNone3;
        radioSolid = ui->radioButtonBgSolid3;
        radioImage = ui->radioButtonBgImage3;
        radioVideo = ui->radioButtonBgVideo3;
        groupImage = ui->groupBoxImageBackground3;
        groupVideo = ui->groupBoxVideoBackground3;
        break;
    case 4:
        radioNone = ui->radioButtonBgNone4;
        radioSolid = ui->radioButtonBgSolid4;
        radioImage = ui->radioButtonBgImage4;
        radioVideo = ui->radioButtonBgVideo4;
        groupImage = ui->groupBoxImageBackground4;
        groupVideo = ui->groupBoxVideoBackground4;
        break;
    }

    if(radioNone && radioSolid && radioImage && radioVideo && groupImage && groupVideo)
    {
        switch(type)
        {
        case B_NONE:
            radioNone->setChecked(true);
            groupImage->setVisible(false);
            groupVideo->setVisible(false);
            break;
        case B_SOLID_COLOR:
            radioSolid->setChecked(true);
            groupImage->setVisible(false);
            groupVideo->setVisible(false);
            break;
        case B_PICTURE:
            radioImage->setChecked(true);
            groupImage->setVisible(true);
            groupVideo->setVisible(false);
            break;
        case B_VIDEO:
            radioVideo->setChecked(true);
            groupImage->setVisible(false);
            groupVideo->setVisible(true);
            break;
        default:
            radioNone->setChecked(true);
            groupImage->setVisible(false);
            groupVideo->setVisible(false);
            break;
        }
    }
}

QString PassiveSettingWidget::getSupportedVideoFormats()
{
    return "*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm *.m4v *.mpeg *.mpg";
}
