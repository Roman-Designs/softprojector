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
****************************************************************************/

#include "../headers/announcementsettingwidget.hpp"
#include "ui_announcementsettingwidget.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QFile>

AnnouncementSettingWidget::AnnouncementSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnnouncementSettingWidget)
{
    ui->setupUi(this);
}

AnnouncementSettingWidget::~AnnouncementSettingWidget()
{
    delete ui;
}

void AnnouncementSettingWidget::changeEvent(QEvent *e)
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

void AnnouncementSettingWidget::setSettings(TextSettings &settings, TextSettings &settings2, TextSettings &settings3, TextSettings &settings4)
{
    mySettings = settings;
    mySettings2 = settings2;
    mySettings3 = settings3;
    mySettings4 = settings4;
    loadSettings();
}

void AnnouncementSettingWidget::loadSettings()
{
    // Set Effects
    ui->checkBoxUseShadow->setChecked(mySettings.useShadow);
    ui->checkBoxUseFading->setChecked(mySettings.useFading);
    ui->checkBoxUseBlurredShadow->setChecked(mySettings.useBlurShadow);

    ui->checkBoxUseShadow2->setChecked(mySettings2.useShadow);
    ui->checkBoxUseFading2->setChecked(mySettings2.useFading);
    ui->checkBoxUseBlurredShadow2->setChecked(mySettings2.useBlurShadow);

    ui->checkBoxUseShadow3->setChecked(mySettings3.useShadow);
    ui->checkBoxUseFading3->setChecked(mySettings3.useFading);
    ui->checkBoxUseBlurredShadow3->setChecked(mySettings3.useBlurShadow);

    ui->checkBoxUseShadow4->setChecked(mySettings4.useShadow);
    ui->checkBoxUseFading4->setChecked(mySettings4.useFading);
    ui->checkBoxUseBlurredShadow4->setChecked(mySettings4.useBlurShadow);

    // Set background type radio buttons and visibility
    setBackgroundTypeRadio(1, mySettings.backgroundType);
    setBackgroundTypeRadio(2, mySettings2.backgroundType);
    setBackgroundTypeRadio(3, mySettings3.backgroundType);
    setBackgroundTypeRadio(4, mySettings4.backgroundType);

    // Set video background settings
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

    // Update video previews
    updateVideoPreview(1);
    updateVideoPreview(2);
    updateVideoPreview(3);
    updateVideoPreview(4);

    // Set Alignment
    ui->comboBoxVerticalAling->setCurrentIndex(mySettings.textAlignmentV);
    ui->comboBoxHorizontalAling->setCurrentIndex(mySettings.textAlignmentH);

    ui->comboBoxVerticalAling2->setCurrentIndex(mySettings2.textAlignmentV);
    ui->comboBoxHorizontalAling2->setCurrentIndex(mySettings2.textAlignmentH);

    ui->comboBoxVerticalAling3->setCurrentIndex(mySettings3.textAlignmentV);
    ui->comboBoxHorizontalAling3->setCurrentIndex(mySettings3.textAlignmentH);

    ui->comboBoxVerticalAling4->setCurrentIndex(mySettings4.textAlignmentV);
    ui->comboBoxHorizontalAling4->setCurrentIndex(mySettings4.textAlignmentH);

    // Set text color
    QPalette p;
    p.setColor(QPalette::Base,mySettings.textColor);
    ui->graphicViewTextColor->setPalette(p);

    p.setColor(QPalette::Base,mySettings2.textColor);
    ui->graphicViewTextColor2->setPalette(p);

    p.setColor(QPalette::Base,mySettings3.textColor);
    ui->graphicViewTextColor3->setPalette(p);

    p.setColor(QPalette::Base,mySettings4.textColor);
    ui->graphicViewTextColor4->setPalette(p);

    // Set text font lable
    ui->labelFont->setText(getFontText(mySettings.textFont));
    ui->labelFont2->setText(getFontText(mySettings2.textFont));
    ui->labelFont3->setText(getFontText(mySettings3.textFont));
    ui->labelFont4->setText(getFontText(mySettings4.textFont));

    ui->groupBoxUseDisp2->setChecked(!mySettings2.useDisp1settings);
    on_groupBoxUseDisp2_toggled(!mySettings2.useDisp1settings);

    ui->groupBoxUseDisp3->setChecked(!mySettings3.useDisp1settings);
    on_groupBoxUseDisp3_toggled(!mySettings3.useDisp1settings);

    ui->groupBoxUseDisp4->setChecked(!mySettings4.useDisp1settings);
    on_groupBoxUseDisp4_toggled(!mySettings4.useDisp1settings);
}

