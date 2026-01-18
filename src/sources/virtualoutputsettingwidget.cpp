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

#include "../headers/virtualoutputsettingwidget.hpp"
#include "ui_virtualoutputsettingwidget.h"

VirtualOutputSettingWidget::VirtualOutputSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VirtualOutputSettingWidget)
{
    ui->setupUi(this);
    loadThemes();
    updateCustomResolutionVisibility();
    updateThemeComboState();
    updateFontPreview();
    updateColorPreview(ui->graphicsViewLowerThirdBgColor, QColor(0, 0, 0));
    updateColorPreview(ui->graphicsViewLowerThirdTextColor, QColor(255, 255, 255));

    connect(ui->comboBoxResolution, QOverload<int>::of(&QComboBox::activated),
            this, &VirtualOutputSettingWidget::on_comboBoxResolution_activated);
    connect(ui->radioButtonMirrorDisplay1, &QRadioButton::toggled,
            this, &VirtualOutputSettingWidget::on_radioButtonMirrorDisplay1_toggled);
    connect(ui->radioButtonCustomTheme, &QRadioButton::toggled,
            this, &VirtualOutputSettingWidget::on_radioButtonCustomTheme_toggled);
    connect(ui->fontComboBoxLowerThird, &QFontComboBox::currentFontChanged,
            this, &VirtualOutputSettingWidget::on_fontComboBoxLowerThird_currentFontChanged);
}

VirtualOutputSettingWidget::~VirtualOutputSettingWidget()
{
    delete ui;
}

void VirtualOutputSettingWidget::setSettings(VirtualOutputSettings &settings)
{
    m_settings = settings;
    loadSettings();
}

void VirtualOutputSettingWidget::getSettings(VirtualOutputSettings &settings)
{
    saveSettings();
    settings = m_settings;
}

void VirtualOutputSettingWidget::loadSettings()
{
    ui->checkBoxEnable->setChecked(m_settings.enabled);

    if (m_settings.width == 1280 && m_settings.height == 720) {
        ui->comboBoxResolution->setCurrentIndex(0);
    } else if (m_settings.width == 1920 && m_settings.height == 1080) {
        ui->comboBoxResolution->setCurrentIndex(1);
    } else if (m_settings.width == 2560 && m_settings.height == 1440) {
        ui->comboBoxResolution->setCurrentIndex(2);
    } else {
        ui->comboBoxResolution->setCurrentIndex(3);
    }

    ui->spinBoxWidth->setValue(m_settings.width);
    ui->spinBoxHeight->setValue(m_settings.height);

    if (m_settings.mirrorDisplay1) {
        ui->radioButtonMirrorDisplay1->setChecked(true);
    } else {
        ui->radioButtonCustomTheme->setChecked(true);
    }

    int themeIndex = m_themeIdList.indexOf(m_settings.streamThemeId);
    if (themeIndex >= 0) {
        ui->comboBoxTheme->setCurrentIndex(themeIndex);
    } else {
        ui->comboBoxTheme->setCurrentIndex(0);
    }

    ui->lineEditOverlayPath->setText(m_settings.overlayPath);

    ui->checkBoxAlwaysOnTop->setChecked(m_settings.displayIsOnTop);

    updateCustomResolutionVisibility();
    updateThemeComboState();
    updateOverlayPreview(m_settings.overlayPath);
}

void VirtualOutputSettingWidget::saveSettings()
{
    m_settings.enabled = ui->checkBoxEnable->isChecked();

    int resolutionIndex = ui->comboBoxResolution->currentIndex();
    switch (resolutionIndex) {
    case 0:
        m_settings.width = 1280;
        m_settings.height = 720;
        break;
    case 1:
        m_settings.width = 1920;
        m_settings.height = 1080;
        break;
    case 2:
        m_settings.width = 2560;
        m_settings.height = 1440;
        break;
    case 3:
    default:
        m_settings.width = ui->spinBoxWidth->value();
        m_settings.height = ui->spinBoxHeight->value();
        break;
    }

    m_settings.mirrorDisplay1 = ui->radioButtonMirrorDisplay1->isChecked();
    if (!m_settings.mirrorDisplay1) {
        int themeIndex = ui->comboBoxTheme->currentIndex();
        if (themeIndex >= 0 && themeIndex < m_themeIdList.size()) {
            m_settings.streamThemeId = m_themeIdList.at(themeIndex);
        }
    }

    m_settings.overlayPath = ui->lineEditOverlayPath->text();

    m_settings.displayIsOnTop = ui->checkBoxAlwaysOnTop->isChecked();
}

void VirtualOutputSettingWidget::loadThemes()
{
    QSqlQuery sq;
    sq.exec("SELECT id, name FROM Themes ORDER BY name");
    m_themeIdList.clear();
    m_themes.clear();
    while (sq.next()) {
        m_themeIdList << sq.value(0).toInt();
        m_themes << sq.value(1).toString();
    }
    ui->comboBoxTheme->clear();
    ui->comboBoxTheme->addItems(m_themes);
}

