#ifndef STREAMSETTINGSWIDGET_HPP
#define STREAMSETTINGSWIDGET_HPP

#include <QWidget>
#include "streamoutput.hpp"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QPushButton;
class QSpinBox;
class StreamPlacementPreview;

class StreamSettingsWidget : public QWidget
{
public:
    explicit StreamSettingsWidget(QWidget *parent = nullptr);
    void setSettings(const StreamSettings &settings);
    StreamSettings getSettings();

private:
    void saveLayout();
    void showLayout();
    void updateUrls();
    void updatePreview();
    QRectF placement() const;
    void setPlacement(const QRectF &rect);

    StreamSettings m_settings;
    int m_current = StreamBible;
    QFormLayout *m_form;
    QCheckBox *m_enabled, *m_visible, *m_shadow, *m_details, *m_crop, *m_backgroundEnabled;
    QComboBox *m_canvas, *m_urls, *m_translation, *m_content, *m_preset, *m_backgroundScope;
    QSpinBox *m_port;
    QDoubleSpinBox *m_x, *m_y, *m_width, *m_height;
    QPushButton *m_font, *m_textColor, *m_panel;
    StreamPlacementPreview *m_preview;
    QPushButton *m_fineTune;
    QWidget *m_geometry;
    QWidget *m_backgroundControls;
    QPushButton *m_backgroundChoose, *m_backgroundRemove;
    QLabel *m_backgroundName;
};

#endif // STREAMSETTINGSWIDGET_HPP
