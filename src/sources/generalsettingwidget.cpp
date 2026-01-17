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

#include "../headers/generalsettingwidget.hpp"
#include "ui_generalsettingwidget.h"

FormatPreviewLabel::FormatPreviewLabel(QWidget *parent)
    : QLabel(parent)
    , m_screenWidth(1920)
    , m_screenHeight(1080)
    , m_maintainAspect(true)
    , m_cropToFit(false)
    , m_screenIndex(1)
{
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color: #1a1a1a;");
}

void FormatPreviewLabel::setFormat(int width, int height, bool maintainAspect, bool cropToFit)
{
    m_screenWidth = width;
    m_screenHeight = height;
    m_maintainAspect = maintainAspect;
    m_cropToFit = cropToFit;
    update();
}

void FormatPreviewLabel::updatePreview(int screenIndex)
{
    m_screenIndex = screenIndex;
    update();
}

QString FormatPreviewLabel::getAspectRatioString(int width, int height)
{
    int gcd = [](int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }(width, height);

    int ratioWidth = width / gcd;
    int ratioHeight = height / gcd;

    if (ratioWidth == 16 && ratioHeight == 9) return "16:9";
    if (ratioWidth == 4 && ratioHeight == 3) return "4:3";
    if (ratioWidth == 16 && ratioHeight == 10) return "16:10";
    if (ratioWidth == 21 && ratioHeight == 9) return "21:9";
    if (ratioWidth == 9 && ratioHeight == 16) return "9:16";

    return QString("%1:%2").arg(ratioWidth).arg(ratioHeight);
}

void FormatPreviewLabel::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect drawRect = contentsRect().adjusted(10, 10, -10, -10);

    double screenAspect = static_cast<double>(m_screenWidth) / m_screenHeight;
    double widgetAspect = static_cast<double>(drawRect.width()) / drawRect.height();

    QRect screenRect;
    if (widgetAspect > screenAspect) {
        int scaledWidth = static_cast<int>(drawRect.height() * screenAspect);
        screenRect = QRect(drawRect.x() + (drawRect.width() - scaledWidth) / 2,
                          drawRect.y(),
                          scaledWidth,
                          drawRect.height());
    } else {
        int scaledHeight = static_cast<int>(drawRect.width() / screenAspect);
        screenRect = QRect(drawRect.x(),
                          drawRect.y() + (drawRect.height() - scaledHeight) / 2,
                          drawRect.width(),
                          scaledHeight);
    }

    painter.fillRect(drawRect, QColor(60, 60, 60));

    if (m_maintainAspect) {
        painter.fillRect(screenRect, Qt::black);

        QColor contentColor(100, 100, 100);
        painter.fillRect(screenRect, contentColor);

        int effectiveWidth = m_screenWidth;
        int effectiveHeight = m_screenHeight;
        QString modeText;

        if (m_cropToFit) {
            effectiveWidth = m_screenWidth;
            effectiveHeight = m_screenHeight;
            modeText = tr("Cropped");
            painter.fillRect(screenRect, QColor(80, 140, 80));
        } else {
            modeText = tr("Letterboxed");
            painter.fillRect(screenRect, QColor(140, 80, 80));
        }

        painter.setPen(Qt::white);
        painter.drawText(screenRect, Qt::AlignCenter,
                         QString("%1x%2\n%3\n%4")
                         .arg(m_screenWidth)
                         .arg(m_screenHeight)
                         .arg(getAspectRatioString(m_screenWidth, m_screenHeight))
                         .arg(modeText));

        painter.setPen(QColor(200, 200, 200));
        painter.drawRect(screenRect.adjusted(0, 0, -1, -1));
    } else {
        painter.fillRect(screenRect, Qt::black);

        painter.setPen(Qt::white);
        painter.drawText(screenRect, Qt::AlignCenter,
                         QString("%1x%2\n%3\n%4")
                         .arg(m_screenWidth)
                         .arg(m_screenHeight)
                         .arg(getAspectRatioString(m_screenWidth, m_screenHeight))
                         .arg(tr("Stretched")));

        painter.setPen(QColor(200, 200, 200));
        painter.drawRect(screenRect.adjusted(0, 0, -1, -1));
    }
}

