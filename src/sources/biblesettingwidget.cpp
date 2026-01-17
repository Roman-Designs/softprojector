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

#include "../headers/biblesettingwidget.hpp"
#include "ui_biblesettingwidget.h"
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QFile>

BibleSettingWidget::BibleSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BibleSettingWidget)
{
    ui->setupUi(this);
}

BibleSettingWidget::~BibleSettingWidget()
{
    delete ui;
}

void BibleSettingWidget::changeEvent(QEvent *e)
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

void BibleSettingWidget::setSettings(BibleSettings &settings, BibleSettings &settings2, BibleSettings &settings3, BibleSettings &settings4)
{
    mySettings = settings;
    mySettings2 = settings2;
    mySettings3 = settings3;
    mySettings4 = settings4;

    loadSettings();
}

void BibleSettingWidget::getSettings(BibleSettings &settings, BibleSettings &settings2, BibleSettings &settings3, BibleSettings &settings4)
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

    // Background
    mySettings.useBackground = ui->groupBoxBackground->isChecked();
    mySettings.backgroundName = ui->lineEditBackPath->text();

    mySettings2.useBackground = ui->groupBoxBackground2->isChecked();
    mySettings2.backgroundName = ui->lineEditBackPath2->text();

    mySettings3.useBackground = ui->groupBoxBackground3->isChecked();
    mySettings3.backgroundName = ui->lineEditBackPath3->text();

    mySettings4.useBackground = ui->groupBoxBackground4->isChecked();
    mySettings4.backgroundName = ui->lineEditBackPath4->text();

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

    // Background Color 4 Text Group
    mySettings.bibleAddBKColorToText = ui->groupBoxAddBackgroundToText->isChecked();
    mySettings2.bibleAddBKColorToText = ui->groupBoxAddBackgroundToText2->isChecked();
    mySettings3.bibleAddBKColorToText = ui->groupBoxAddBackgroundToText3->isChecked();
    mySettings4.bibleAddBKColorToText = ui->groupBoxAddBackgroundToText4->isChecked();

    //Alignment
    mySettings.textAlignmentV = ui->comboBoxVerticalAling->currentIndex();
    mySettings.textAlignmentH = ui->comboBoxHorizontalAling->currentIndex();

    mySettings2.textAlignmentV = ui->comboBoxVerticalAling2->currentIndex();
    mySettings2.textAlignmentH = ui->comboBoxHorizontalAling2->currentIndex();

    mySettings3.textAlignmentV = ui->comboBoxVerticalAling3->currentIndex();
    mySettings3.textAlignmentH = ui->comboBoxHorizontalAling3->currentIndex();

    mySettings4.textAlignmentV = ui->comboBoxVerticalAling4->currentIndex();
    mySettings4.textAlignmentH = ui->comboBoxHorizontalAling4->currentIndex();

    //Caption Alignment
    mySettings.captionPosition = ui->comboBoxCaptionPosition->currentIndex();
    mySettings.captionAlignment = ui->comboBoxCaptionAlign->currentIndex();

    mySettings2.captionPosition = ui->comboBoxCaptionPosition2->currentIndex();
    mySettings2.captionAlignment = ui->comboBoxCaptionAlign2->currentIndex();

    mySettings3.captionPosition = ui->comboBoxCaptionPosition3->currentIndex();
    mySettings3.captionAlignment = ui->comboBoxCaptionAlign3->currentIndex();

    mySettings4.captionPosition = ui->comboBoxCaptionPosition4->currentIndex();
    mySettings4.captionAlignment = ui->comboBoxCaptionAlign4->currentIndex();

    // Version Abbreviations
    mySettings.useAbbriviation = ui->checkBoxAbbiviations->isChecked();
    mySettings2.useAbbriviation = ui->checkBoxAbbiviations2->isChecked();
    mySettings3.useAbbriviation = ui->checkBoxAbbiviations3->isChecked();
    mySettings4.useAbbriviation = ui->checkBoxAbbiviations4->isChecked();

    // Max screen use
    mySettings.screenUse = ui->spinBoxMaxScreen->value();
    mySettings.screenPosition = ui->comboBoxScreenPosition->currentIndex();

    mySettings2.screenUse = ui->spinBoxMaxScreen2->value();
    mySettings2.screenPosition = ui->comboBoxScreenPosition2->currentIndex();

    mySettings3.screenUse = ui->spinBoxMaxScreen3->value();
    mySettings3.screenPosition = ui->comboBoxScreenPosition3->currentIndex();

    mySettings4.screenUse = ui->spinBoxMaxScreen4->value();
    mySettings4.screenPosition = ui->comboBoxScreenPosition4->currentIndex();

    // Get if to use secodary screen settings
    mySettings2.useDisp1settings = !ui->groupBoxUseDisp2->isChecked();
    mySettings2.useDisp2settings = ui->groupBoxUseDisp2->isChecked();

    // Get if to use tertiary screen settings
    mySettings3.useDisp1settings = !ui->groupBoxUseDisp3->isChecked();
    mySettings3.useDisp3settings = ui->groupBoxUseDisp3->isChecked();

    // Get if to use quaternary screen settings
    mySettings4.useDisp1settings = !ui->groupBoxUseDisp4->isChecked();
    mySettings4.useDisp4settings = ui->groupBoxUseDisp4->isChecked();

    settings = mySettings;
    settings2 = mySettings2;
    settings3 = mySettings3;
    settings4 = mySettings4;
}

void BibleSettingWidget::setBibleVersions(BibleVersionSettings &bver, BibleVersionSettings &bver2, BibleVersionSettings &bver3, BibleVersionSettings &bver4)
{
    bversion = bver;
    bversion2 = bver2;
    bversion3 = bver3;
    bversion4 = bver4;
    loadBibleVersions();
}