QString VirtualOutputSettingWidget::getFontText(QFont font)
{
    QString f = font.toString().split(",").at(0);
    QString st = QString("%1: %2").arg(f).arg(font.pointSize());
    if (font.bold())
        st += ", " + tr("Bold");
    if (font.italic())
        st += ", " + tr("Italic");
    if (font.strikeOut())
        st += ", " + tr("StrikeOut");
    if (font.underline())
        st += ", " + tr("Underline");
    return st;
}

void VirtualOutputSettingWidget::updateColorPreview(QGraphicsView *graphicsView, const QColor &color)
{
    QPalette p;
    p.setColor(QPalette::Base, color);
    graphicsView->setPalette(p);
}

void VirtualOutputSettingWidget::updateFontPreview()
{
    QFont font = ui->fontComboBoxLowerThird->currentFont();
    ui->labelLowerThirdFontPreview->setFont(font);
    ui->labelLowerThirdFontPreview->setText(getFontText(font));
}

void VirtualOutputSettingWidget::updateCustomResolutionVisibility()
{
    bool isCustom = ui->comboBoxResolution->currentIndex() == 3;
    ui->horizontalLayoutCustomResolution->setEnabled(isCustom);
    ui->spinBoxWidth->setEnabled(isCustom);
    ui->spinBoxHeight->setEnabled(isCustom);
}

void VirtualOutputSettingWidget::updateThemeComboState()
{
    bool useCustom = ui->radioButtonCustomTheme->isChecked();
    ui->comboBoxTheme->setEnabled(useCustom);
}

void VirtualOutputSettingWidget::updateOverlayPreview(const QString &path)
{
    if (path.isEmpty()) {
        ui->labelOverlayImage->clear();
        ui->labelOverlayImage->setText(tr("No image"));
        return;
    }

    QPixmap pixmap(path);
    if (!pixmap.isNull()) {
        ui->labelOverlayImage->setPixmap(pixmap.scaled(200, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelOverlayImage->clear();
        ui->labelOverlayImage->setText(tr("Invalid image"));
    }
}

void VirtualOutputSettingWidget::on_checkBoxEnable_toggled(bool checked)
{
    Q_UNUSED(checked)
    emit update();
}

void VirtualOutputSettingWidget::on_comboBoxResolution_activated(int index)
{
    Q_UNUSED(index)
    updateCustomResolutionVisibility();
    if (index == 0) {
        ui->spinBoxWidth->setValue(1280);
        ui->spinBoxHeight->setValue(720);
    } else if (index == 1) {
        ui->spinBoxWidth->setValue(1920);
        ui->spinBoxHeight->setValue(1080);
    } else if (index == 2) {
        ui->spinBoxWidth->setValue(2560);
        ui->spinBoxHeight->setValue(1440);
    }
    emit update();
}

void VirtualOutputSettingWidget::on_radioButtonMirrorDisplay1_toggled(bool checked)
{
    Q_UNUSED(checked)
    updateThemeComboState();
    emit update();
}

void VirtualOutputSettingWidget::on_radioButtonCustomTheme_toggled(bool checked)
{
    Q_UNUSED(checked)
    updateThemeComboState();
    emit update();
}

void VirtualOutputSettingWidget::on_toolButtonBrowseOverlay_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select overlay image"),
        ".",
        tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.svg)"));
    if (!filename.isNull()) {
        ui->lineEditOverlayPath->setText(filename);
        updateOverlayPreview(filename);
        emit update();
    }
}

void VirtualOutputSettingWidget::on_toolButtonClearOverlay_clicked()
{
    ui->lineEditOverlayPath->clear();
    updateOverlayPreview(QString());
    emit update();
}

void VirtualOutputSettingWidget::on_toolButtonLowerThirdFont_clicked()
{
    bool ok;
    QFont currentFont = ui->fontComboBoxLowerThird->currentFont();
    QFont font = QFontDialog::getFont(&ok, currentFont, this);
    if (ok) {
        ui->fontComboBoxLowerThird->setCurrentFont(font);
        updateFontPreview();
        emit update();
    }
}

void VirtualOutputSettingWidget::on_toolButtonLowerThirdBgColor_clicked()
{
    QColor currentColor = ui->graphicsViewLowerThirdBgColor->palette().color(QPalette::Base);
    QColor color = QColorDialog::getColor(currentColor, this);
    if (color.isValid()) {
        updateColorPreview(ui->graphicsViewLowerThirdBgColor, color);
        emit update();
    }
}

void VirtualOutputSettingWidget::on_toolButtonLowerThirdTextColor_clicked()
{
    QColor currentColor = ui->graphicsViewLowerThirdTextColor->palette().color(QPalette::Base);
    QColor color = QColorDialog::getColor(currentColor, this);
    if (color.isValid()) {
        updateColorPreview(ui->graphicsViewLowerThirdTextColor, color);
        emit update();
    }
}

void VirtualOutputSettingWidget::on_fontComboBoxLowerThird_currentFontChanged(const QFont &font)
{
    Q_UNUSED(font)
    updateFontPreview();
    emit update();
}

void VirtualOutputSettingWidget::on_pushButtonPreview_clicked()
{
    emit previewRequested();
}