GeneralSettingWidget::GeneralSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GeneralSettingWidget)
{
    ui->setupUi(this);
    virtualOutputWidget = new VirtualOutputSettingWidget;
    ui->verticalLayoutVirtualOutput->addWidget(virtualOutputWidget);
    populateFormatComboBoxes();
    createPreviewWidgets();
    connect(ui->comboBoxFormat1, QOverload<int>::of(&QComboBox::activated), this, [this](int index){ on_comboBoxFormat1_activated(index); });
    connect(ui->comboBoxFormat2, QOverload<int>::of(&QComboBox::activated), this, [this](int index){ on_comboBoxFormat2_activated(index); });
    connect(ui->comboBoxFormat3, QOverload<int>::of(&QComboBox::activated), this, [this](int index){ on_comboBoxFormat3_activated(index); });
    connect(ui->comboBoxFormat4, QOverload<int>::of(&QComboBox::activated), this, [this](int index){ on_comboBoxFormat4_activated(index); });
}

void GeneralSettingWidget::createPreviewWidgets()
{
    previewLabel1 = new FormatPreviewLabel(ui->tabScreenFormat1);
    previewLabel1->setObjectName("previewLabel1");
    ui->horizontalLayoutPreview1->replaceWidget(ui->labelPreview1, previewLabel1);
    ui->labelPreview1->deleteLater();

    previewLabel2 = new FormatPreviewLabel(ui->tabScreenFormat2);
    previewLabel2->setObjectName("previewLabel2");
    ui->horizontalLayoutPreview2->replaceWidget(ui->labelPreview2, previewLabel2);
    ui->labelPreview2->deleteLater();

    previewLabel3 = new FormatPreviewLabel(ui->tabScreenFormat3);
    previewLabel3->setObjectName("previewLabel3");
    ui->horizontalLayoutPreview3->replaceWidget(ui->labelPreview3, previewLabel3);
    ui->labelPreview3->deleteLater();

    previewLabel4 = new FormatPreviewLabel(ui->tabScreenFormat4);
    previewLabel4->setObjectName("previewLabel4");
    ui->horizontalLayoutPreview4->replaceWidget(ui->labelPreview4, previewLabel4);
    ui->labelPreview4->deleteLater();

    connect(ui->spinBoxCustomWidth1, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(1); });
    connect(ui->spinBoxCustomHeight1, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(1); });
    connect(ui->checkBoxMaintainAspect1, &QCheckBox::toggled, this, [this](){ updateFormatPreview(1); });
    connect(ui->checkBoxCropToFit1, &QCheckBox::toggled, this, [this](){ updateFormatPreview(1); });

    connect(ui->spinBoxCustomWidth2, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(2); });
    connect(ui->spinBoxCustomHeight2, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(2); });
    connect(ui->checkBoxMaintainAspect2, &QCheckBox::toggled, this, [this](){ updateFormatPreview(2); });
    connect(ui->checkBoxCropToFit2, &QCheckBox::toggled, this, [this](){ updateFormatPreview(2); });

    connect(ui->spinBoxCustomWidth3, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(3); });
    connect(ui->spinBoxCustomHeight3, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(3); });
    connect(ui->checkBoxMaintainAspect3, &QCheckBox::toggled, this, [this](){ updateFormatPreview(3); });
    connect(ui->checkBoxCropToFit3, &QCheckBox::toggled, this, [this](){ updateFormatPreview(3); });

    connect(ui->spinBoxCustomWidth4, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(4); });
    connect(ui->spinBoxCustomHeight4, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ updateFormatPreview(4); });
    connect(ui->checkBoxMaintainAspect4, &QCheckBox::toggled, this, [this](){ updateFormatPreview(4); });
    connect(ui->checkBoxCropToFit4, &QCheckBox::toggled, this, [this](){ updateFormatPreview(4); });
}

GeneralSettingWidget::~GeneralSettingWidget()
{
    delete ui;
    delete virtualOutputWidget;
}

void GeneralSettingWidget::changeEvent(QEvent *e)
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