void BibleSettingWidget::getBibleVersions(BibleVersionSettings &bver, BibleVersionSettings &bver2, BibleVersionSettings &bver3, BibleVersionSettings &bver4)
{
    int pbx = ui->comboBoxPrimaryBible->currentIndex();
    int sbx = ui->comboBoxSecondaryBible->currentIndex();
    int tbx = ui->comboBoxTrinaryBible->currentIndex();
    int obx = ui->comboBoxOperatorBible->currentIndex();

    // Get Bible version settings
    if(pbx != -1)
    {
        // Primary
        bversion.primaryBible = bible_id_list.at(pbx);
        // Secondary
        if(sbx <=0)
            bversion.secondaryBible = "none";
        else
            bversion.secondaryBible = secondary_id_list.at(sbx-1);
        // Trinary
        if(tbx <=0)
            bversion.trinaryBible = "none";
        else
            bversion.trinaryBible = trinary_id_list.at(tbx-1);
        // Operatror
        if(obx <=0)
            bversion.operatorBible = "same";
        else
            bversion.operatorBible = operator_id_list.at(obx-1);
    }
    else
    {
        // There are no bibles in the database
        bversion.primaryBible = "none";
        bversion.secondaryBible = "none";
        bversion.trinaryBible = "none";
        bversion.operatorBible = "same";
    }

    // Get Bible versions for secondary screen
    pbx = ui->comboBoxPrimaryBible2->currentIndex();
    sbx = ui->comboBoxSecondaryBible2->currentIndex();
    tbx = ui->comboBoxTrinaryBible2->currentIndex();
    if(pbx != -1)
    {
        // Primary
        bversion2.primaryBible = bible_id_list.at(pbx);
        // Secondary
        if(sbx <=0)
            bversion2.secondaryBible = "none";
        else
            bversion2.secondaryBible = secondary_id_list2.at(sbx-1);
        // Trinary
        if(tbx <=0)
            bversion2.trinaryBible = "none";
        else
            bversion2.trinaryBible = trinary_id_list2.at(tbx-1);
    }
    else
    {
        // There are no bibles in the database
        bversion2.primaryBible = "none";
        bversion2.secondaryBible = "none";
        bversion2.trinaryBible = "none";
        bversion2.operatorBible = "same";
    }

    // Get Bible versions for tertiary screen
    pbx = ui->comboBoxPrimaryBible3->currentIndex();
    sbx = ui->comboBoxSecondaryBible3->currentIndex();
    tbx = ui->comboBoxTrinaryBible3->currentIndex();
    if(pbx != -1)
    {
        // Primary
        bversion3.primaryBible = bible_id_list.at(pbx);
        // Secondary
        if(sbx <=0)
            bversion3.secondaryBible = "none";
        else
            bversion3.secondaryBible = secondary_id_list3.at(sbx-1);
        // Trinary
        if(tbx <=0)
            bversion3.trinaryBible = "none";
        else
            bversion3.trinaryBible = trinary_id_list3.at(tbx-1);
    }
    else
    {
        // There are no bibles in the database
        bversion3.primaryBible = "none";
        bversion3.secondaryBible = "none";
        bversion3.trinaryBible = "none";
        bversion3.operatorBible = "same";
    }

    // Get Bible versions for quaternary screen
    pbx = ui->comboBoxPrimaryBible4->currentIndex();
    sbx = ui->comboBoxSecondaryBible4->currentIndex();
    tbx = ui->comboBoxTrinaryBible4->currentIndex();
    if(pbx != -1)
    {
        // Primary
        bversion4.primaryBible = bible_id_list.at(pbx);
        // Secondary
        if(sbx <=0)
            bversion4.secondaryBible = "none";
        else
            bversion4.secondaryBible = secondary_id_list4.at(sbx-1);
        // Trinary
        if(tbx <=0)
            bversion4.trinaryBible = "none";
        else
            bversion4.trinaryBible = trinary_id_list4.at(tbx-1);
    }
    else
    {
        // There are no bibles in the database
        bversion4.primaryBible = "none";
        bversion4.secondaryBible = "none";
        bversion4.trinaryBible = "none";
        bversion4.operatorBible = "same";
    }

    bver = bversion;
    bver2 = bversion2;
    bver3 = bversion3;
    bver4 = bversion4;
}

