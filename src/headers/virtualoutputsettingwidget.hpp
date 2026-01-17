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

#ifndef VIRTUALOUTPUTSETTINGWIDGET_HPP
#define VIRTUALOUTPUTSETTINGWIDGET_HPP

#include <QtWidgets>
#include <QtSql>
#include "settings.hpp"

namespace Ui {
class VirtualOutputSettingWidget;
}

class VirtualOutputSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualOutputSettingWidget(QWidget *parent = 0);
    ~VirtualOutputSettingWidget();

public slots:
    void setSettings(VirtualOutputSettings &settings);
    void getSettings(VirtualOutputSettings &settings);

signals:
    void update();
    void previewRequested();

private slots:
    void on_checkBoxEnable_toggled(bool checked);
    void on_comboBoxResolution_activated(int index);
    void on_radioButtonMirrorDisplay1_toggled(bool checked);
    void on_radioButtonCustomTheme_toggled(bool checked);
    void on_toolButtonBrowseOverlay_clicked();
    void on_toolButtonClearOverlay_clicked();
    void on_toolButtonLowerThirdFont_clicked();
    void on_toolButtonLowerThirdBgColor_clicked();
    void on_toolButtonLowerThirdTextColor_clicked();
    void on_fontComboBoxLowerThird_currentFontChanged(const QFont &font);
    void on_pushButtonPreview_clicked();

    void updateCustomResolutionVisibility();
    void updateThemeComboState();
    void updateOverlayPreview(const QString &path);

private:
    void loadSettings();
    void saveSettings();
    QString getFontText(QFont font);
    void updateColorPreview(QGraphicsView *graphicsView, const QColor &color);
    void updateFontPreview();

    VirtualOutputSettings m_settings;
    QStringList m_themes;
    QList<int> m_themeIdList;
    Ui::VirtualOutputSettingWidget *ui;
};

#endif // VIRTUALOUTPUTSETTINGWIDGET_HPP