void GeneralSettingWidget::setSettings(GeneralSettings settings)
{
    mySettings = settings;
    loadSettings();
}

void GeneralSettingWidget::loadSettings()
{
    ui->checkBoxDisplayOnTop->setChecked(mySettings.displayIsOnTop);
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",QSettings::NativeFormat);
    if(settings.value("SoftProjectorUseLightTheme")==0)
        ui->checkBoxUseDarkTheme->setChecked(true);
    else
        ui->checkBoxUseDarkTheme->setChecked(false);
    ui->labelDarkThemeInfo->setToolTip(qApp->applicationDirPath()+"/DarkTheme.ini");
    ui->checkBoxDisplayOnStartUp->setChecked(mySettings.displayOnStartUp);

    // Load Themes
    loadThemes();
    ui->comboBoxTheme->setCurrentIndex(themeIdList.indexOf(mySettings.currentThemeId));

    // Get display screen infomration
    monitors.clear();

    int screen_count = QApplication::screens().count();
    int i = 1;
    ui->comboBoxDisplayScreen->clear();
    for(QScreen * s: QApplication::screens())
    {
        monitors << QString("%1 - %2x%3").arg(i).arg(s->geometry().width()).arg(s->geometry().height());
        ++i;
    }

    if(screen_count>1)
    {
        ui->groupBoxDisplayScreen->setEnabled(true);
    }
    else
    {
        ui->groupBoxDisplayScreen->setEnabled(false);
    }

    // Fill display screen comboBoxes
    ui->comboBoxDisplayScreen->clear();
    ui->comboBoxDisplayScreen->addItems(monitors);
    ui->comboBoxDisplayScreen_2->clear();
    ui->comboBoxDisplayScreen_2->addItem(tr("none"));
    ui->comboBoxDisplayScreen_2->addItems(monitors);
    ui->comboBoxDisplayScreen_3->clear();
    ui->comboBoxDisplayScreen_3->addItem(tr("none"));
    ui->comboBoxDisplayScreen_3->addItems(monitors);
    ui->comboBoxDisplayScreen_4->clear();
    ui->comboBoxDisplayScreen_4->addItem(tr("none"));
    ui->comboBoxDisplayScreen_4->addItems(monitors);

    // Set primary display screen
    if(mySettings.displayScreen<0 || mySettings.displayScreen>=screen_count)
        ui->comboBoxDisplayScreen->setCurrentIndex(0);
    else
        ui->comboBoxDisplayScreen->setCurrentIndex(mySettings.displayScreen);

    // Set secondary display screen
    if(mySettings.displayScreen2<-1 || mySettings.displayScreen2>=screen_count)
        ui->comboBoxDisplayScreen_2->setCurrentIndex(0);
    else
        ui->comboBoxDisplayScreen_2->setCurrentIndex(mySettings.displayScreen2+1);

    // Set third display screen
    if(mySettings.displayScreen3<-1 || mySettings.displayScreen3>=screen_count)
        ui->comboBoxDisplayScreen_3->setCurrentIndex(0);
    else
        ui->comboBoxDisplayScreen_3->setCurrentIndex(mySettings.displayScreen3+1);

    // Set fourth display screen
    if(mySettings.displayScreen4<-1 || mySettings.displayScreen4>=screen_count)
        ui->comboBoxDisplayScreen_4->setCurrentIndex(0);
    else
        ui->comboBoxDisplayScreen_4->setCurrentIndex(mySettings.displayScreen4+1);

    if(screen_count -1 <2)
    {
        ui->comboBoxDisplayScreen_2->setCurrentIndex(0);
        ui->comboBoxDisplayScreen_2->setEnabled(false);
        ui->comboBoxDisplayScreen_2->setToolTip("Display unavailable");
    }

    if(ui->comboBoxDisplayScreen_2->currentIndex() == 0)
        ui->comboBoxDisplayScreen_3->setEnabled(false);

    if(screen_count -1 <3)
    {
        ui->comboBoxDisplayScreen_3->setCurrentIndex(0);
        ui->comboBoxDisplayScreen_3->setEnabled(false);
        ui->comboBoxDisplayScreen_3->setToolTip("Display unavailable");
    }

    if(screen_count -1 <4)
    {
        ui->comboBoxDisplayScreen_4->setCurrentIndex(0);
        ui->comboBoxDisplayScreen_4->setEnabled(false);
        ui->comboBoxDisplayScreen_4->setToolTip("Display unavailable");
    }
    updateSecondaryDisplayScreen();

    // Set Display Controls
    if(screen_count>1)
        ui->groupBoxDisplayControls->setEnabled(false);
    else
        ui->groupBoxDisplayControls->setEnabled(true);
    ui->comboBoxIconSize->setCurrentIndex(mySettings.displayControls.buttonSize);
    ui->comboBoxControlsAlignV->setCurrentIndex(mySettings.displayControls.alignmentV);
    ui->comboBoxControlsAlignH->setCurrentIndex(mySettings.displayControls.alignmentH);
    ui->horizontalSliderOpacity->setValue(mySettings.displayControls.opacity*100);

    // Load Virtual Output settings
    virtualOutputWidget->setSettings(mySettings.virtualOutput);

    // Load Screen Format Settings
    loadScreenFormatSettings();
}