void BibleSettingWidget::loadSettings()
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

    // Set background use
    ui->groupBoxBackground->setChecked(mySettings.useBackground);
    ui->lineEditBackPath->setText(mySettings.backgroundName);

    ui->groupBoxBackground2->setChecked(mySettings2.useBackground);
    ui->lineEditBackPath2->setText(mySettings2.backgroundName);

    ui->groupBoxBackground3->setChecked(mySettings3.useBackground);
    ui->lineEditBackPath3->setText(mySettings3.backgroundName);

    ui->groupBoxBackground4->setChecked(mySettings4.useBackground);
    ui->lineEditBackPath4->setText(mySettings4.backgroundName);

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

    // Set text font
    ui->labelFont->setText(getFontText(mySettings.textFont));
    ui->labelFont2->setText(getFontText(mySettings2.textFont));
    ui->labelFont3->setText(getFontText(mySettings3.textFont));
    ui->labelFont4->setText(getFontText(mySettings4.textFont));

    // Set Alignment
    ui->comboBoxVerticalAling->setCurrentIndex(mySettings.textAlignmentV);
    ui->comboBoxHorizontalAling->setCurrentIndex(mySettings.textAlignmentH);

    ui->comboBoxVerticalAling2->setCurrentIndex(mySettings2.textAlignmentV);
    ui->comboBoxHorizontalAling2->setCurrentIndex(mySettings2.textAlignmentH);

    ui->comboBoxVerticalAling3->setCurrentIndex(mySettings3.textAlignmentV);
    ui->comboBoxHorizontalAling3->setCurrentIndex(mySettings3.textAlignmentH);

    ui->comboBoxVerticalAling4->setCurrentIndex(mySettings4.textAlignmentV);
    ui->comboBoxHorizontalAling4->setCurrentIndex(mySettings4.textAlignmentH);

    // Set caption color
    p.setColor(QPalette::Base,mySettings.captionColor);
    ui->graphicViewCaptionColor->setPalette(p);

    p.setColor(QPalette::Base,mySettings2.captionColor);
    ui->graphicViewCaptionColor2->setPalette(p);

    p.setColor(QPalette::Base,mySettings3.captionColor);
    ui->graphicViewCaptionColor3->setPalette(p);

    p.setColor(QPalette::Base,mySettings4.captionColor);
    ui->graphicViewCaptionColor4->setPalette(p);

    // Set caption font
    ui->labelFontCaption->setText(getFontText(mySettings.captionFont));
    ui->labelFontCaption2->setText(getFontText(mySettings2.captionFont));
    ui->labelFontCaption3->setText(getFontText(mySettings3.captionFont));
    ui->labelFontCaption4->setText(getFontText(mySettings4.captionFont));

    // Set caption alignment
    ui->comboBoxCaptionPosition->setCurrentIndex(mySettings.captionPosition);
    ui->comboBoxCaptionPosition2->setCurrentIndex(mySettings2.captionPosition);
    ui->comboBoxCaptionPosition3->setCurrentIndex(mySettings3.captionPosition);
    ui->comboBoxCaptionPosition4->setCurrentIndex(mySettings4.captionPosition);

    ui->comboBoxCaptionAlign->setCurrentIndex(mySettings.captionAlignment);
    ui->comboBoxCaptionAlign2->setCurrentIndex(mySettings2.captionAlignment);
    ui->comboBoxCaptionAlign3->setCurrentIndex(mySettings3.captionAlignment);
    ui->comboBoxCaptionAlign4->setCurrentIndex(mySettings4.captionAlignment);

    // Set abbriviations use
    ui->checkBoxAbbiviations->setChecked(mySettings.useAbbriviation);
    ui->checkBoxAbbiviations2->setChecked(mySettings2.useAbbriviation);
    ui->checkBoxAbbiviations3->setChecked(mySettings3.useAbbriviation);
    ui->checkBoxAbbiviations4->setChecked(mySettings4.useAbbriviation);

    // Set max screen use
    ui->spinBoxMaxScreen->setValue(mySettings.screenUse);
    ui->comboBoxScreenPosition->setCurrentIndex(mySettings.screenPosition);

    ui->spinBoxMaxScreen2->setValue(mySettings2.screenUse);
    ui->comboBoxScreenPosition2->setCurrentIndex(mySettings2.screenPosition);

    ui->spinBoxMaxScreen3->setValue(mySettings3.screenUse);
    ui->comboBoxScreenPosition3->setCurrentIndex(mySettings3.screenPosition);

    ui->spinBoxMaxScreen4->setValue(mySettings4.screenUse);
    ui->comboBoxScreenPosition4->setCurrentIndex(mySettings4.screenPosition);

    // Background Color 4 Text
    // Display 1
    ui->groupBoxAddBackgroundToText->setChecked(mySettings.bibleAddBKColorToText);
    p.setColor(QPalette::Base,mySettings.bibleTextRecBKColor);
    ui->graphicViewAddBKColor->setPalette(p);

    p.setColor(QPalette::Base,mySettings.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor->setPalette(p);

    // Display 2
    ui->groupBoxAddBackgroundToText2->setChecked(mySettings2.bibleAddBKColorToText);
    p.setColor(QPalette::Base,mySettings2.bibleTextRecBKColor);
    ui->graphicViewAddBKColor2->setPalette(p);

    p.setColor(QPalette::Base,mySettings2.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor2->setPalette(p);

    // Display 3
    ui->groupBoxAddBackgroundToText3->setChecked(mySettings3.bibleAddBKColorToText);
    p.setColor(QPalette::Base,mySettings3.bibleTextRecBKColor);
    ui->graphicViewAddBKColor3->setPalette(p);

    p.setColor(QPalette::Base,mySettings3.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor3->setPalette(p);

    // Display 4
    ui->groupBoxAddBackgroundToText4->setChecked(mySettings4.bibleAddBKColorToText);
    p.setColor(QPalette::Base,mySettings4.bibleTextRecBKColor);
    ui->graphicViewAddBKColor4->setPalette(p);

    p.setColor(QPalette::Base,mySettings4.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor4->setPalette(p);

    // Set if to use secondary screen settings
    ui->groupBoxUseDisp2->setChecked(!mySettings2.useDisp1settings);
    on_groupBoxUseDisp2_toggled(!mySettings2.useDisp1settings);

    // Set if to use tertiary screen settings
    ui->groupBoxUseDisp3->setChecked(!mySettings3.useDisp1settings);
    on_groupBoxUseDisp3_toggled(!mySettings3.useDisp1settings);

    // Set if to use quaternary screen settings
    ui->groupBoxUseDisp4->setChecked(!mySettings4.useDisp1settings);
    on_groupBoxUseDisp4_toggled(!mySettings4.useDisp1settings);
}

void BibleSettingWidget::loadBibleVersions()
{
    // Clear items if exitst
    bibles.clear();
    bible_id_list.clear();

    // Get Bibles that exist in the database
    QSqlQuery sq;
    sq.exec("SELECT bible_name, id FROM BibleVersions");
    while(sq.next())
    {
        bibles << sq.value(0).toString();
        bible_id_list << sq.value(1).toString();
    }
    sq.clear();

    // Fill bibles comboboxes
    ui->comboBoxPrimaryBible->clear();
    ui->comboBoxPrimaryBible->addItems(bibles);
    ui->comboBoxSecondaryBible->clear();
    ui->comboBoxSecondaryBible->addItem(tr("None"));
    ui->comboBoxSecondaryBible->addItems(bibles);
    ui->comboBoxTrinaryBible->clear();
    ui->comboBoxTrinaryBible->addItem(tr("None"));
    ui->comboBoxTrinaryBible->addItems(bibles);
    ui->comboBoxOperatorBible->clear();
    ui->comboBoxOperatorBible->addItem(tr("Same as primary Bible"));
    ui->comboBoxOperatorBible->addItems(bibles);

    ui->comboBoxPrimaryBible2->clear();
    ui->comboBoxPrimaryBible2->addItems(bibles);
    ui->comboBoxSecondaryBible2->clear();
    ui->comboBoxSecondaryBible2->addItem(tr("None"));
    ui->comboBoxSecondaryBible2->addItems(bibles);
    ui->comboBoxTrinaryBible2->clear();
    ui->comboBoxTrinaryBible2->addItem(tr("None"));
    ui->comboBoxTrinaryBible2->addItems(bibles);

    ui->comboBoxPrimaryBible3->clear();
    ui->comboBoxPrimaryBible3->addItems(bibles);
    ui->comboBoxSecondaryBible3->clear();
    ui->comboBoxSecondaryBible3->addItem(tr("None"));
    ui->comboBoxSecondaryBible3->addItems(bibles);
    ui->comboBoxTrinaryBible3->clear();
    ui->comboBoxTrinaryBible3->addItem(tr("None"));
    ui->comboBoxTrinaryBible3->addItems(bibles);

    ui->comboBoxPrimaryBible4->clear();
    ui->comboBoxPrimaryBible4->addItems(bibles);
    ui->comboBoxSecondaryBible4->clear();
    ui->comboBoxSecondaryBible4->addItem(tr("None"));
    ui->comboBoxSecondaryBible4->addItems(bibles);
    ui->comboBoxTrinaryBible4->clear();
    ui->comboBoxTrinaryBible4->addItem(tr("None"));
    ui->comboBoxTrinaryBible4->addItems(bibles);

    // Set current primary bible
    if(bversion.primaryBible == "none")
        ui->comboBoxPrimaryBible->setCurrentIndex(0);
    else
        ui->comboBoxPrimaryBible->setCurrentIndex(bible_id_list.indexOf(bversion.primaryBible));

    if(bversion2.primaryBible == "none")
        ui->comboBoxPrimaryBible2->setCurrentIndex(0);
    else
        ui->comboBoxPrimaryBible2->setCurrentIndex(bible_id_list.indexOf(bversion2.primaryBible));

    if(bversion3.primaryBible == "none")
        ui->comboBoxPrimaryBible3->setCurrentIndex(0);
    else
        ui->comboBoxPrimaryBible3->setCurrentIndex(bible_id_list.indexOf(bversion3.primaryBible));

    if(bversion4.primaryBible == "none")
        ui->comboBoxPrimaryBible4->setCurrentIndex(0);
    else
        ui->comboBoxPrimaryBible4->setCurrentIndex(bible_id_list.indexOf(bversion4.primaryBible));

    // Set current secondary bible
    if(bversion.secondaryBible == "none")
        ui->comboBoxSecondaryBible->setCurrentIndex(0);
    else
        ui->comboBoxSecondaryBible->setCurrentIndex(bible_id_list.indexOf(bversion.secondaryBible)+1);

    if(bversion2.secondaryBible == "none")
        ui->comboBoxSecondaryBible2->setCurrentIndex(0);
    else
        ui->comboBoxSecondaryBible2->setCurrentIndex(bible_id_list.indexOf(bversion2.secondaryBible)+1);

    if(bversion3.secondaryBible == "none")
        ui->comboBoxSecondaryBible3->setCurrentIndex(0);
    else
        ui->comboBoxSecondaryBible3->setCurrentIndex(bible_id_list.indexOf(bversion3.secondaryBible)+1);

    if(bversion4.secondaryBible == "none")
        ui->comboBoxSecondaryBible4->setCurrentIndex(0);
    else
        ui->comboBoxSecondaryBible4->setCurrentIndex(bible_id_list.indexOf(bversion4.secondaryBible)+1);

    // Set current trinary bible
    if(bversion.trinaryBible == "none")
        ui->comboBoxTrinaryBible->setCurrentIndex(0);
    else
        ui->comboBoxTrinaryBible->setCurrentIndex(bible_id_list.indexOf(bversion.trinaryBible)+1);
    updateSecondaryBibleMenu();

    if(bversion2.trinaryBible == "none")
        ui->comboBoxTrinaryBible2->setCurrentIndex(0);
    else
        ui->comboBoxTrinaryBible2->setCurrentIndex(bible_id_list.indexOf(bversion2.trinaryBible)+1);
    updateSecondaryBibleMenu2();

    if(bversion3.trinaryBible == "none")
        ui->comboBoxTrinaryBible3->setCurrentIndex(0);
    else
        ui->comboBoxTrinaryBible3->setCurrentIndex(bible_id_list.indexOf(bversion3.trinaryBible)+1);
    updateSecondaryBibleMenu3();

    if(bversion4.trinaryBible == "none")
        ui->comboBoxTrinaryBible4->setCurrentIndex(0);
    else
        ui->comboBoxTrinaryBible4->setCurrentIndex(bible_id_list.indexOf(bversion4.trinaryBible)+1);
    updateSecondaryBibleMenu4();

    // Set current operator bible
    if(bversion.operatorBible == "same")
        ui->comboBoxOperatorBible->setCurrentIndex(0);
    else
        ui->comboBoxOperatorBible->setCurrentIndex(bible_id_list.indexOf(bversion.operatorBible)+1);
    updateOperatorBibleMenu();
}

void BibleSettingWidget::updateSecondaryBibleMenu()
{
    QString pbible = ui->comboBoxPrimaryBible->currentText();
    QString sbible = ui->comboBoxSecondaryBible->currentText();
    secondary_bibles = bibles;
    secondary_bibles.removeOne(pbible);

    secondary_id_list = bible_id_list;
    secondary_id_list.removeAt(ui->comboBoxPrimaryBible->currentIndex());
    ui->comboBoxSecondaryBible->clear();
    ui->comboBoxSecondaryBible->addItem(tr("None"));
    ui->comboBoxSecondaryBible->addItems(secondary_bibles);

    int i = ui->comboBoxSecondaryBible->findText(sbible);
    if( i != -1 )
        // The same secondary bible is still available
        ui->comboBoxSecondaryBible->setCurrentIndex(i);

    updateTrinaryBibleMenu();
}

void BibleSettingWidget::updateSecondaryBibleMenu2()
{
    QString pbible = ui->comboBoxPrimaryBible2->currentText();
    QString sbible = ui->comboBoxSecondaryBible2->currentText();
    secondary_bibles2 = bibles;
    secondary_bibles2.removeOne(pbible);

    secondary_id_list2 = bible_id_list;
    secondary_id_list2.removeAt(ui->comboBoxPrimaryBible2->currentIndex());
    ui->comboBoxSecondaryBible2->clear();
    ui->comboBoxSecondaryBible2->addItem(tr("None"));
    ui->comboBoxSecondaryBible2->addItems(secondary_bibles2);

    int i = ui->comboBoxSecondaryBible2->findText(sbible);
    if( i != -1 ) // The same secondary bible is still available
        ui->comboBoxSecondaryBible2->setCurrentIndex(i);

    updateTrinaryBibleMenu2();
}

// Bible for Display 3
void BibleSettingWidget::updateSecondaryBibleMenu3()
{
    QString pbible = ui->comboBoxPrimaryBible3->currentText();
    QString sbible = ui->comboBoxSecondaryBible3->currentText();
    secondary_bibles3 = bibles;
    secondary_bibles3.removeOne(pbible);

    secondary_id_list3 = bible_id_list;
    secondary_id_list3.removeAt(ui->comboBoxPrimaryBible3->currentIndex());
    ui->comboBoxSecondaryBible3->clear();
    ui->comboBoxSecondaryBible3->addItem(tr("None"));
    ui->comboBoxSecondaryBible3->addItems(secondary_bibles3);

    int i = ui->comboBoxSecondaryBible3->findText(sbible);
    if( i != -1 ) // The same Tertiary bible is still available
        ui->comboBoxSecondaryBible3->setCurrentIndex(i);

    updateTrinaryBibleMenu3();
}

// Bible for Display 4
void BibleSettingWidget::updateSecondaryBibleMenu4()
{
    QString pbible = ui->comboBoxPrimaryBible4->currentText();
    QString sbible = ui->comboBoxSecondaryBible4->currentText();
    secondary_bibles4 = bibles;
    secondary_bibles4.removeOne(pbible);

    secondary_id_list4 = bible_id_list;
    secondary_id_list4.removeAt(ui->comboBoxPrimaryBible4->currentIndex());
    ui->comboBoxSecondaryBible4->clear();
    ui->comboBoxSecondaryBible4->addItem(tr("None"));
    ui->comboBoxSecondaryBible4->addItems(secondary_bibles4);

    int i = ui->comboBoxSecondaryBible4->findText(sbible);
    if( i != -1 ) // The same Tertiary bible is still available
        ui->comboBoxSecondaryBible4->setCurrentIndex(i);

    updateTrinaryBibleMenu4();
}

void BibleSettingWidget::updateTrinaryBibleMenu()
{
    if (ui->comboBoxSecondaryBible->currentIndex() == 0)
    {
        ui->comboBoxTrinaryBible->setCurrentIndex(0);
        ui->comboBoxTrinaryBible->setEnabled(false);
    }
    else
    {
        ui->comboBoxTrinaryBible->setEnabled(true);
        QString sbible = ui->comboBoxSecondaryBible->currentText();
        QString tbible = ui->comboBoxTrinaryBible->currentText();
        QStringList trinary_bibles = secondary_bibles;
        trinary_bibles.removeOne(sbible);

        trinary_id_list = secondary_id_list;
        trinary_id_list.removeAt(ui->comboBoxSecondaryBible->currentIndex()-1);
        ui->comboBoxTrinaryBible->clear();
        ui->comboBoxTrinaryBible->addItem(tr("None"));
        ui->comboBoxTrinaryBible->addItems(trinary_bibles);

        int i = ui->comboBoxTrinaryBible->findText(tbible);
        if( i != -1 )
            // The same secondary bible is still available
            ui->comboBoxTrinaryBible->setCurrentIndex(i);
    }
}

void BibleSettingWidget::updateTrinaryBibleMenu2()
{
    if (ui->comboBoxSecondaryBible2->currentIndex() == 0)
    {
        ui->comboBoxTrinaryBible2->setCurrentIndex(0);
        ui->comboBoxTrinaryBible2->setEnabled(false);
    }
    else
    {
        ui->comboBoxTrinaryBible2->setEnabled(true);
        QString sbible = ui->comboBoxSecondaryBible2->currentText();
        QString tbible = ui->comboBoxTrinaryBible2->currentText();
        QStringList trinary_bibles = secondary_bibles2;
        trinary_bibles.removeOne(sbible);

        trinary_id_list2 = secondary_id_list2;
        trinary_id_list2.removeAt(ui->comboBoxSecondaryBible2->currentIndex()-1);
        ui->comboBoxTrinaryBible2->clear();
        ui->comboBoxTrinaryBible2->addItem(tr("None"));
        ui->comboBoxTrinaryBible2->addItems(trinary_bibles);

        int i = ui->comboBoxTrinaryBible2->findText(tbible);
        if( i != -1 )
            // The same secondary bible is still available
            ui->comboBoxTrinaryBible2->setCurrentIndex(i);
    }
}

void BibleSettingWidget::updateTrinaryBibleMenu3()
{
    if (ui->comboBoxSecondaryBible3->currentIndex() == 0)
    {
        ui->comboBoxTrinaryBible3->setCurrentIndex(0);
        ui->comboBoxTrinaryBible3->setEnabled(false);
    }
    else
    {
        ui->comboBoxTrinaryBible3->setEnabled(true);
        QString sbible = ui->comboBoxSecondaryBible3->currentText();
        QString tbible = ui->comboBoxTrinaryBible3->currentText();
        QStringList trinary_bibles = secondary_bibles3;
        trinary_bibles.removeOne(sbible);

        trinary_id_list3 = secondary_id_list3;
        trinary_id_list3.removeAt(ui->comboBoxSecondaryBible3->currentIndex()-1);
        ui->comboBoxTrinaryBible3->clear();
        ui->comboBoxTrinaryBible3->addItem(tr("None"));
        ui->comboBoxTrinaryBible3->addItems(trinary_bibles);

        int i = ui->comboBoxTrinaryBible3->findText(tbible);
        if( i != -1 )
            // The same secondary bible is still available
            ui->comboBoxTrinaryBible3->setCurrentIndex(i);
    }
}

void BibleSettingWidget::updateTrinaryBibleMenu4()
{
    if (ui->comboBoxSecondaryBible4->currentIndex() == 0)
    {
        ui->comboBoxTrinaryBible4->setCurrentIndex(0);
        ui->comboBoxTrinaryBible4->setEnabled(false);
    }
    else
    {
        ui->comboBoxTrinaryBible4->setEnabled(true);
        QString sbible = ui->comboBoxSecondaryBible4->currentText();
        QString tbible = ui->comboBoxTrinaryBible4->currentText();
        QStringList trinary_bibles = secondary_bibles4;
        trinary_bibles.removeOne(sbible);

        trinary_id_list4 = secondary_id_list4;
        trinary_id_list4.removeAt(ui->comboBoxSecondaryBible4->currentIndex()-1);
        ui->comboBoxTrinaryBible4->clear();
        ui->comboBoxTrinaryBible4->addItem(tr("None"));
        ui->comboBoxTrinaryBible4->addItems(trinary_bibles);

        int i = ui->comboBoxTrinaryBible4->findText(tbible);
        if( i != -1 )
            // The same secondary bible is still available
            ui->comboBoxTrinaryBible4->setCurrentIndex(i);
    }
}

void BibleSettingWidget::updateOperatorBibleMenu()
{
    QString pbible = ui->comboBoxPrimaryBible->currentText();
    QString obible = ui->comboBoxOperatorBible->currentText();
    QStringList operator_bibles = bibles;
    operator_bibles.removeOne(pbible);

    operator_id_list = bible_id_list;
    operator_id_list.removeAt(ui->comboBoxPrimaryBible->currentIndex());
    ui->comboBoxOperatorBible->clear();
    ui->comboBoxOperatorBible->addItem(tr("Same as primary Bible"));
    ui->comboBoxOperatorBible->addItems(operator_bibles);

    int i = ui->comboBoxOperatorBible->findText(obible);
    if( i != -1 )
        // The same operaotr bible is still available
        ui->comboBoxOperatorBible->setCurrentIndex(i);
}

void BibleSettingWidget::setDispScreen2Visible(bool visible)
{
    ui->groupBoxUseDisp2->setVisible(visible);
}

void BibleSettingWidget::setDispScreen3Visible(bool visible)
{
    ui->groupBoxUseDisp3->setVisible(visible);
}

void BibleSettingWidget::setDispScreen4Visible(bool visible)
{
    ui->groupBoxUseDisp4->setVisible(visible);
}

void BibleSettingWidget::on_comboBoxPrimaryBible_activated(const QString &arg1)
{
    updateSecondaryBibleMenu();
    updateOperatorBibleMenu();
}

void BibleSettingWidget::on_comboBoxPrimaryBible2_activated(const QString &arg1)
{
    updateSecondaryBibleMenu2();
    updateTrinaryBibleMenu2();
}

void BibleSettingWidget::on_comboBoxPrimaryBible3_activated(const QString &arg1)
{
    updateSecondaryBibleMenu3();
    updateTrinaryBibleMenu3();
}

void BibleSettingWidget::on_comboBoxPrimaryBible4_activated(const QString &arg1)
{
    updateSecondaryBibleMenu4();
    updateTrinaryBibleMenu4();
}

void BibleSettingWidget::on_comboBoxSecondaryBible_activated(const QString &arg1)
{
    updateTrinaryBibleMenu();
}

void BibleSettingWidget::on_comboBoxSecondaryBible2_activated(const QString &arg1)
{
    updateTrinaryBibleMenu2();
}

void BibleSettingWidget::on_comboBoxSecondaryBible3_activated(const QString &arg1)
{
    updateTrinaryBibleMenu3();
}

void BibleSettingWidget::on_comboBoxSecondaryBible4_activated(const QString &arg1)
{
    updateTrinaryBibleMenu4();
}

void BibleSettingWidget::on_checkBoxUseShadow_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow->setChecked(false);
        ui->checkBoxUseBlurredShadow->setEnabled(false);
    }
}

void BibleSettingWidget::on_checkBoxUseShadow2_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow2->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow2->setChecked(false);
        ui->checkBoxUseBlurredShadow2->setEnabled(false);
    }
}