void AnnouncementSettingWidget::getSettings(TextSettings &settings, TextSettings &settings2, TextSettings &settings3, TextSettings &settings4)
{
    // Effects
    mySettings.useShadow = ui->checkBoxUseShadow->isChecked();
    mySettings.useFading = ui->checkBoxUseFading->isChecked();
    mySettings.useBlurShadow = ui->checkBoxUseBlurredShadow->isChecked();

    mySettings2.useShadow = ui->checkBoxUseShadow2->isChecked();
    mySettings2.useFading = ui->checkBoxUseFading2->isChecked();
    mySettings2.useBlurShadow = ui->checkBoxUseBlurredShadow2->isChecked();

    mySettings3.useShadow = ui->checkBoxUseShadow3->isChecked();
    mySettings3.useFading = ui->checkBoxUseFading3->isChecked();
    mySettings3.useBlurShadow = ui->checkBoxUseBlurredShadow3->isChecked();

    mySettings4.useShadow = ui->checkBoxUseShadow4->isChecked();
    mySettings4.useFading = ui->checkBoxUseFading4->isChecked();
    mySettings4.useBlurShadow = ui->checkBoxUseBlurredShadow4->isChecked();

    // Background type is already set by radio button handlers
    // Video background settings
    mySettings.backgroundVideoPath = ui->lineEditVideoPath->text();
    mySettings.backgroundVideoLoop = ui->checkBoxVideoLoop->isChecked();
    mySettings.backgroundVideoFillMode = ui->comboBoxVideoFillMode->currentIndex();

    mySettings2.backgroundVideoPath = ui->lineEditVideoPath2->text();
    mySettings2.backgroundVideoLoop = ui->checkBoxVideoLoop2->isChecked();
    mySettings2.backgroundVideoFillMode = ui->comboBoxVideoFillMode2->currentIndex();

    mySettings3.backgroundVideoPath = ui->lineEditVideoPath3->text();
    mySettings3.backgroundVideoLoop = ui->checkBoxVideoLoop3->isChecked();
    mySettings3.backgroundVideoFillMode = ui->comboBoxVideoFillMode3->currentIndex();

    mySettings4.backgroundVideoPath = ui->lineEditVideoPath4->text();
    mySettings4.backgroundVideoLoop = ui->checkBoxVideoLoop4->isChecked();
    mySettings4.backgroundVideoFillMode = ui->comboBoxVideoFillMode4->currentIndex();

    // Alignment
    mySettings.textAlignmentV = ui->comboBoxVerticalAling->currentIndex();
    mySettings.textAlignmentH = ui->comboBoxHorizontalAling->currentIndex();

    mySettings2.textAlignmentV = ui->comboBoxVerticalAling2->currentIndex();
    mySettings2.textAlignmentH = ui->comboBoxHorizontalAling2->currentIndex();

    mySettings3.textAlignmentV = ui->comboBoxVerticalAling3->currentIndex();
    mySettings3.textAlignmentH = ui->comboBoxHorizontalAling3->currentIndex();

    mySettings4.textAlignmentV = ui->comboBoxVerticalAling4->currentIndex();
    mySettings4.textAlignmentH = ui->comboBoxHorizontalAling4->currentIndex();

    mySettings2.useDisp1settings = !ui->groupBoxUseDisp2->isChecked();
    mySettings2.useDisp2settings = ui->groupBoxUseDisp2->isChecked();

    mySettings3.useDisp1settings = !ui->groupBoxUseDisp3->isChecked();
    mySettings3.useDisp3settings = ui->groupBoxUseDisp3->isChecked();

    mySettings4.useDisp1settings = !ui->groupBoxUseDisp4->isChecked();
    mySettings4.useDisp4settings = ui->groupBoxUseDisp4->isChecked();

    settings = mySettings;
    settings2 = mySettings2;
    settings3 = mySettings3;
    settings4 = mySettings4;
}