void GeneralSettingWidget::populateFormatComboBoxes()
{
    QStringList formatNames = {
        tr("Auto"),
        tr("4:3 (SD)"),
        tr("16:9 (HD)"),
        tr("16:10 (HD)"),
        tr("Ultrawide 21:9"),
        tr("Vertical 9:16"),
        tr("Custom")
    };

    ui->comboBoxFormat1->clear();
    ui->comboBoxFormat2->clear();
    ui->comboBoxFormat3->clear();
    ui->comboBoxFormat4->clear();

    ui->comboBoxFormat1->addItems(formatNames);
    ui->comboBoxFormat2->addItems(formatNames);
    ui->comboBoxFormat3->addItems(formatNames);
    ui->comboBoxFormat4->addItems(formatNames);

    updateCustomResolutionState(1);
    updateCustomResolutionState(2);
    updateCustomResolutionState(3);
    updateCustomResolutionState(4);
}

void GeneralSettingWidget::loadScreenFormatSettings()
{
    ui->comboBoxFormat1->setCurrentIndex(mySettings.screenFormat[0].aspectRatio);
    ui->comboBoxFormat2->setCurrentIndex(mySettings.screenFormat[1].aspectRatio);
    ui->comboBoxFormat3->setCurrentIndex(mySettings.screenFormat[2].aspectRatio);
    ui->comboBoxFormat4->setCurrentIndex(mySettings.screenFormat[3].aspectRatio);

    ui->spinBoxCustomWidth1->setValue(mySettings.screenFormat[0].customWidth);
    ui->spinBoxCustomWidth2->setValue(mySettings.screenFormat[1].customWidth);
    ui->spinBoxCustomWidth3->setValue(mySettings.screenFormat[2].customWidth);
    ui->spinBoxCustomWidth4->setValue(mySettings.screenFormat[3].customWidth);

    ui->spinBoxCustomHeight1->setValue(mySettings.screenFormat[0].customHeight);
    ui->spinBoxCustomHeight2->setValue(mySettings.screenFormat[1].customHeight);
    ui->spinBoxCustomHeight3->setValue(mySettings.screenFormat[2].customHeight);
    ui->spinBoxCustomHeight4->setValue(mySettings.screenFormat[3].customHeight);

    ui->checkBoxMaintainAspect1->setChecked(mySettings.screenFormat[0].maintainAspect);
    ui->checkBoxMaintainAspect2->setChecked(mySettings.screenFormat[1].maintainAspect);
    ui->checkBoxMaintainAspect3->setChecked(mySettings.screenFormat[2].maintainAspect);
    ui->checkBoxMaintainAspect4->setChecked(mySettings.screenFormat[3].maintainAspect);

    ui->checkBoxCropToFit1->setChecked(mySettings.screenFormat[0].cropToFit);
    ui->checkBoxCropToFit2->setChecked(mySettings.screenFormat[1].cropToFit);
    ui->checkBoxCropToFit3->setChecked(mySettings.screenFormat[2].cropToFit);
    ui->checkBoxCropToFit4->setChecked(mySettings.screenFormat[3].cropToFit);

    updateCustomResolutionState(1);
    updateCustomResolutionState(2);
    updateCustomResolutionState(3);
    updateCustomResolutionState(4);

    updateFormatPreview(1);
    updateFormatPreview(2);
    updateFormatPreview(3);
    updateFormatPreview(4);
}