// Display screen 3
void BibleSettingWidget::on_checkBoxUseShadow3_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow3->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow3->setChecked(false);
        ui->checkBoxUseBlurredShadow3->setEnabled(false);
    }
}

// Display screen 4
void BibleSettingWidget::on_checkBoxUseShadow4_stateChanged(int arg1)
{
    if(arg1==2)
        ui->checkBoxUseBlurredShadow4->setEnabled(true);
    else
    {
        ui->checkBoxUseBlurredShadow4->setChecked(false);
        ui->checkBoxUseBlurredShadow4->setEnabled(false);
    }
}

void BibleSettingWidget::on_buttonBrowseBackground_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for Bible wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings.backgroundName = filename;
        ui->lineEditBackPath->setText(filename);
    }
}

void BibleSettingWidget::on_buttonBrowseBackground2_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for Bible wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings2.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings2.backgroundName = filename;
        ui->lineEditBackPath2->setText(filename);
    }
}

void BibleSettingWidget::on_buttonBrowseBackground3_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for Bible wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings3.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings3.backgroundName = filename;
        ui->lineEditBackPath3->setText(filename);
    }
}

void BibleSettingWidget::on_buttonBrowseBackground4_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a image for Bible wallpaper"),
                                                    ".", tr("Images(%1)").arg(getSupportedImageFormats()));
    if(!filename.isNull())
    {
        QPixmap p(filename);
        mySettings4.backgroundPix = p;
        QFileInfo fi(filename);
        filename = fi.fileName();
        mySettings4.backgroundName = filename;
        ui->lineEditBackPath4->setText(filename);
    }
}

