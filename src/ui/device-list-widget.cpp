#include "device-list-widget.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

namespace berrystreamcam {

DeviceListWidget::DeviceListWidget(QWidget* parent)
    : QWidget(parent)
    , selected_device_index_(-1)
{
    setup_ui();

    // Auto-refresh every 5 seconds
    auto_refresh_timer_ = new QTimer(this);
    connect(auto_refresh_timer_, &QTimer::timeout, this, &DeviceListWidget::on_refresh_clicked);
    auto_refresh_timer_->start(5000);
}

DeviceListWidget::~DeviceListWidget()
{
}

void DeviceListWidget::setup_ui()
{
    QVBoxLayout* main_layout = new QVBoxLayout(this);

    // Title
    QLabel* title = new QLabel("Streamberry Devices", this);
    QFont title_font = title->font();
    title_font.setPointSize(12);
    title_font.setBold(true);
    title->setFont(title_font);
    main_layout->addWidget(title);

    // Device list
    QGroupBox* device_group = new QGroupBox("Available Devices", this);
    QVBoxLayout* device_layout = new QVBoxLayout(device_group);

    device_list_ = new QListWidget(this);
    device_list_->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(device_list_, &QListWidget::itemSelectionChanged,
            this, &DeviceListWidget::on_device_selection_changed);
    device_layout->addWidget(device_list_);

    main_layout->addWidget(device_group);

    // Protocol selection
    QGroupBox* protocol_group = new QGroupBox("Protocol", this);
    QVBoxLayout* protocol_layout = new QVBoxLayout(protocol_group);

    protocol_combo_ = new QComboBox(this);
    protocol_combo_->addItem("WebSocket (OBS Droid) - Recommended",
                            static_cast<int>(ProtocolType::WEBSOCKET_OBS_DROID));
    protocol_combo_->addItem("RTSP",
                            static_cast<int>(ProtocolType::RTSP));
    protocol_combo_->addItem("HTTP Raw H.264",
                            static_cast<int>(ProtocolType::HTTP_RAW_H264));
    protocol_combo_->addItem("MJPEG",
                            static_cast<int>(ProtocolType::HTTP_MJPEG));

    connect(protocol_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeviceListWidget::on_protocol_selection_changed);
    protocol_layout->addWidget(protocol_combo_);

    main_layout->addWidget(protocol_group);

    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();

    refresh_button_ = new QPushButton("Refresh", this);
    connect(refresh_button_, &QPushButton::clicked,
            this, &DeviceListWidget::on_refresh_clicked);
    button_layout->addWidget(refresh_button_);

    connect_button_ = new QPushButton("Connect", this);
    connect_button_->setEnabled(false);
    connect(connect_button_, &QPushButton::clicked,
            this, &DeviceListWidget::on_connect_clicked);
    button_layout->addWidget(connect_button_);

    main_layout->addLayout(button_layout);

    // Status label
    status_label_ = new QLabel("No devices found", this);
    status_label_->setWordWrap(true);
    main_layout->addWidget(status_label_);

    main_layout->addStretch();

    setLayout(main_layout);
}

void DeviceListWidget::update_device_list(const std::vector<StreamDevice>& devices)
{
    current_devices_ = devices;

    device_list_->clear();

    if (devices.empty()) {
        status_label_->setText("No devices found. Make sure your Streamberry app is running.");
        connect_button_->setEnabled(false);
        return;
    }

    for (const auto& device : devices) {
        QString item_text = QString("%1 (%2)")
            .arg(QString::fromStdString(device.name))
            .arg(QString::fromStdString(device.ip_address));

        if (!device.available_protocols.empty()) {
            item_text += QString(" - %1 protocol(s)")
                .arg(device.available_protocols.size());
        }

        device_list_->addItem(item_text);
    }

    status_label_->setText(QString("Found %1 device(s)").arg(devices.size()));
}

void DeviceListWidget::on_refresh_clicked()
{
    status_label_->setText("Scanning for devices...");
    emit refresh_requested();
}

void DeviceListWidget::on_connect_clicked()
{
    if (selected_device_index_ < 0 ||
        selected_device_index_ >= static_cast<int>(current_devices_.size())) {
        return;
    }

    const auto& device = current_devices_[selected_device_index_];
    ProtocolType protocol = static_cast<ProtocolType>(
        protocol_combo_->currentData().toInt()
    );

    // Check if selected protocol is available for this device
    bool protocol_available = false;
    for (const auto& proto : device.available_protocols) {
        if (proto.type == protocol) {
            protocol_available = true;
            break;
        }
    }

    if (!protocol_available) {
        status_label_->setText("Selected protocol not available on this device");
        return;
    }

    status_label_->setText(QString("Connecting to %1 via %2...")
        .arg(QString::fromStdString(device.ip_address))
        .arg(protocol_to_string(protocol)));

    emit device_selected(device.ip_address, protocol);
}

void DeviceListWidget::on_device_selection_changed()
{
    auto selected_items = device_list_->selectedItems();

    if (selected_items.isEmpty()) {
        selected_device_index_ = -1;
        connect_button_->setEnabled(false);
        protocol_combo_->setEnabled(false);
        return;
    }

    selected_device_index_ = device_list_->row(selected_items.first());

    if (selected_device_index_ >= 0 &&
        selected_device_index_ < static_cast<int>(current_devices_.size())) {

        const auto& device = current_devices_[selected_device_index_];

        // Update protocol combo with available protocols
        protocol_combo_->clear();
        for (const auto& proto : device.available_protocols) {
            protocol_combo_->addItem(
                protocol_to_string(proto.type),
                static_cast<int>(proto.type)
            );
        }

        protocol_combo_->setEnabled(true);
        connect_button_->setEnabled(true);

        status_label_->setText(QString("Selected: %1 (%2 protocol(s) available)")
            .arg(QString::fromStdString(device.name))
            .arg(device.available_protocols.size()));
    }
}

void DeviceListWidget::on_protocol_selection_changed(int index)
{
    if (index < 0) return;

    ProtocolType protocol = static_cast<ProtocolType>(
        protocol_combo_->itemData(index).toInt()
    );

    status_label_->setText(QString("Protocol: %1").arg(protocol_to_string(protocol)));
}

} // namespace berrystreamcam
