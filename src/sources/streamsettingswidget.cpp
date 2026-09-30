#include "../headers/streamsettingswidget.hpp"

#include <QtWidgets>
#include <QNetworkInterface>
#include <QSqlQuery>
#include <functional>

namespace {
QRectF presetRect(int preset)
{
    switch (preset) {
    case 1: return QRectF(.10, .68, .80, .27);
    case 2: return QRectF(0, 0, 1, 1);
    case 3: return QRectF(.66, .07, .30, .32);
    case 4: return QRectF(.10, .05, .80, .27);
    case 5: return QRectF(.15, .25, .70, .50);
    default: return {};
    }
}

int matchingPreset(const QRectF &rect)
{
    for (int i = 1; i <= 5; ++i) {
        const QRectF preset = presetRect(i);
        if (qAbs(rect.x() - preset.x()) < .0001 && qAbs(rect.y() - preset.y()) < .0001 &&
            qAbs(rect.width() - preset.width()) < .0001 && qAbs(rect.height() - preset.height()) < .0001)
            return i;
    }
    return 0;
}
}

class StreamPlacementPreview : public QWidget
{
public:
    explicit StreamPlacementPreview(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName("streamPlacementPreview");
        setMinimumSize(240, 150);
        setMaximumHeight(290);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setAccessibleName(tr("Stream placement preview"));
        setAccessibleDescription(tr("Drag the box to move it. Drag the bottom-right corner to resize. Arrow keys move; Shift plus arrow keys resize."));
        setToolTip(accessibleDescription());
    }

    QSize sizeHint() const override { return QSize(480, 270); }
    std::function<void(const QRectF &)> placementChanged;

    void setLayout(const StreamLayout &layout, const QString &label, bool editable)
    {
        m_layout = layout;
        m_label = label;
        setEnabled(editable);
        if (!editable) m_dragging = false;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        const QRect screen = screenRect().toAlignedRect();
        painter.fillRect(screen, QColor(43, 47, 50));
        painter.save();
        painter.setClipRect(screen);
        for (int y = screen.top(); y < screen.bottom(); y += 16)
            for (int x = screen.left(); x < screen.right(); x += 16)
                if (((x - screen.left()) / 16 + (y - screen.top()) / 16) % 2)
                    painter.fillRect(x, y, 16, 16, QColor(56, 60, 63));
        const QRectF box = boxRect();
        if (isEnabled()) {
            m_layout.paintBackground(painter, screen, box.toAlignedRect());
            painter.setPen(m_layout.visible ? m_layout.textColor : QColor(Qt::white));
            painter.drawText(box.adjusted(8, 4, -8, -4), Qt::AlignCenter | Qt::TextWordWrap,
                             m_layout.visible ? m_label : tr("Hidden on stream"));
            painter.setPen(QPen(QColor(105, 190, 255), hasFocus() ? 3 : 2));
            painter.drawRect(box.adjusted(1, 1, -1, -1));
            painter.fillRect(resizeHandle(), QColor(105, 190, 255));
        } else {
            painter.setPen(Qt::white);
            painter.drawText(screen, Qt::AlignCenter, tr("Video stays transparent"));
        }
        painter.restore();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() != Qt::LeftButton) return;
        m_resizing = resizeHandle().adjusted(-5, -5, 5, 5).contains(event->position());
        if (!m_resizing && !boxRect().contains(event->position())) return;
        setFocus(Qt::MouseFocusReason);
        m_dragging = true;
        m_start = event->position();
        m_startRect = m_layout.rect;
        setCursor(m_resizing ? Qt::SizeFDiagCursor : Qt::ClosedHandCursor);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (!m_dragging) {
            setCursor(resizeHandle().adjusted(-5, -5, 5, 5).contains(event->position()) ? Qt::SizeFDiagCursor
                      : boxRect().contains(event->position()) ? Qt::OpenHandCursor : Qt::ArrowCursor);
            return;
        }
        const QRectF screen = screenRect();
        const QPointF delta((event->position().x() - m_start.x()) / screen.width(),
                            (event->position().y() - m_start.y()) / screen.height());
        QRectF rect = m_startRect;
        if (m_resizing) {
            rect.setWidth(qBound(.05, rect.width() + delta.x(), 1.0 - rect.x()));
            rect.setHeight(qBound(.05, rect.height() + delta.y(), 1.0 - rect.y()));
        } else {
            rect.moveTo(qBound(0.0, rect.x() + delta.x(), 1.0 - rect.width()),
                        qBound(0.0, rect.y() + delta.y(), 1.0 - rect.height()));
        }
        if (placementChanged) placementChanged(rect);
    }

    void mouseReleaseEvent(QMouseEvent *) override
    {
        m_dragging = false;
        setCursor(Qt::OpenHandCursor);
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        const double dx = event->key() == Qt::Key_Left ? -.01 : event->key() == Qt::Key_Right ? .01 : 0;
        const double dy = event->key() == Qt::Key_Up ? -.01 : event->key() == Qt::Key_Down ? .01 : 0;
        if (!dx && !dy) { QWidget::keyPressEvent(event); return; }
        QRectF rect = m_layout.rect;
        if (event->modifiers() & Qt::ShiftModifier) {
            rect.setWidth(qBound(.05, rect.width() + dx, 1.0 - rect.x()));
            rect.setHeight(qBound(.05, rect.height() + dy, 1.0 - rect.y()));
        } else {
            rect.moveTo(qBound(0.0, rect.x() + dx, 1.0 - rect.width()),
                        qBound(0.0, rect.y() + dy, 1.0 - rect.height()));
        }
        if (placementChanged) placementChanged(rect);
    }

    void focusInEvent(QFocusEvent *event) override { QWidget::focusInEvent(event); update(); }
    void focusOutEvent(QFocusEvent *event) override { QWidget::focusOutEvent(event); update(); }

