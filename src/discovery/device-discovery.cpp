#include "device-discovery.hpp"
#include "mdns-scanner.hpp"
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <unistd.h>

namespace berrystreamcam {

DeviceDiscovery::DeviceDiscovery()
{
    BLOG_INFO("Device discovery initialized");
    curl_global_init(CURL_GLOBAL_ALL);
}

DeviceDiscovery::~DeviceDiscovery()
{
    curl_global_cleanup();
}

std::vector<StreamDevice> DeviceDiscovery::scan_network()
{
    std::vector<StreamDevice> devices;

    // Try mDNS discovery first (faster and more reliable)
    MdnsScanner mdns;
    auto mdns_devices = mdns.scan();

    for (const auto& ip : mdns_devices) {
        StreamDevice device;
        if (probe_device(ip, device)) {
            devices.push_back(device);
        }
    }

    // If no devices found via mDNS, try subnet scanning
    if (devices.empty()) {
        BLOG_DEBUG("No devices found via mDNS, trying subnet scan");
        auto subnet_ips = get_local_subnet_ips();

        for (const auto& ip : subnet_ips) {
            StreamDevice device;
            if (probe_device(ip, device)) {
                devices.push_back(device);
            }
        }
    }

    return devices;
}

bool DeviceDiscovery::probe_device(const std::string& ip_address, StreamDevice& device)
{
    device.ip_address = ip_address;
    device.name = "Streamberry-" + ip_address;
    device.is_active = false;
    device.last_seen = std::chrono::system_clock::now().time_since_epoch().count();
    device.available_protocols.clear();

    BLOG_DEBUG("Probing device at %s", ip_address.c_str());

    // Probe WebSocket port (8080)
    ProtocolInfo ws_proto;
    probe_websocket_port(ip_address, ws_proto);
    if (ws_proto.is_available) {
        device.available_protocols.push_back(ws_proto);
        device.is_active = true;
    }

    // Probe HTTP port (8081)
    std::vector<ProtocolInfo> http_protos;
    probe_http_port(ip_address, http_protos);
    if (!http_protos.empty()) {
        device.available_protocols.insert(device.available_protocols.end(),
                                         http_protos.begin(), http_protos.end());
        device.is_active = true;
    }

    // Probe RTSP port (8554)
    ProtocolInfo rtsp_proto;
    probe_rtsp_port(ip_address, rtsp_proto);
    if (rtsp_proto.is_available) {
        device.available_protocols.push_back(rtsp_proto);
        device.is_active = true;
    }

    if (device.is_active) {
        BLOG_INFO("✓ Found Streamberry device at %s with %zu protocol(s)",
                  ip_address.c_str(), device.available_protocols.size());
    }

    return device.is_active;
}

void DeviceDiscovery::probe_websocket_port(const std::string& ip, ProtocolInfo& proto)
{
    proto.type = ProtocolType::WEBSOCKET_OBS_DROID;
    proto.port = WEBSOCKET_PORT;
    proto.url = "ws://" + ip + ":8080/stream";
    proto.is_available = false;
    proto.connected_clients = 0;

    // Try to connect to WebSocket health endpoint
    CURL *curl = curl_easy_init();
    if (curl) {
        std::string health_url = "http://" + ip + ":8080/health";
        curl_easy_setopt(curl, CURLOPT_URL, health_url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200) {
                proto.is_available = true;
                BLOG_DEBUG("  ✓ WebSocket (OBS Droid) available at %s", proto.url.c_str());
            }
        }

        curl_easy_cleanup(curl);
    }
}

void DeviceDiscovery::probe_http_port(const std::string& ip, std::vector<ProtocolInfo>& protos)
{
    // Check if HTTP server is running
    CURL *curl = curl_easy_init();
    if (!curl) return;

    std::string base_url = "http://" + ip + ":8081/";
    curl_easy_setopt(curl, CURLOPT_URL, base_url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return;
    }

    // HTTP server is running, add all HTTP-based protocols

    // Raw H.264
    ProtocolInfo h264_proto;
    h264_proto.type = ProtocolType::HTTP_RAW_H264;
    h264_proto.port = HTTP_PORT;
    h264_proto.url = "http://" + ip + ":8081/stream.h264";
    h264_proto.is_available = true;
    h264_proto.connected_clients = 0;
    protos.push_back(h264_proto);
    BLOG_DEBUG("  ✓ HTTP Raw H.264 available at %s", h264_proto.url.c_str());

    // MJPEG
    ProtocolInfo mjpeg_proto;
    mjpeg_proto.type = ProtocolType::HTTP_MJPEG;
    mjpeg_proto.port = HTTP_PORT;
    mjpeg_proto.url = "http://" + ip + ":8081/mjpeg";
    mjpeg_proto.is_available = true;
    mjpeg_proto.connected_clients = 0;
    protos.push_back(mjpeg_proto);
    BLOG_DEBUG("  ✓ MJPEG available at %s", mjpeg_proto.url.c_str());

    // MPEG-DASH
    ProtocolInfo dash_proto;
    dash_proto.type = ProtocolType::HTTP_DASH;
    dash_proto.port = HTTP_PORT;
    dash_proto.url = "http://" + ip + ":8081/dash/manifest.mpd";
    dash_proto.is_available = true;
    dash_proto.connected_clients = 0;
    protos.push_back(dash_proto);
    BLOG_DEBUG("  ✓ MPEG-DASH available at %s", dash_proto.url.c_str());
}

void DeviceDiscovery::probe_rtsp_port(const std::string& ip, ProtocolInfo& proto)
{
    proto.type = ProtocolType::RTSP;
    proto.port = RTSP_PORT;
    proto.url = "rtsp://" + ip + ":8554/stream";
    proto.is_available = false;
    proto.connected_clients = 0;

    // Simple TCP connection test
    if (ping_host(ip, RTSP_PORT)) {
        proto.is_available = true;
        BLOG_DEBUG("  ✓ RTSP available at %s", proto.url.c_str());
    }
}

std::vector<std::string> DeviceDiscovery::get_local_subnet_ips()
{
    std::vector<std::string> ips;

    // Scan common local network IP ranges
    // This is a basic implementation that checks common private IP ranges
    BLOG_DEBUG("Scanning common local network IP ranges");

    // Get the system's hostname to determine likely subnet
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        BLOG_DEBUG("Hostname: %s", hostname);
    }

    // Try common router/gateway IPs to determine subnet
    // Reduced to most common subnets
    std::vector<std::string> common_subnets = {
        "192.168.1.",
        "192.168.0.",
        "10.0.0."
    };

    // For each subnet, check a range of IPs (e.g., .1 to .254)
    // But to avoid too many requests, only check likely device IPs
    // Reduced from 11 to 6 IPs per subnet for faster scanning
    std::vector<int> likely_ips = {1, 2, 10, 100, 101, 150};

    for (const auto& subnet : common_subnets) {
        for (int ip_suffix : likely_ips) {
            ips.push_back(subnet + std::to_string(ip_suffix));
        }
    }

    BLOG_DEBUG("Generated %zu IPs to scan", ips.size());

    return ips;
}

bool DeviceDiscovery::ping_host(const std::string& ip, int port)
{
    // Simple TCP connection test
    // In a real implementation, you'd use socket APIs to test connectivity

    CURL *curl = curl_easy_init();
    if (!curl) return false;

    std::string url = "http://" + ip + ":" + std::to_string(port);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

} // namespace berrystreamcam