void GeneralSettingWidget::updateFormatPreview(int screenIndex)
{
    QComboBox *comboBox;
    QSpinBox *widthSpinBox;
    QSpinBox *heightSpinBox;

    switch(screenIndex) {
    case 1:
        comboBox = ui->comboBoxFormat1;
        widthSpinBox = ui->spinBoxCustomWidth1;
        heightSpinBox = ui->spinBoxCustomHeight1;
        break;
    case 2:
        comboBox = ui->comboBoxFormat2;
        widthSpinBox = ui->spinBoxCustomWidth2;
        heightSpinBox = ui->spinBoxCustomHeight2;
        break;
    case 3:
        comboBox = ui->comboBoxFormat3;
        widthSpinBox = ui->spinBoxCustomWidth3;
        heightSpinBox = ui->spinBoxCustomHeight3;
        break;
    case 4:
        comboBox = ui->comboBoxFormat4;
        widthSpinBox = ui->spinBoxCustomWidth4;
        heightSpinBox = ui->spinBoxCustomHeight4;
        break;
    default:
        return;
    }

    bool isCustom = (comboBox->currentIndex() == FORMAT_CUSTOM);
    widthSpinBox->setEnabled(isCustom);
    heightSpinBox->setEnabled(isCustom);
}

void GeneralSettingWidget::updateFormatPreview(int screenIndex)
{
    QSpinBox *widthSpinBox;
    QSpinBox *heightSpinBox;
    QCheckBox *maintainAspectCheck;
    QCheckBox *cropToFitCheck;
    FormatPreviewLabel *previewLabel;
    QLabel *resolutionInfoLabel;

    switch(screenIndex) {
    case 1:
        widthSpinBox = ui->spinBoxCustomWidth1;
        heightSpinBox = ui->spinBoxCustomHeight1;
        maintainAspectCheck = ui->checkBoxMaintainAspect1;
        cropToFitCheck = ui->checkBoxCropToFit1;
        previewLabel = previewLabel1;
        resolutionInfoLabel = ui->labelResolutionInfo1;
        break;
    case 2:
        widthSpinBox = ui->spinBoxCustomWidth2;
        heightSpinBox = ui->spinBoxCustomHeight2;
        maintainAspectCheck = ui->checkBoxMaintainAspect2;
        cropToFitCheck = ui->checkBoxCropToFit2;
        previewLabel = previewLabel2;
        resolutionInfoLabel = ui->labelResolutionInfo2;
        break;
    case 3:
        widthSpinBox = ui->spinBoxCustomWidth3;
        heightSpinBox = ui->spinBoxCustomHeight3;
        maintainAspectCheck = ui->checkBoxMaintainAspect3;
        cropToFitCheck = ui->checkBoxCropToFit3;
        previewLabel = previewLabel3;
        resolutionInfoLabel = ui->labelResolutionInfo3;
        break;
    case 4:
        widthSpinBox = ui->spinBoxCustomWidth4;
        heightSpinBox = ui->spinBoxCustomHeight4;
        maintainAspectCheck = ui->checkBoxMaintainAspect4;
        cropToFitCheck = ui->checkBoxCropToFit4;
        previewLabel = previewLabel4;
        resolutionInfoLabel = ui->labelResolutionInfo4;
        break;
    default:
        return;
    }

    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    bool maintainAspect = maintainAspectCheck->isChecked();
    bool cropToFit = cropToFitCheck->isChecked();

    previewLabel->setFormat(width, height, maintainAspect, cropToFit);
    previewLabel->updatePreview(screenIndex);

    QString aspectRatio = previewLabel->getAspectRatioString(width, height);
    QString mode;
    if (!maintainAspect) {
        mode = tr("Stretched");
    } else if (cropToFit) {
        mode = tr("Cropped");
    } else {
        mode = tr("Letterboxed");
    }

    resolutionInfoLabel->setText(QString(
        "<b>%1</b><br>"
        "Resolution: %2x%3 (%4)<br>"
        "Aspect: %5")
        .arg(tr("Screen Format"))
        .arg(width)
        .arg(height)
        .arg(aspectRatio)
        .arg(mode));
}