private:
    QRectF screenRect() const
    {
        const double width = qMin(double(this->width() - 16), (height() - 16) * 16.0 / 9);
        const double height = width * 9 / 16;
        return QRectF((this->width() - width) / 2, (this->height() - height) / 2, width, height);
    }
    QRectF boxRect() const
    {
        const QRectF screen = screenRect(), rect = m_layout.rect;
        return QRectF(screen.x() + rect.x() * screen.width(), screen.y() + rect.y() * screen.height(),
                      rect.width() * screen.width(), rect.height() * screen.height());
    }
    QRectF resizeHandle() const { return QRectF(boxRect().bottomRight() - QPointF(12, 12), QSizeF(12, 12)); }
    StreamLayout m_layout;
    QString m_label;
    bool m_dragging = false, m_resizing = false;
    QPointF m_start;
    QRectF m_startRect;
};

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
    auto *description = new QLabel(tr("Independent graphics for an OBS Browser Source. The canvas stays transparent unless you add a full-canvas background."));
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
    m_preset->setObjectName("streamPlacementPreset");
    m_preset->addItems({tr("Custom position"), tr("Bottom banner (lower third)"), tr("Full screen"),
                        tr("Small overlay (top right)"), tr("Top banner"), tr("Centered box")});
    settings->addRow(tr("Placement"), m_preset);

    auto *placementControls = new QWidget;
    auto *placementLayout = new QVBoxLayout(placementControls);
    placementLayout->setContentsMargins(0, 0, 0, 0);
    m_preview = new StreamPlacementPreview;
    placementLayout->addWidget(m_preview);
    auto *placementHint = new QLabel(tr("Choose a placement above, or drag the box to move it. Drag its bottom-right corner to resize."));
    placementHint->setWordWrap(true);
    placementLayout->addWidget(placementHint);
    m_fineTune = new QPushButton(tr("Fine-tune placement"));
    m_fineTune->setObjectName("streamFineTunePlacement");
    m_fineTune->setCheckable(true);
    placementLayout->addWidget(m_fineTune, 0, Qt::AlignLeft);
    m_geometry = new QWidget;
    m_geometry->setObjectName("streamExactPlacement");
    m_geometry->setVisible(false);
    placementLayout->addWidget(m_geometry);
    connect(m_fineTune, &QPushButton::toggled, m_geometry, &QWidget::setVisible);
    settings->addRow(QString(), placementControls);

    auto *geometry = new QGridLayout(m_geometry);
    QDoubleSpinBox *boxes[4] = {m_x = new QDoubleSpinBox, m_y = new QDoubleSpinBox,
                               m_width = new QDoubleSpinBox, m_height = new QDoubleSpinBox};
    const QString labels[] = {tr("Left"), tr("Top"), tr("Width"), tr("Height")};
    const QString names[] = {"streamLeft", "streamTop", "streamWidth", "streamHeight"};
    for (int i = 0; i < 4; ++i) {
        boxes[i]->setObjectName(names[i]);
        boxes[i]->setRange(i < 2 ? 0 : 5, i < 2 ? 95 : 100);
        boxes[i]->setDecimals(1);
        boxes[i]->setSuffix("%");
        auto *label = new QLabel(labels[i]);
        label->setBuddy(boxes[i]);
        geometry->addWidget(label, i/2, (i%2)*2);
        geometry->addWidget(boxes[i], i/2, (i%2)*2+1);
        connect(boxes[i], qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this] {
            m_width->setMaximum(100 - m_x->value());
            m_height->setMaximum(100 - m_y->value());
            m_preset->setCurrentIndex(matchingPreset(placement()));
            updatePreview();
        });
    }
    m_preview->placementChanged = [this](const QRectF &rect) { setPlacement(rect); };
    m_font = new QPushButton;
    settings->addRow(tr("Text font"), m_font);
    m_textColor = new QPushButton(tr("Choose text color…"));
    settings->addRow(tr("Text color"), m_textColor);
    m_panel = new QPushButton(tr("Choose panel color…"));
    settings->addRow(tr("Panel (alpha supported)"), m_panel);
    m_backgroundControls = new QWidget;
    auto *backgroundLayout = new QVBoxLayout(m_backgroundControls);
    backgroundLayout->setContentsMargins(0, 0, 0, 0);
    m_backgroundEnabled = new QCheckBox(tr("Use background image"));
    m_backgroundEnabled->setObjectName("streamBackgroundEnabled");
    backgroundLayout->addWidget(m_backgroundEnabled);
    auto *backgroundButtons = new QHBoxLayout;
    m_backgroundChoose = new QPushButton(tr("Choose image…"));
    m_backgroundRemove = new QPushButton(tr("Remove"));
    backgroundButtons->addWidget(m_backgroundChoose);
    backgroundButtons->addWidget(m_backgroundRemove);
    backgroundLayout->addLayout(backgroundButtons);
    m_backgroundName = new QLabel;
    m_backgroundName->setWordWrap(true);
    backgroundLayout->addWidget(m_backgroundName);
    m_backgroundScope = new QComboBox;
    m_backgroundScope->setObjectName("streamBackgroundScope");
    m_backgroundScope->addItems({tr("Fill content region"), tr("Fill entire canvas")});
    backgroundLayout->addWidget(m_backgroundScope);
    settings->addRow(tr("Background image"), m_backgroundControls);
    m_shadow = new QCheckBox(tr("Text shadow"));
    settings->addRow(QString(), m_shadow);
    m_details = new QCheckBox;
    settings->addRow(QString(), m_details);
    m_crop = new QCheckBox(tr("Crop photo to fill region"));
    settings->addRow(QString(), m_crop);
    layout->addLayout(settings);

    layout->addStretch();

    connect(m_content, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        saveLayout();
        m_current = index;
        showLayout();
    });
    connect(m_preset, qOverload<int>(&QComboBox::activated), this, [this](int preset) {
        if (preset > 0) setPlacement(presetRect(preset));
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
    connect(m_backgroundChoose, &QPushButton::clicked, this, [this] {
        const QString path = QFileDialog::getOpenFileName(this, tr("Choose stream background"), QString(),
                                                        tr("Images (*.png *.jpg *.jpeg *.bmp *.webp);;All files (*)"));
        if (path.isEmpty()) return;
        const QImage image(path);
        if (image.isNull()) {
            QMessageBox::warning(this, tr("Cannot load image"), tr("The selected file could not be loaded as an image."));
            return;
        }
        auto &layout = m_settings.layouts[m_current];
        layout.background = image;
        layout.backgroundName = QFileInfo(path).fileName();
        m_backgroundName->setText(layout.backgroundName);
        m_backgroundRemove->setEnabled(true);
        m_backgroundEnabled->setChecked(true);
        updatePreview();
    });
    connect(m_backgroundRemove, &QPushButton::clicked, this, [this] {
        auto &layout = m_settings.layouts[m_current];
        layout.background = QImage();
        layout.backgroundName.clear();
        m_backgroundName->setText(tr("No image selected"));
        m_backgroundRemove->setEnabled(false);
        m_backgroundEnabled->setChecked(false);
        updatePreview();
    });
    connect(m_backgroundEnabled, &QCheckBox::toggled, this, [this] { updatePreview(); });
    connect(m_backgroundScope, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] { updatePreview(); });
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
    layout.rect = placement();
    layout.shadow = m_shadow->isChecked();
    layout.showDetails = m_details->isChecked();
    layout.crop = m_crop->isChecked();
    layout.useBackground = m_backgroundEnabled->isChecked();
    layout.backgroundFullCanvas = m_backgroundScope->currentIndex() == 1;
}