void AnnouncementSettingWidget::on_checkBoxUseShadow_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow->setChecked(false);
        ui->checkBoxUseBlurredShadow->setEnabled(false);
    }
}

void AnnouncementSettingWidget::on_checkBoxUseShadow2_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow2->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow2->setChecked(false);
        ui->checkBoxUseBlurredShadow2->setEnabled(false);
    }
}

void AnnouncementSettingWidget::on_checkBoxUseShadow3_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow3->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow3->setChecked(false);
        ui->checkBoxUseBlurredShadow3->setEnabled(false);
    }
}

void AnnouncementSettingWidget::on_checkBoxUseShadow4_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow4->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow4->setChecked(false);
        ui->checkBoxUseBlurredShadow4->setEnabled(false);
    }
}

void AnnouncementSettingWidget::on_buttonBackground_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for announcement wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings.backgroundName = filename;
        ui->lineEditBackground->setText(filename);
    }
}

void AnnouncementSettingWidget::on_buttonBackground2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for announcement wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings2.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings2.backgroundName = filename;
        ui->lineEditBackground2->setText(filename);
    }
}

void AnnouncementSettingWidget::on_buttonBackground3_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for announcement wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings3.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings3.backgroundName = filename;
        ui->lineEditBackground3->setText(filename);
    }
}

void AnnouncementSettingWidget::on_buttonBackground4_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for announcement wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings4.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings4.backgroundName = filename;
        ui->lineEditBackground4->setText(filename);
    }
}

void AnnouncementSettingWidget::setDispScreen2Visible(bool visible)
{
    ui->groupBoxUseDisp2->setVisible(visible);
}

void AnnouncementSettingWidget::setDispScreen3Visible(bool visible)
{
    ui->groupBoxUseDisp3->setVisible(visible);
}

void AnnouncementSettingWidget::setDispScreen4Visible(bool visible)
{
    ui->groupBoxUseDisp4->setVisible(visible);
}

void AnnouncementSettingWidget::on_toolButtonColor_clicked()
{
    QColor c(QColorDialog::getColor(mySettings.textColor,this));
    if(c.isValid())
        mySettings.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings.textColor);
    ui->graphicViewTextColor->setPalette(p);
}

void AnnouncementSettingWidget::on_toolButtonColor2_clicked()
{
    QColor c(QColorDialog::getColor(mySettings2.textColor,this));
    if(c.isValid())
        mySettings2.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings2.textColor);
    ui->graphicViewTextColor2->setPalette(p);
}

void AnnouncementSettingWidget::on_toolButtonColor3_clicked()
{
    QColor c(QColorDialog::getColor(mySettings3.textColor,this));
    if(c.isValid())
        mySettings3.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings3.textColor);
    ui->graphicViewTextColor3->setPalette(p);
}

void AnnouncementSettingWidget::on_toolButtonColor4_clicked()
{
    QColor c(QColorDialog::getColor(mySettings4.textColor,this));
    if(c.isValid())
        mySettings4.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings4.textColor);
    ui->graphicViewTextColor4->setPalette(p);
}

void AnnouncementSettingWidget::on_toolButtonFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings.textFont,this);
    if(ok)
        mySettings.textFont = font;

    ui->labelFont->setText(getFontText(mySettings.textFont));
}

void AnnouncementSettingWidget::on_toolButtonFont2_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings2.textFont,this);
    if(ok)
        mySettings2.textFont = font;

    ui->labelFont2->setText(getFontText(mySettings2.textFont));
}

