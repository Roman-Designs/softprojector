#include "../headers/streamsettingswidget.hpp"

#include <QtWidgets>
#include <QNetworkInterface>
#include <QSqlQuery>

StreamSettingsWidget::StreamSettingsWidget(QWidget *parent) : QWidget(parent)
{
    auto *outer = new QVBoxLayout(this);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    outer->addWidget(scroll);
    auto *content = new QWidget;
    scroll->setWidget(content);
    auto *layout = new QVBoxLayout(content);
    layout->setSpacing(16);

    auto *heading = new QLabel(tr("Stream output"));
    QFont title = heading->font();
    title.setPointSize(title.pointSize() + 4);
    title.setBold(true);
    heading->setFont(title);
    layout->addWidget(heading);
    auto *description = new QLabel(tr("Independent graphics for an OBS Browser Source. The rest of the canvas stays transparent."));
    description->setWordWrap(true);
    layout->addWidget(description);

    m_enabled = new QCheckBox(tr("Enable on the local network"));
    layout->addWidget(m_enabled);
    auto *output = new QFormLayout;
    m_canvas = new QComboBox;
    m_canvas->addItems({tr("1920 × 1080"), tr("3840 × 2160")});
    output->addRow(tr("Canvas"), m_canvas);
    m_port = new QSpinBox;
    m_port->setRange(1024, 65535);
    output->addRow(tr("Port"), m_port);
    m_urls = new QComboBox;
    m_urls->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    auto *urlRow = new QHBoxLayout;
    urlRow->addWidget(m_urls, 1);
    auto *copy = new QPushButton(tr("Copy URL"));
    urlRow->addWidget(copy);
    output->addRow(tr("OBS Browser Source"), urlRow);
    layout->addLayout(output);
    auto *hint = new QLabel(tr("Press Apply to start the stream, then set the OBS source size to match the canvas."));
    hint->setWordWrap(true);
    layout->addWidget(hint);
    connect(copy, &QPushButton::clicked, this, [this] { QApplication::clipboard()->setText(m_urls->currentText()); });
    connect(m_port, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateUrls(); });

    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    layout->addWidget(separator);
    auto *settings = m_form = new QFormLayout;
    m_content = new QComboBox;
    m_content->addItems({tr("Bible"), tr("Songs"), tr("Announcements"), tr("Photos"), tr("Video")});
    settings->addRow(tr("Appearance for"), m_content);
    m_visible = new QCheckBox(tr("Show on stream"));
    settings->addRow(QString(), m_visible);
    connect(m_visible, &QCheckBox::toggled, this, [this] { updatePreview(); });
    m_translation = new QComboBox;
    m_translation->addItem(tr("Match main projector"), QString());
    settings->addRow(tr("Stream translation"), m_translation);
    m_preset = new QComboBox;
    m_preset->addItems({tr("Custom"), tr("Lower third"), tr("Full canvas"), tr("Picture in picture")});
    settings->addRow(tr("Layout preset"), m_preset);

    auto *geometry = new QGridLayout;
    QDoubleSpinBox *boxes[4] = {m_x = new QDoubleSpinBox, m_y = new QDoubleSpinBox,
                               m_width = new QDoubleSpinBox, m_height = new QDoubleSpinBox};
    const QString labels[] = {tr("Left"), tr("Top"), tr("Width"), tr("Height")};
    for (int i = 0; i < 4; ++i) {
        boxes[i]->setRange(i < 2 ? 0 : 5, i < 2 ? 95 : 100);
        boxes[i]->setDecimals(1);
        boxes[i]->setSuffix("%");
        geometry->addWidget(new QLabel(labels[i]), i/2, (i%2)*2);
        geometry->addWidget(boxes[i], i/2, (i%2)*2+1);
        connect(boxes[i], qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this] {
            m_preset->setCurrentIndex(0);
            updatePreview();
        });
    }
    connect(m_x, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this](double x) { m_width->setMaximum(100 - x); });
    connect(m_y, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this](double y) { m_height->setMaximum(100 - y); });
    settings->addRow(tr("Placement"), geometry);
    m_font = new QPushButton;
    settings->addRow(tr("Text font"), m_font);
    m_textColor = new QPushButton(tr("Choose text color…"));
    settings->addRow(tr("Text color"), m_textColor);
    m_panel = new QPushButton(tr("Choose panel color…"));
    settings->addRow(tr("Panel (alpha supported)"), m_panel);
    m_shadow = new QCheckBox(tr("Text shadow"));
    settings->addRow(QString(), m_shadow);
    m_details = new QCheckBox;
    settings->addRow(QString(), m_details);
    m_crop = new QCheckBox(tr("Crop photo to fill region"));
    settings->addRow(QString(), m_crop);
    layout->addLayout(settings);

    layout->addWidget(new QLabel(tr("Placement preview")));
    m_preview = new QLabel;
    m_preview->setMinimumHeight(170);
    m_preview->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_preview);
    layout->addStretch();

    connect(m_content, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        saveLayout();
        m_current = index;
        showLayout();
    });
    connect(m_preset, qOverload<int>(&QComboBox::activated), this, [this](int preset) {
        const QRectF r = preset == 1 ? QRectF(.10, .68, .80, .27)
                       : preset == 2 ? QRectF(.05, .05, .90, .90)
                       : preset == 3 ? QRectF(.66, .07, .30, .32) : m_settings.layouts[m_current].rect;
        m_x->setValue(r.x()*100); m_y->setValue(r.y()*100);
        m_width->setValue(r.width()*100); m_height->setValue(r.height()*100);
        m_preset->setCurrentIndex(preset);
        updatePreview();
    });
    connect(m_font, &QPushButton::clicked, this, [this] {
        bool accepted;
        const QFont font = QFontDialog::getFont(&accepted, m_settings.layouts[m_current].font, this);
        if (accepted) { m_settings.layouts[m_current].font = font; m_font->setText(font.family() + " · " + QString::number(font.pointSize()) + " pt"); }
    });
    connect(m_textColor, &QPushButton::clicked, this, [this] {
        const QColor color = QColorDialog::getColor(m_settings.layouts[m_current].textColor, this);
        if (color.isValid()) { m_settings.layouts[m_current].textColor = color; updatePreview(); }
    });
    connect(m_panel, &QPushButton::clicked, this, [this] {
        const QColor color = QColorDialog::getColor(m_settings.layouts[m_current].panel, this,
                                                    tr("Panel color"), QColorDialog::ShowAlphaChannel);
        if (color.isValid()) { m_settings.layouts[m_current].panel = color; updatePreview(); }
    });
    showLayout();
}

