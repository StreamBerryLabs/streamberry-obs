/*
 * BerryStreamCam - Common Types and Definitions
 *
 * Author: Swadhin Biswas (https://github.com/StreamBerryLabs/streamberry-obs)
 * License: GPL-2.0
 */

#pragma once

#include <obs-module.h>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

// Common definitions for BerryStreamCam plugin

namespace berrystreamcam {

// Protocol types supported
enum class ProtocolType {
    WEBSOCKET_OBS_DROID,  // Port 8080 - Custom WebSocket protocol
    HTTP_DASH,             // Port 8081 - MPEG-DASH
    HTTP_MJPEG,            // Port 8081 - MJPEG stream
    HTTP_RAW_H264,         // Port 8081 - Raw H.264
    RTSP,                  // Port 8554 - RTSP/RTP
    UNKNOWN
};

// Protocol information (must be declared before StreamDevice)
struct ProtocolInfo {
    ProtocolType type;
    std::string url;
    int port;
    bool is_available;
    int connected_clients;
};

// Device information
struct StreamDevice {
    std::string name;
    std::string ip_address;
    std::vector<ProtocolInfo> available_protocols;
    bool is_active;
    int64_t last_seen;
};

// Video frame data
struct VideoFrame {
    uint8_t* data;
    size_t size;
    int64_t timestamp;
    int64_t pts;
    int64_t dts;
    bool is_keyframe;
    int width;
    int height;
};

// Audio frame data
struct AudioFrame {
    uint8_t* data;
    size_t size;
    int64_t timestamp;
    int sample_rate;
    int channels;
};

// Stream configuration
struct StreamConfig {
    ProtocolType protocol;
    std::string device_ip;
    std::string stream_url;
    int video_width;
    int video_height;
    int video_fps;
    int audio_sample_rate;
    int audio_channels;
};

// Constants
constexpr int WEBSOCKET_PORT = 8080;
constexpr int HTTP_PORT = 8081;
constexpr int RTSP_PORT = 8554;
constexpr int DISCOVERY_INTERVAL_MS = 5000;
constexpr int CONNECTION_TIMEOUT_MS = 10000;
constexpr int FRAME_BUFFER_SIZE = 1920 * 1080 * 3; // Max frame size

// Protocol names
inline const char* protocol_to_string(ProtocolType type) {
    switch (type) {
        case ProtocolType::WEBSOCKET_OBS_DROID:
            return "WebSocket (OBS Droid)";
        case ProtocolType::HTTP_DASH:
            return "MPEG-DASH";
        case ProtocolType::HTTP_MJPEG:
            return "MJPEG";
        case ProtocolType::HTTP_RAW_H264:
            return "HTTP Raw H.264";
        case ProtocolType::RTSP:
            return "RTSP";
        default:
            return "Unknown";
    }
}

} // namespace berrystreamcam

// Logging helpers (must be outside namespace to use global blog() function)
#define BLOG(level, format, ...) \
    blog(level, "[BerryStreamCam] " format, ##__VA_ARGS__)

#define BLOG_INFO(format, ...) BLOG(LOG_INFO, format, ##__VA_ARGS__)
#define BLOG_WARNING(format, ...) BLOG(LOG_WARNING, format, ##__VA_ARGS__)
#define BLOG_ERROR(format, ...) BLOG(LOG_ERROR, format, ##__VA_ARGS__)
#define BLOG_DEBUG(format, ...) BLOG(LOG_DEBUG, format, ##__VA_ARGS__)