void AnnouncementSettingWidget::on_toolButtonFont3_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings3.textFont,this);
    if(ok)
        mySettings3.textFont = font;

    ui->labelFont3->setText(getFontText(mySettings3.textFont));
}

void AnnouncementSettingWidget::on_toolButtonFont4_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings4.textFont,this);
    if(ok)
        mySettings4.textFont = font;

    ui->labelFont4->setText(getFontText(mySettings4.textFont));
}

void AnnouncementSettingWidget::on_groupBoxUseDisp2_toggled(bool arg1)
{
    ui->groupBoxBackground2->setVisible(arg1);
    ui->groupBoxEffects2->setVisible(arg1);
    ui->groupBoxTextProperties2->setVisible(arg1);
}

void AnnouncementSettingWidget::on_groupBoxUseDisp3_toggled(bool arg1)
{
    ui->groupBoxBackground3->setVisible(arg1);
    ui->groupBoxEffects3->setVisible(arg1);
    ui->groupBoxTextProperties3->setVisible(arg1);
}

void AnnouncementSettingWidget::on_groupBoxUseDisp4_toggled(bool arg1)
{
    ui->groupBoxBackground4->setVisible(arg1);
    ui->groupBoxEffects4->setVisible(arg1);
    ui->groupBoxTextProperties4->setVisible(arg1);
}

void AnnouncementSettingWidget::on_pushButtonDefault_clicked()
{
    TextSettings a;
    mySettings = a;
    mySettings2 = a;
    mySettings3 = a;
    mySettings4 = a;
    loadSettings();
}

QString AnnouncementSettingWidget::getFontText(QFont font)
{
    QString f = font.toString().split(",").at(0);
    QString st(QString("%1: %2").arg(f).arg(font.pointSize()));
    if(font.bold())
        st += ", " + tr("Bold");
    if(font.italic())
        st += ", " + tr("Italic");
    if(font.strikeOut())
        st += ", " + tr("StrikeOut");
    if(font.underline())
        st += ", " + tr("Underline");

    return st;
}

void AnnouncementSettingWidget::on_pushButtonApplyToAll_clicked()
{
    emit applyBackToAll(3,mySettings.backgroundName,mySettings.backgroundPix);
}

void AnnouncementSettingWidget::setBackgroungds(QString name, QPixmap back)
{
    mySettings.backgroundName = name;
    mySettings.backgroundPix = back;
    mySettings2.backgroundName = name;
    mySettings2.backgroundPix = back;
    mySettings3.backgroundName = name;
    mySettings3.backgroundPix = back;
    mySettings4.backgroundName = name;
    mySettings4.backgroundPix = back;
    ui->lineEditBackground->setText(name);
    ui->lineEditBackground2->setText(name);
    ui->lineEditBackground3->setText(name);
    ui->lineEditBackground4->setText(name);
}

QString AnnouncementSettingWidget::getSupportedVideoFormats()
{
    return "Video files (*.mp4 *.avi *.mov *.webm *.mkv *.flv *.wmv);;All files (*)";
}

void AnnouncementSettingWidget::on_radioButtonBgNone_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_NONE;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgSolid_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgImage_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground->setVisible(true);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgVideo_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(true);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgNone2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_NONE;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgSolid2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgImage2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground2->setVisible(true);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgVideo2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(true);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgNone3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_NONE;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgSolid3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgImage3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground3->setVisible(true);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgVideo3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(true);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgNone4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_NONE;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgSolid4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgImage4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground4->setVisible(true);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void AnnouncementSettingWidget::on_radioButtonBgVideo4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(true);
    }
}

void AnnouncementSettingWidget::on_toolButtonBrowseVideo_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", getSupportedVideoFormats());
    if(!filename.isNull())
    {
        mySettings.backgroundVideoPath = filename;
        ui->lineEditVideoPath->setText(filename);
        updateVideoPreview(1);
    }
}

void AnnouncementSettingWidget::on_toolButtonBrowseVideo2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", getSupportedVideoFormats());
    if(!filename.isNull())
    {
        mySettings2.backgroundVideoPath = filename;
        ui->lineEditVideoPath2->setText(filename);
        updateVideoPreview(2);
    }
}