void BibleSettingWidget::on_toolButtonTextColor_clicked()
{
    QColor c(QColorDialog::getColor(mySettings.textColor,this));
    if(c.isValid())
        mySettings.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings.textColor);
    ui->graphicViewTextColor->setPalette(p);
}

void BibleSettingWidget::on_toolButtonTextColor2_clicked()
{
    QColor c(QColorDialog::getColor(mySettings2.textColor,this));
    if(c.isValid())
        mySettings2.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings2.textColor);
    ui->graphicViewTextColor2->setPalette(p);
}

void BibleSettingWidget::on_toolButtonTextColor3_clicked()
{
    QColor c(QColorDialog::getColor(mySettings3.textColor,this));
    if(c.isValid())
        mySettings3.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings3.textColor);
    ui->graphicViewTextColor3->setPalette(p);
}

void BibleSettingWidget::on_toolButtonTextColor4_clicked()
{
    QColor c(QColorDialog::getColor(mySettings4.textColor,this));
    if(c.isValid())
        mySettings4.textColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings4.textColor);
    ui->graphicViewTextColor4->setPalette(p);
}

void BibleSettingWidget::on_toolButtonTextFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings.textFont,this);
    if(ok)
        mySettings.textFont = font;

    ui->labelFont->setText(getFontText(mySettings.textFont));
}