void GeneralSettingWidget::applyFormatPreset(int screenIndex, int formatIndex)
{
    QSpinBox *widthSpinBox;
    QSpinBox *heightSpinBox;

    switch(screenIndex) {
    case 1:
        widthSpinBox = ui->spinBoxCustomWidth1;
        heightSpinBox = ui->spinBoxCustomHeight1;
        break;
    case 2:
        widthSpinBox = ui->spinBoxCustomWidth2;
        heightSpinBox = ui->spinBoxCustomHeight2;
        break;
    case 3:
        widthSpinBox = ui->spinBoxCustomWidth3;
        heightSpinBox = ui->spinBoxCustomHeight3;
        break;
    case 4:
        widthSpinBox = ui->spinBoxCustomWidth4;
        heightSpinBox = ui->spinBoxCustomHeight4;
        break;
    default:
        return;
    }

    switch(formatIndex) {
    case FORMAT_AUTO:
        widthSpinBox->setValue(1920);
        heightSpinBox->setValue(1080);
        break;
    case FORMAT_SD_4_3:
        widthSpinBox->setValue(1024);
        heightSpinBox->setValue(768);
        break;
    case FORMAT_HD_16_9:
        widthSpinBox->setValue(1920);
        heightSpinBox->setValue(1080);
        break;
    case FORMAT_HD_16_10:
        widthSpinBox->setValue(1920);
        heightSpinBox->setValue(1200);
        break;
    case FORMAT_ULTRAWIDE_21_9:
        widthSpinBox->setValue(2560);
        heightSpinBox->setValue(1080);
        break;
    case FORMAT_VERTICAL_9_16:
        widthSpinBox->setValue(1080);
        heightSpinBox->setValue(1920);
        break;
    case FORMAT_CUSTOM:
        break;
    }
}

void GeneralSettingWidget::loadThemes()
{
    QSqlQuery sq;
    sq.exec("SELECT id, name FROM Themes");
    themeIdList.clear();
    themes.clear();
    while(sq.next())
    {
        themeIdList<< sq.value(0).toInt();
        themes<<sq.value(1).toString();
    }
    ui->comboBoxTheme->clear();
    ui->comboBoxTheme->addItems(themes);
}