void StreamSettingsWidget::showLayout()
{
    const StreamLayout &layout = m_settings.layouts[m_current];
    const QSignalBlocker blockX(m_x), blockY(m_y), blockW(m_width), blockH(m_height);
    m_visible->setChecked(layout.visible);
    m_visible->setEnabled(m_current != StreamVideo);
    m_visible->setText(m_current == StreamVideo ? tr("Video stays transparent for now") : tr("Show on stream"));
    setPlacement(layout.rect);
    m_shadow->setChecked(layout.shadow);
    m_details->setChecked(layout.showDetails);
    m_crop->setChecked(layout.crop);
    const QSignalBlocker blockBackground(m_backgroundEnabled), blockScope(m_backgroundScope);
    m_backgroundEnabled->setChecked(layout.useBackground);
    m_backgroundScope->setCurrentIndex(layout.backgroundFullCanvas ? 1 : 0);
    m_backgroundName->setText(layout.background.isNull() ? tr("No image selected") : layout.backgroundName);
    m_backgroundRemove->setEnabled(!layout.background.isNull());
    m_backgroundControls->setVisible(m_current != StreamVideo);
    m_form->labelForField(m_backgroundControls)->setVisible(m_current != StreamVideo);
    m_font->setText(layout.font.family() + " · " + QString::number(layout.font.pointSize()) + " pt");
    m_translation->setVisible(m_current == StreamBible);
    m_form->labelForField(m_translation)->setVisible(m_current == StreamBible);
    m_details->setVisible(m_current == StreamBible || m_current == StreamSong);
    m_details->setText(m_current == StreamBible ? tr("Show verse reference") : tr("Show stanza title"));
    m_crop->setVisible(m_current == StreamPicture);
    m_preset->setEnabled(m_current != StreamVideo);
    m_fineTune->setEnabled(m_current != StreamVideo);
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
    StreamLayout layout = m_settings.layouts[m_current];
    layout.visible = m_visible->isChecked();
    layout.rect = placement();
    layout.useBackground = m_backgroundEnabled->isChecked();
    layout.backgroundFullCanvas = m_backgroundScope->currentIndex() == 1;
    m_preview->setLayout(layout, m_content->currentText(), m_current != StreamVideo);
}

QRectF StreamSettingsWidget::placement() const
{
    const double x = m_x->value() / 100, y = m_y->value() / 100;
    return QRectF(x, y, qMin(m_width->value() / 100, 1.0 - x), qMin(m_height->value() / 100, 1.0 - y));
}

void StreamSettingsWidget::setPlacement(const QRectF &rect)
{
    const QSignalBlocker blockX(m_x), blockY(m_y), blockW(m_width), blockH(m_height);
    m_x->setValue(rect.x() * 100);
    m_y->setValue(rect.y() * 100);
    m_width->setMaximum(100 - m_x->value());
    m_height->setMaximum(100 - m_y->value());
    m_width->setValue(rect.width() * 100);
    m_height->setValue(rect.height() * 100);
    m_preset->setCurrentIndex(matchingPreset(placement()));
    updatePreview();
}