void BibleSettingWidget::on_toolButtonTextFont2_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings2.textFont,this);
    if(ok)
        mySettings2.textFont = font;

    ui->labelFont2->setText(getFontText(mySettings2.textFont));
}

void BibleSettingWidget::on_toolButtonTextFont3_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings3.textFont,this);
    if(ok)
        mySettings3.textFont = font;

    ui->labelFont3->setText(getFontText(mySettings3.textFont));
}

void BibleSettingWidget::on_toolButtonTextFont4_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings4.textFont,this);
    if(ok)
        mySettings4.textFont = font;

    ui->labelFont4->setText(getFontText(mySettings4.textFont));
}

void BibleSettingWidget::on_toolButtonCaptionColor_clicked()
{
    QColor c(QColorDialog::getColor(mySettings.captionColor,this));
    if(c.isValid())
        mySettings.captionColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings.captionColor);
    ui->graphicViewCaptionColor->setPalette(p);
}

void BibleSettingWidget::on_toolButtonCaptionColor2_clicked()
{
    QColor c(QColorDialog::getColor(mySettings2.captionColor,this));
    if(c.isValid())
        mySettings2.captionColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings2.captionColor);
    ui->graphicViewCaptionColor2->setPalette(p);
}

void BibleSettingWidget::on_toolButtonCaptionColor3_clicked()
{
    QColor c(QColorDialog::getColor(mySettings3.captionColor,this));
    if(c.isValid())
        mySettings3.captionColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings3.captionColor);
    ui->graphicViewCaptionColor3->setPalette(p);
}

void BibleSettingWidget::on_toolButtonCaptionColor4_clicked()
{
    QColor c(QColorDialog::getColor(mySettings4.captionColor,this));
    if(c.isValid())
        mySettings4.captionColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings4.captionColor);
    ui->graphicViewCaptionColor4->setPalette(p);
}