void AnnouncementSettingWidget::on_toolButtonBrowseVideo3_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", getSupportedVideoFormats());
    if(!filename.isNull())
    {
        mySettings3.backgroundVideoPath = filename;
        ui->lineEditVideoPath3->setText(filename);
        updateVideoPreview(3);
    }
}

void AnnouncementSettingWidget::on_toolButtonBrowseVideo4_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a video for background"),
                                                    ".", getSupportedVideoFormats());
    if(!filename.isNull())
    {
        mySettings4.backgroundVideoPath = filename;
        ui->lineEditVideoPath4->setText(filename);
        updateVideoPreview(4);
    }
}

void AnnouncementSettingWidget::updateVideoPreview(int display)
{
    QString videoPath;
    QLabel *thumbnailLabel = nullptr;

    switch(display)
    {
    case 1:
        videoPath = mySettings.backgroundVideoPath;
        thumbnailLabel = ui->labelVideoThumbnail;
        break;
    case 2:
        videoPath = mySettings2.backgroundVideoPath;
        thumbnailLabel = ui->labelVideoThumbnail2;
        break;
    case 3:
        videoPath = mySettings3.backgroundVideoPath;
        thumbnailLabel = ui->labelVideoThumbnail3;
        break;
    case 4:
        videoPath = mySettings4.backgroundVideoPath;
        thumbnailLabel = ui->labelVideoThumbnail4;
        break;
    }

    if(!videoPath.isEmpty() && QFile::exists(videoPath))
    {
        QPixmap pix(videoPath);
        if(!pix.isNull())
        {
            thumbnailLabel->setPixmap(pix.scaled(160, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            return;
        }
    }
    thumbnailLabel->clear();
    thumbnailLabel->setText(tr("No preview"));
}

void AnnouncementSettingWidget::setBackgroundTypeRadio(int display, int type)
{
    switch(display)
    {
    case 1:
        ui->radioButtonBgNone->setChecked(type == B_NONE);
        ui->radioButtonBgSolid->setChecked(type == B_SOLID_COLOR);
        ui->radioButtonBgImage->setChecked(type == B_PICTURE);
        ui->radioButtonBgVideo->setChecked(type == B_VIDEO);
        ui->groupBoxImageBackground->setVisible(type == B_PICTURE);
        ui->groupBoxVideoBackground->setVisible(type == B_VIDEO);
        break;
    case 2:
        ui->radioButtonBgNone2->setChecked(type == B_NONE);
        ui->radioButtonBgSolid2->setChecked(type == B_SOLID_COLOR);
        ui->radioButtonBgImage2->setChecked(type == B_PICTURE);
        ui->radioButtonBgVideo2->setChecked(type == B_VIDEO);
        ui->groupBoxImageBackground2->setVisible(type == B_PICTURE);
        ui->groupBoxVideoBackground2->setVisible(type == B_VIDEO);
        break;
    case 3:
        ui->radioButtonBgNone3->setChecked(type == B_NONE);
        ui->radioButtonBgSolid3->setChecked(type == B_SOLID_COLOR);
        ui->radioButtonBgImage3->setChecked(type == B_PICTURE);
        ui->radioButtonBgVideo3->setChecked(type == B_VIDEO);
        ui->groupBoxImageBackground3->setVisible(type == B_PICTURE);
        ui->groupBoxVideoBackground3->setVisible(type == B_VIDEO);
        break;
    case 4:
        ui->radioButtonBgNone4->setChecked(type == B_NONE);
        ui->radioButtonBgSolid4->setChecked(type == B_SOLID_COLOR);
        ui->radioButtonBgImage4->setChecked(type == B_PICTURE);
        ui->radioButtonBgVideo4->setChecked(type == B_VIDEO);
        ui->groupBoxImageBackground4->setVisible(type == B_PICTURE);
        ui->groupBoxVideoBackground4->setVisible(type == B_VIDEO);
        break;
    }
}
