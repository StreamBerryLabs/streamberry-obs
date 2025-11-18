#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include "../common.hpp"
#include "../discovery/device-discovery.hpp"

namespace berrystreamcam {

class DeviceListWidget : public QWidget {
    Q_OBJECT

public:
    explicit DeviceListWidget(QWidget* parent = nullptr);
    ~DeviceListWidget();

    void update_device_list(const std::vector<StreamDevice>& devices);

signals:
    void device_selected(const std::string& ip, ProtocolType protocol);
    void refresh_requested();

private slots:
    void on_refresh_clicked();
    void on_connect_clicked();
    void on_device_selection_changed();
    void on_protocol_selection_changed(int index);

private:
    void setup_ui();

    QListWidget* device_list_;
    QComboBox* protocol_combo_;
    QPushButton* refresh_button_;
    QPushButton* connect_button_;
    QLabel* status_label_;
    QTimer* auto_refresh_timer_;

    std::vector<StreamDevice> current_devices_;
    int selected_device_index_;
};

} // namespace berrystreamcam