void BibleSettingWidget::on_toolButtonCaptionFont_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings.captionFont,this);
    if(ok)
        mySettings.captionFont = font;

    ui->labelFontCaption->setText(getFontText(mySettings.captionFont));
}

void BibleSettingWidget::on_toolButtonCaptionFont2_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings2.captionFont,this);
    if(ok)
        mySettings2.captionFont = font;

    ui->labelFontCaption2->setText(getFontText(mySettings2.captionFont));
}

void BibleSettingWidget::on_toolButtonCaptionFont3_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings3.captionFont,this);
    if(ok)
        mySettings3.captionFont = font;

    ui->labelFontCaption3->setText(getFontText(mySettings3.captionFont));
}

void BibleSettingWidget::on_toolButtonCaptionFont4_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,mySettings4.captionFont,this);
    if(ok)
        mySettings4.captionFont = font;

    ui->labelFontCaption4->setText(getFontText(mySettings4.captionFont));
}

void BibleSettingWidget::on_groupBoxAddBackgroundToText_toggled(bool arg1)
{
    QColor c(mySettings.bibleTextGenBKColor);
    if(arg1)
    {
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings.useBackground = arg1;
        mySettings.backgroundName = c.name();
        mySettings.backgroundPix = px;
        ui->comboBoxVerticalAling->setCurrentIndex(2); // Bottom
        ui->lineEditBackPath->setText(c.name());
        ui->groupBoxBackground->setChecked(arg1);
    }
    else
    {
        mySettings.useBackground = arg1;
        mySettings.backgroundName = "";
        mySettings.backgroundPix = QPixmap(1,1);
        ui->groupBoxBackground->setChecked(arg1);
        ui->lineEditBackPath->clear();
    }
}

void BibleSettingWidget::on_groupBoxAddBackgroundToText2_toggled(bool arg1)
{
    QColor c(mySettings2.bibleTextGenBKColor);
    if(arg1)
    {
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings2.useBackground = arg1;
        mySettings2.backgroundName = c.name();
        mySettings2.backgroundPix = px;
        ui->comboBoxVerticalAling2->setCurrentIndex(2); // Bottom
        ui->lineEditBackPath2->setText(c.name());
        ui->groupBoxBackground2->setChecked(arg1);
    }
    else
    {
        mySettings2.useBackground = arg1;
        mySettings2.backgroundName = "";
        mySettings2.backgroundPix = QPixmap(1,1);
        ui->groupBoxBackground2->setChecked(arg1);
        ui->lineEditBackPath2->clear();
    }
}

void BibleSettingWidget::on_groupBoxAddBackgroundToText3_toggled(bool arg1)
{
    QColor c(mySettings3.bibleTextGenBKColor);
    if(arg1)
    {
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings3.useBackground = arg1;
        mySettings3.backgroundName = c.name();
        mySettings3.backgroundPix = px;
        ui->comboBoxVerticalAling3->setCurrentIndex(2); // Bottom
        ui->lineEditBackPath3->setText(c.name());
        ui->groupBoxBackground3->setChecked(arg1);
    }
    else
    {
        mySettings3.useBackground = arg1;
        mySettings3.backgroundName = "";
        mySettings3.backgroundPix = QPixmap(1,1);
        ui->groupBoxBackground3->setChecked(arg1);
        ui->lineEditBackPath3->clear();
    }
}

void BibleSettingWidget::on_groupBoxAddBackgroundToText4_toggled(bool arg1)
{
    QColor c(mySettings4.bibleTextGenBKColor);
    if(arg1)
    {
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings4.useBackground = arg1;
        mySettings4.backgroundName = c.name();
        mySettings4.backgroundPix = px;
        ui->comboBoxVerticalAling4->setCurrentIndex(2); // Bottom
        ui->lineEditBackPath4->setText(c.name());
        ui->groupBoxBackground4->setChecked(arg1);
    }
    else
    {
        mySettings4.useBackground = arg1;
        mySettings4.backgroundName = "";
        mySettings4.backgroundPix = QPixmap(1,1);
        ui->groupBoxBackground4->setChecked(arg1);
        ui->lineEditBackPath4->clear();
    }
}

void BibleSettingWidget::on_toolButtonAddBKColorText_clicked()
{
    QColor c(QColorDialog::getColor(mySettings.bibleTextRecBKColor,this));
    if(c.isValid())
        mySettings.bibleTextRecBKColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings.bibleTextRecBKColor);
    ui->graphicViewAddBKColor->setPalette(p);
}

void BibleSettingWidget::on_toolButtonAddBKColorText2_clicked()
{
    QColor c(QColorDialog::getColor(mySettings2.bibleTextRecBKColor,this));
    if(c.isValid())
        mySettings2.bibleTextRecBKColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings2.bibleTextRecBKColor);
    ui->graphicViewAddBKColor2->setPalette(p);
}

void BibleSettingWidget::on_toolButtonAddBKColorText3_clicked()
{
    QColor c(QColorDialog::getColor(mySettings3.bibleTextRecBKColor,this));
    if(c.isValid())
        mySettings3.bibleTextRecBKColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings3.bibleTextRecBKColor);
    ui->graphicViewAddBKColor3->setPalette(p);
}

void BibleSettingWidget::on_toolButtonAddBKColorText4_clicked()
{
    QColor c(QColorDialog::getColor(mySettings4.bibleTextRecBKColor,this));
    if(c.isValid())
        mySettings4.bibleTextRecBKColor = c;
    QPalette p;
    p.setColor(QPalette::Base,mySettings4.bibleTextRecBKColor);
    ui->graphicViewAddBKColor4->setPalette(p);
}