GeneralSettings GeneralSettingWidget::getSettings()
{
    mySettings.displayIsOnTop = ui->checkBoxDisplayOnTop->isChecked();
    mySettings.useDarkTheme = ui->checkBoxUseDarkTheme->isChecked();
    mySettings.displayOnStartUp = ui->checkBoxDisplayOnStartUp->isChecked();

    int tmx = ui->comboBoxTheme->currentIndex();
    if(tmx != -1)
        mySettings.currentThemeId = themeIdList.at(tmx);
    else
        mySettings.currentThemeId = 0;

    mySettings.displayScreen = ui->comboBoxDisplayScreen->currentIndex();
    mySettings.displayScreen2 = monitors.indexOf(ui->comboBoxDisplayScreen_2->currentText());
    mySettings.displayScreen3 = monitors.indexOf(ui->comboBoxDisplayScreen_3->currentText());
    mySettings.displayScreen4 = monitors.indexOf(ui->comboBoxDisplayScreen_4->currentText());

    mySettings.displayControls.buttonSize = ui->comboBoxIconSize->currentIndex();
    mySettings.displayControls.alignmentV = ui->comboBoxControlsAlignV->currentIndex();
    mySettings.displayControls.alignmentH = ui->comboBoxControlsAlignH->currentIndex();
    qreal r = ui->horizontalSliderOpacity->value();
    r = r/100;
    mySettings.displayControls.opacity = r;

    // Get Virtual Output settings
    virtualOutputWidget->getSettings(mySettings.virtualOutput);

    // Get Screen Format Settings
    mySettings.screenFormat[0].aspectRatio = ui->comboBoxFormat1->currentIndex();
    mySettings.screenFormat[1].aspectRatio = ui->comboBoxFormat2->currentIndex();
    mySettings.screenFormat[2].aspectRatio = ui->comboBoxFormat3->currentIndex();
    mySettings.screenFormat[3].aspectRatio = ui->comboBoxFormat4->currentIndex();

    mySettings.screenFormat[0].customWidth = ui->spinBoxCustomWidth1->value();
    mySettings.screenFormat[1].customWidth = ui->spinBoxCustomWidth2->value();
    mySettings.screenFormat[2].customWidth = ui->spinBoxCustomWidth3->value();
    mySettings.screenFormat[3].customWidth = ui->spinBoxCustomWidth4->value();

    mySettings.screenFormat[0].customHeight = ui->spinBoxCustomHeight1->value();
    mySettings.screenFormat[1].customHeight = ui->spinBoxCustomHeight2->value();
    mySettings.screenFormat[2].customHeight = ui->spinBoxCustomHeight3->value();
    mySettings.screenFormat[3].customHeight = ui->spinBoxCustomHeight4->value();

    mySettings.screenFormat[0].maintainAspect = ui->checkBoxMaintainAspect1->isChecked();
    mySettings.screenFormat[1].maintainAspect = ui->checkBoxMaintainAspect2->isChecked();
    mySettings.screenFormat[2].maintainAspect = ui->checkBoxMaintainAspect3->isChecked();
    mySettings.screenFormat[3].maintainAspect = ui->checkBoxMaintainAspect4->isChecked();

    mySettings.screenFormat[0].cropToFit = ui->checkBoxCropToFit1->isChecked();
    mySettings.screenFormat[1].cropToFit = ui->checkBoxCropToFit2->isChecked();
    mySettings.screenFormat[2].cropToFit = ui->checkBoxCropToFit3->isChecked();
    mySettings.screenFormat[3].cropToFit = ui->checkBoxCropToFit4->isChecked();

    return mySettings;
}

void GeneralSettingWidget::on_pushButtonDefault_clicked()
{
    GeneralSettings g;
    mySettings = g;
    loadSettings();
}

void GeneralSettingWidget::updateSecondaryDisplayScreen()
{
    QString ds1 = ui->comboBoxDisplayScreen->currentText();
    QString ds2 = ui->comboBoxDisplayScreen_2->currentText();
    QString ds3 = ui->comboBoxDisplayScreen_3->currentText();
    QString ds4 = ui->comboBoxDisplayScreen_4->currentText();
    QStringList monitors2 = monitors;
    //Display 2 (ds2)
    monitors2.removeOne(ds1);

    ui->comboBoxDisplayScreen_2->clear();
    ui->comboBoxDisplayScreen_2->addItem(tr("None"));
    ui->comboBoxDisplayScreen_2->addItems(monitors2);

    int i = ui->comboBoxDisplayScreen_2->findText(ds2);
    if(i != -1)
        ui->comboBoxDisplayScreen_2->setCurrentIndex(i);
    else
    {
        ui->comboBoxDisplayScreen_2->setCurrentIndex(0);
        emit setDisp2Use(false);
    }

    // Display 3 (ds3)
    monitors2.removeOne(ds1);
    monitors2.removeOne(ds2);

    ui->comboBoxDisplayScreen_3->clear();
    ui->comboBoxDisplayScreen_3->addItem(tr("None"));
    ui->comboBoxDisplayScreen_3->addItems(monitors2);

    i = ui->comboBoxDisplayScreen_3->findText(ds3);
    if(i != -1)
        ui->comboBoxDisplayScreen_3->setCurrentIndex(i);
    else
    {
        ui->comboBoxDisplayScreen_3->setCurrentIndex(0);
        emit setDisp3Use(false);
    }

    // Display 4 (ds4)
    monitors2.removeOne(ds1);
    monitors2.removeOne(ds2);
    monitors2.removeOne(ds3);

    ui->comboBoxDisplayScreen_4->clear();
    ui->comboBoxDisplayScreen_4->addItem(tr("None"));
    ui->comboBoxDisplayScreen_4->addItems(monitors2);

    i = ui->comboBoxDisplayScreen_4->findText(ds4);
    if(i != -1)
        ui->comboBoxDisplayScreen_4->setCurrentIndex(i);
    else
    {
        ui->comboBoxDisplayScreen_4->setCurrentIndex(0);
        emit setDisp4Use(false);
    }
}