void StreamSettingsWidget::setSettings(const StreamSettings &settings)
{
    m_settings = settings;
    m_translation->clear();
    m_translation->addItem(tr("Match main projector"), QString());
    QSqlQuery bibles("SELECT id, bible_name FROM BibleVersions ORDER BY bible_name");
    while (bibles.next())
        m_translation->addItem(bibles.value(1).toString(), bibles.value(0).toString());
    if (!settings.bibleId.isEmpty() && m_translation->findData(settings.bibleId) < 0)
        m_translation->addItem(tr("Missing translation (%1)").arg(settings.bibleId), settings.bibleId);
    m_enabled->setChecked(settings.enabled);
    m_canvas->setCurrentIndex(settings.canvas.height() == 2160 ? 1 : 0);
    m_port->setValue(settings.port);
    m_translation->setCurrentIndex(qMax(0, m_translation->findData(settings.bibleId)));
    const QSignalBlocker block(m_content);
    m_content->setCurrentIndex(StreamBible);
    m_current = StreamBible;
    showLayout();
    updateUrls();
}

StreamSettings StreamSettingsWidget::getSettings()
{
    saveLayout();
    m_settings.enabled = m_enabled->isChecked();
    m_settings.canvas = m_canvas->currentIndex() ? QSize(3840, 2160) : QSize(1920, 1080);
    m_settings.port = quint16(m_port->value());
    m_settings.bibleId = m_translation->currentData().toString();
    return m_settings;
}