void BibleSettingWidget::on_toolButtonChangeGenBKColor_clicked()
{
    QColor c(QColorDialog::getColor(mySettings.bibleTextGenBKColor,this));
    if(c.isValid())
    {
        mySettings.bibleTextGenBKColor = c;
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings.backgroundPix = px;
        mySettings.backgroundName = c.name();
        ui->lineEditBackPath->setText(c.name());
    }
    QPalette p;
    p.setColor(QPalette::Base,mySettings.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor->setPalette(p);
}

void BibleSettingWidget::on_toolButtonChangeGenBKColor2_clicked()
{
    QColor c(QColorDialog::getColor(mySettings2.bibleTextGenBKColor,this));
    if(c.isValid())
    {
        mySettings2.bibleTextGenBKColor = c;
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings2.backgroundPix = px;
        mySettings2.backgroundName = c.name();
        ui->lineEditBackPath2->setText(c.name());
    }
    QPalette p;
    p.setColor(QPalette::Base,mySettings2.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor2->setPalette(p);    
}

void BibleSettingWidget::on_toolButtonChangeGenBKColor3_clicked()
{
    QColor c(QColorDialog::getColor(mySettings3.bibleTextGenBKColor,this));
    if(c.isValid())
    {
        mySettings3.bibleTextGenBKColor = c;
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings3.backgroundPix = px;
        mySettings3.backgroundName = c.name();
        ui->lineEditBackPath3->setText(c.name());
    }
    QPalette p;
    p.setColor(QPalette::Base,mySettings3.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor3->setPalette(p);
}

void BibleSettingWidget::on_toolButtonChangeGenBKColor4_clicked()
{
    QColor c(QColorDialog::getColor(mySettings4.bibleTextGenBKColor,this));
    if(c.isValid())
    {
        mySettings4.bibleTextGenBKColor = c;
        QPixmap px(1920, 1080);
        px.fill(c);
        mySettings4.backgroundPix = px;
        mySettings4.backgroundName = c.name();
        ui->lineEditBackPath4->setText(c.name());
    }
    QPalette p;
    p.setColor(QPalette::Base,mySettings4.bibleTextGenBKColor);
    ui->graphicViewChangeGenBKColor4->setPalette(p);
}

void BibleSettingWidget::on_groupBoxUseDisp2_toggled(bool arg1)
{
    ui->widgetBibles2->setVisible(arg1);
    ui->groupBoxEffects2->setVisible(arg1);
    ui->groupBoxBackground2->setVisible(arg1);
    ui->groupBoxMaxScreen2->setVisible(arg1);
    ui->groupBoxCaptionProperties2->setVisible(arg1);
    ui->groupBoxTextProperties2->setVisible(arg1);
    ui->groupBoxAddBackgroundToText2->setVisible(arg1);
    ui->checkBoxAbbiviations2->setVisible(arg1);
}

void BibleSettingWidget::on_groupBoxUseDisp3_toggled(bool arg1)
{
    ui->groupWidgetBibles3->setVisible(arg1);
    ui->groupBoxEffects3->setVisible(arg1);
    ui->groupBoxBackground3->setVisible(arg1);
    ui->groupBoxMaxScreen3->setVisible(arg1);
    ui->groupBoxCaptionProperties3->setVisible(arg1);
    ui->groupBoxTextProperties3->setVisible(arg1);
    ui->groupBoxAddBackgroundToText3->setVisible(arg1);
    ui->checkBoxAbbiviations3->setVisible(arg1);
}

void BibleSettingWidget::on_groupBoxUseDisp4_toggled(bool arg1)
{
    ui->groupWidgetBibles4->setVisible(arg1);
    ui->groupBoxEffects4->setVisible(arg1);
    ui->groupBoxBackground4->setVisible(arg1);
    ui->groupBoxMaxScreen4->setVisible(arg1);
    ui->groupBoxCaptionProperties4->setVisible(arg1);
    ui->groupBoxTextProperties4->setVisible(arg1);
    ui->groupBoxAddBackgroundToText4->setVisible(arg1);
    ui->checkBoxAbbiviations4->setVisible(arg1);
}

void BibleSettingWidget::on_pushButtonDefault_clicked()
{
    BibleSettings b;
    mySettings = b;
    mySettings2 = b;
    mySettings3 = b;
    mySettings4 = b;
    loadSettings();
}

QString BibleSettingWidget::getFontText(QFont font)
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

void BibleSettingWidget::on_pushButtonApplyToAll_clicked()
{
    emit applyBackToAll(1,mySettings.backgroundName,mySettings.backgroundPix);
}

void BibleSettingWidget::setBackgroungds(QString name, QPixmap back)
{
    mySettings.backgroundName = name;
    mySettings.backgroundPix = back;
    mySettings2.backgroundName = name;
    mySettings2.backgroundPix = back;
    mySettings3.backgroundName = name;
    mySettings3.backgroundPix = back;
    mySettings4.backgroundName = name;
    mySettings4.backgroundPix = back;
    ui->lineEditBackPath->setText(name);
    ui->lineEditBackPath2->setText(name);
    ui->lineEditBackPath3->setText(name);
    ui->lineEditBackPath4->setText(name);
}

QString BibleSettingWidget::getSupportedVideoFormats()
{
    return "Video files (*.mp4 *.avi *.mov *.webm *.mkv *.flv *.wmv);;All files (*)";
}

void BibleSettingWidget::on_radioButtonBgNone_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_NONE;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgSolid_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgImage_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground->setVisible(true);
        ui->groupBoxVideoBackground->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgVideo_toggled(bool checked)
{
    if(checked)
    {
        mySettings.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground->setVisible(false);
        ui->groupBoxVideoBackground->setVisible(true);
    }
}

void BibleSettingWidget::on_radioButtonBgNone2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_NONE;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgSolid2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgImage2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground2->setVisible(true);
        ui->groupBoxVideoBackground2->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgVideo2_toggled(bool checked)
{
    if(checked)
    {
        mySettings2.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground2->setVisible(false);
        ui->groupBoxVideoBackground2->setVisible(true);
    }
}

void BibleSettingWidget::on_radioButtonBgNone3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_NONE;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgSolid3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgImage3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground3->setVisible(true);
        ui->groupBoxVideoBackground3->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgVideo3_toggled(bool checked)
{
    if(checked)
    {
        mySettings3.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground3->setVisible(false);
        ui->groupBoxVideoBackground3->setVisible(true);
    }
}

void BibleSettingWidget::on_radioButtonBgNone4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_NONE;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgSolid4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_SOLID_COLOR;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgImage4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_PICTURE;
        ui->groupBoxImageBackground4->setVisible(true);
        ui->groupBoxVideoBackground4->setVisible(false);
    }
}

void BibleSettingWidget::on_radioButtonBgVideo4_toggled(bool checked)
{
    if(checked)
    {
        mySettings4.backgroundType = B_VIDEO;
        ui->groupBoxImageBackground4->setVisible(false);
        ui->groupBoxVideoBackground4->setVisible(true);
    }
}

void BibleSettingWidget::on_toolButtonBrowseVideo_clicked()
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

void BibleSettingWidget::on_toolButtonBrowseVideo2_clicked()
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

void BibleSettingWidget::on_toolButtonBrowseVideo3_clicked()
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

void BibleSettingWidget::on_toolButtonBrowseVideo4_clicked()
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

void BibleSettingWidget::updateVideoPreview(int display)
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

void BibleSettingWidget::setBackgroundTypeRadio(int display, int type)
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