void GeneralSettingWidget::on_comboBoxDisplayScreen_activated(const QString &arg1)
{
    updateSecondaryDisplayScreen();
}

void GeneralSettingWidget::on_comboBoxDisplayScreen_2_activated(int index)
{
    updateSecondaryDisplayScreen();
    int screen_count = QGuiApplication::screens().count();
    if(index<=0)
    {
        emit setDisp2Use(false);
        ui->comboBoxDisplayScreen_3->setEnabled(false);
    }
    else
    {
        emit setDisp2Use(true);
        if(screen_count -1 >=3)
            ui->comboBoxDisplayScreen_3->setEnabled(true);
    }
}

void GeneralSettingWidget::on_comboBoxDisplayScreen_3_activated(int index)
{
    updateSecondaryDisplayScreen();
    int screen_count = QGuiApplication::screens().count();
    if(index<=0)
    {
        emit setDisp3Use(false);
        ui->comboBoxDisplayScreen_4->setEnabled(false);
    }

    else
    {
        emit setDisp3Use(true);
        if(screen_count -1 >=4)
            ui->comboBoxDisplayScreen_4->setEnabled(true);
    }
}

void GeneralSettingWidget::on_comboBoxDisplayScreen_4_activated(int index)
{
    if(index<=0)
        emit setDisp4Use(false);
    else
        emit setDisp4Use(true);
}

void GeneralSettingWidget::on_pushButtonAddTheme_clicked()
{
    Theme tm;
    ThemeInfo tmi;
    int nId;

    AddSongbookDialog theme_dia;
    theme_dia.setWindowTitle(tr("Edit Theme"));
    theme_dia.setWindowText(tr("Theme Name:"),tr("Comments:"));
    theme_dia.setSongbook(tr("Default"),tr("This theme will contain program default settings."));
    int ret = theme_dia.exec();
    switch(ret)
    {
    case AddSongbookDialog::Accepted:
        tmi.name = theme_dia.title;
        tmi.comments = theme_dia.info;
        tm.setThemeInfo(tmi);
        tm.saveThemeNew();
        nId = tm.getThemeId();

        loadThemes();

        ui->comboBoxTheme->setCurrentIndex(themeIdList.indexOf(nId));
        emit themeChanged(nId);

        break;
    case AddSongbookDialog::Rejected:
        break;
    }
}

void GeneralSettingWidget::on_comboBoxTheme_activated(int index)
{
    emit themeChanged(themeIdList.at(index));
}

void GeneralSettingWidget::on_checkBoxUseDarkTheme_clicked()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if (ui->checkBoxUseDarkTheme->isChecked())
        settings.setValue("SoftProjectorUseLightTheme", 0);
    else
        settings.setValue("SoftProjectorUseLightTheme", 1);
    QMessageBox msgBox;
    msgBox.setText("Re-launch the application for the changes to take place.");
    msgBox.exec();
}

void GeneralSettingWidget::on_comboBoxFormat1_activated(int index)
{
    applyFormatPreset(1, index);
    updateCustomResolutionState(1);
    updateFormatPreview(1);
}

void GeneralSettingWidget::on_comboBoxFormat2_activated(int index)
{
    applyFormatPreset(2, index);
    updateCustomResolutionState(2);
    updateFormatPreview(2);
}

void GeneralSettingWidget::on_comboBoxFormat3_activated(int index)
{
    applyFormatPreset(3, index);
    updateCustomResolutionState(3);
    updateFormatPreview(3);
}

void GeneralSettingWidget::on_comboBoxFormat4_activated(int index)
{
    applyFormatPreset(4, index);
    updateCustomResolutionState(4);
    updateFormatPreview(4);
}
