#pragma once

#include "../common.hpp"
#include <vector>
#include <string>

namespace berrystreamcam {

class DeviceDiscovery {
public:
    DeviceDiscovery();
    ~DeviceDiscovery();

    // Scan network for Streamberry devices
    std::vector<StreamDevice> scan_network();

    // Check if specific device is available
    bool probe_device(const std::string& ip_address, StreamDevice& device);

private:
    void probe_websocket_port(const std::string& ip, ProtocolInfo& proto);
    void probe_http_port(const std::string& ip, std::vector<ProtocolInfo>& protos);
    void probe_rtsp_port(const std::string& ip, ProtocolInfo& proto);

    std::vector<std::string> get_local_subnet_ips();
    bool ping_host(const std::string& ip, int port);
};

} // namespace berrystreamcam