void StreamSettingsWidget::saveLayout()
{
    auto &layout = m_settings.layouts[m_current];
    layout.visible = m_visible->isChecked();
    const double x = m_x->value()/100, y = m_y->value()/100;
    layout.rect = QRectF(x, y, qMin(m_width->value()/100, 1.0-x), qMin(m_height->value()/100, 1.0-y));
    layout.shadow = m_shadow->isChecked();
    layout.showDetails = m_details->isChecked();
    layout.crop = m_crop->isChecked();
}

void StreamSettingsWidget::showLayout()
{
    const StreamLayout &layout = m_settings.layouts[m_current];
    const QSignalBlocker blockX(m_x), blockY(m_y), blockW(m_width), blockH(m_height);
    m_visible->setChecked(layout.visible);
    m_visible->setEnabled(m_current != StreamVideo);
    m_visible->setText(m_current == StreamVideo ? tr("Video stays transparent for now") : tr("Show on stream"));
    m_x->setValue(layout.rect.x()*100); m_y->setValue(layout.rect.y()*100);
    m_width->setMaximum(100 - m_x->value());
    m_height->setMaximum(100 - m_y->value());
    m_width->setValue(layout.rect.width()*100); m_height->setValue(layout.rect.height()*100);
    m_preset->setCurrentIndex(0);
    m_shadow->setChecked(layout.shadow);
    m_details->setChecked(layout.showDetails);
    m_crop->setChecked(layout.crop);
    m_font->setText(layout.font.family() + " · " + QString::number(layout.font.pointSize()) + " pt");
    m_translation->setVisible(m_current == StreamBible);
    m_form->labelForField(m_translation)->setVisible(m_current == StreamBible);
    m_details->setVisible(m_current == StreamBible || m_current == StreamSong);
    m_details->setText(m_current == StreamBible ? tr("Show verse reference") : tr("Show stanza title"));
    m_crop->setVisible(m_current == StreamPicture);
    m_preset->setEnabled(m_current != StreamVideo);
    for (QDoubleSpinBox *box : {m_x, m_y, m_width, m_height})
        box->setEnabled(m_current != StreamVideo);
    m_font->setVisible(m_current <= StreamAnnouncement);
    m_form->labelForField(m_font)->setVisible(m_current <= StreamAnnouncement);
    m_textColor->setVisible(m_current <= StreamAnnouncement);
    m_form->labelForField(m_textColor)->setVisible(m_current <= StreamAnnouncement);
    m_shadow->setVisible(m_current <= StreamAnnouncement);
    m_panel->setVisible(m_current != StreamVideo);
    m_form->labelForField(m_panel)->setVisible(m_current != StreamVideo);
    updatePreview();
}

void StreamSettingsWidget::updateUrls()
{
    m_urls->clear();
    const QString suffix = ":" + QString::number(m_port->value()) + "/stream";
    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsUp) || (interface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            const QHostAddress address = entry.ip();
            if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLinkLocal())
                m_urls->addItem("http://" + address.toString() + suffix);
        }
    }
    if (!m_urls->count()) m_urls->addItem("http://127.0.0.1" + suffix);
}

void StreamSettingsWidget::updatePreview()
{
    QPixmap preview(400, 225);
    preview.fill(QColor(43, 47, 50));
    QPainter painter(&preview);
    for (int y = 0; y < preview.height(); y += 20)
        for (int x = 0; x < preview.width(); x += 20)
            if ((x+y)/20 % 2) painter.fillRect(x, y, 20, 20, QColor(56, 60, 63));
    const QRect r(qRound(m_x->value()*4), qRound(m_y->value()*2.25),
                  qRound(m_width->value()*4), qRound(m_height->value()*2.25));
    if (m_current != StreamVideo && m_visible->isChecked()) {
        painter.fillRect(r, m_settings.layouts[m_current].panel);
        painter.setPen(m_settings.layouts[m_current].textColor);
        painter.drawRect(r.adjusted(0, 0, -1, -1));
        painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, m_content->currentText());
    }
    m_preview->setPixmap(preview.scaledToWidth(380, Qt::SmoothTransformation));
}
