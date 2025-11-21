/*
 * BerryStreamCam - Source Implementation
 * Author: Swadhin Biswas (https://github.com/StreamBerryLabs/streamberry-obs)
 * License: GPL-2.0
 */

#include "berrystreamcam-source.hpp"
#include <util/platform.h>
#include <graphics/image-file.h>

namespace berrystreamcam {

// OBS source info structure
static const char *berrystreamcam_get_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    return "BerryStreamCam";
}

static void *berrystreamcam_create(obs_data_t *settings, obs_source_t *source)
{
    return new BerryStreamCamSource(settings, source);
}

static void berrystreamcam_destroy(void *data)
{
    delete static_cast<BerryStreamCamSource*>(data);
}

static void berrystreamcam_update(void *data, obs_data_t *settings)
{
    static_cast<BerryStreamCamSource*>(data)->update(settings);
}

static void berrystreamcam_activate(void *data)
{
    static_cast<BerryStreamCamSource*>(data)->activate();
}

static void berrystreamcam_deactivate(void *data)
{
    static_cast<BerryStreamCamSource*>(data)->deactivate();
}

static void berrystreamcam_show(void *data)
{
    static_cast<BerryStreamCamSource*>(data)->show();
}

static void berrystreamcam_hide(void *data)
{
    static_cast<BerryStreamCamSource*>(data)->hide();
}

static uint32_t berrystreamcam_get_width(void *data)
{
    return static_cast<BerryStreamCamSource*>(data)->get_width();
}

static uint32_t berrystreamcam_get_height(void *data)
{
    return static_cast<BerryStreamCamSource*>(data)->get_height();
}

static void berrystreamcam_render(void *data, gs_effect_t *effect)
{
    static_cast<BerryStreamCamSource*>(data)->render(effect);
}

static obs_properties_t *berrystreamcam_get_properties(void *data)
{
    return BerryStreamCamSource::get_properties(data);
}

static void berrystreamcam_get_defaults(obs_data_t *settings)
{
    BerryStreamCamSource::get_defaults(settings);
}

// Constructor
BerryStreamCamSource::BerryStreamCamSource(obs_data_t *settings, obs_source_t *source)
    : source_(source)
    , texture_(nullptr)
    , active_(false)
    , streaming_(false)
    , stream_state_(StreamState::STOPPED)
    , width_(1920)
    , height_(1080)
    , frame_buffer_(nullptr)
    , frame_buffer_size_(0)
    , last_protocol_(ProtocolType::HTTP_RAW_H264)
    , fallback_protocol_(ProtocolType::WEBSOCKET_OBS_DROID)
    , connection_failed_(false)
    , connection_retry_count_(0)
{
    BLOG_INFO("Creating BerryStreamCam source");

    // Allocate frame buffer
    frame_buffer_size_ = FRAME_BUFFER_SIZE;
    frame_buffer_ = new uint8_t[frame_buffer_size_];

    // Initialize discovery
    discovery_ = std::make_unique<DeviceDiscovery>();

    // Initialize decoder
    decoder_ = std::make_unique<H264Decoder>();
    decoder_->initialize();

    // Initialize handlers in main thread (Qt requirement)
    ws_handler_ = std::make_unique<WebSocketHandler>();
    http_handler_ = std::make_unique<HttpHandler>();
    // rtsp_handler_ = std::make_unique<RtspHandler>();  // Coming soon

    // Set active flag so discovery thread runs
    active_ = true;

    // Start discovery thread
    discovery_thread_ = std::thread(&BerryStreamCamSource::discovery_thread_func, this);

    update(settings);
}

// Destructor
BerryStreamCamSource::~BerryStreamCamSource()
{
    BLOG_INFO("Destroying BerryStreamCam source");

    // Stop streaming first
    stop_streaming();

    // Stop discovery thread
    active_ = false;
    if (discovery_thread_.joinable()) {
        try {
            discovery_thread_.join();
        } catch (const std::exception& e) {
            BLOG_WARNING("Exception while joining discovery thread: %s", e.what());
        }
    }

    // Clean up handlers explicitly to prevent crashes
    try {
        ws_handler_.reset();
    } catch (...) {
        BLOG_WARNING("Exception cleaning up WebSocket handler");
    }

    try {
        http_handler_.reset();
    } catch (...) {
        BLOG_WARNING("Exception cleaning up HTTP handler");
    }

    try {
        rtsp_handler_.reset();
    } catch (...) {
        BLOG_WARNING("Exception cleaning up RTSP handler");
    }

    // Clean up decoder
    if (decoder_) {
        try {
            decoder_->shutdown();
        } catch (...) {
            BLOG_WARNING("Exception shutting down decoder");
        }
    }

    // Clean up texture
    try {
        obs_enter_graphics();
        if (texture_) {
            gs_texture_destroy(texture_);
            texture_ = nullptr;
        }
        obs_leave_graphics();
    } catch (const std::exception& e) {
        BLOG_WARNING("Exception while cleaning up texture: %s", e.what());
    }

    // Clean up buffer
    delete[] frame_buffer_;
}

void BerryStreamCamSource::update(obs_data_t *settings)
{
    BLOG_DEBUG("Updating settings");

    const char *device_ip = obs_data_get_string(settings, "device_ip");
    const char *manual_ip = obs_data_get_string(settings, "manual_ip");
    const char *protocol = obs_data_get_string(settings, "protocol");

    // Use manual IP if device_ip is empty or is the placeholder
    std::string ip_to_use;
    if (manual_ip && strlen(manual_ip) > 0) {
        ip_to_use = manual_ip;
    } else if (device_ip && strlen(device_ip) > 0) {
        ip_to_use = device_ip;
    }

    if (!ip_to_use.empty()) {
        config_.device_ip = ip_to_use;

        // Store old protocol to detect changes
        ProtocolType old_protocol = config_.protocol;

        // Determine protocol type
        if (strcmp(protocol, "websocket") == 0) {
            config_.protocol = ProtocolType::WEBSOCKET_OBS_DROID;
            config_.stream_url = "ws://" + config_.device_ip + ":8080/stream";
        } /* else if (strcmp(protocol, "rtsp") == 0) {
            config_.protocol = ProtocolType::RTSP;
            config_.stream_url = "rtsp://" + config_.device_ip + ":8554/stream";
        } */ else if (strcmp(protocol, "http_h264") == 0) {
            config_.protocol = ProtocolType::HTTP_RAW_H264;
            config_.stream_url = "http://" + config_.device_ip + ":8081/stream.h264";
        } else if (strcmp(protocol, "mjpeg") == 0) {
            config_.protocol = ProtocolType::HTTP_MJPEG;
            config_.stream_url = "http://" + config_.device_ip + ":8081/mjpeg";
        }

        BLOG_INFO("Updated config: %s via %s",
                  config_.device_ip.c_str(),
                  protocol_to_string(config_.protocol));

        // If protocol changed, restart stream (can't use pause/resume for protocol changes)
        if (old_protocol != config_.protocol && last_protocol_ != config_.protocol) {
            BLOG_INFO("Protocol changed from %s to %s, restarting stream",
                     protocol_to_string(last_protocol_),
                     protocol_to_string(config_.protocol));

            // Clear texture to prevent frozen frame
            try {
                obs_enter_graphics();
                if (texture_) {
                    gs_texture_destroy(texture_);
                    texture_ = nullptr;
                }
                obs_leave_graphics();
            } catch (const std::exception& e) {
                BLOG_WARNING("Exception while clearing texture: %s", e.what());
            }

            if (decoder_) {
                decoder_->flush();
            }

            last_protocol_ = config_.protocol;
            connection_failed_ = false;
            connection_retry_count_ = 0;

            // Full restart for protocol changes
            restart_streaming();
        }
    }
}

void BerryStreamCamSource::activate()
{
    BLOG_INFO("Source activated");
    active_ = true;
    start_streaming();
}

void BerryStreamCamSource::deactivate()
{
    BLOG_INFO("Source deactivated");
    active_ = false;
    stop_streaming();
}

void BerryStreamCamSource::show()
{
    BLOG_INFO("Source shown - resuming stream");
    resume_streaming();
}

void BerryStreamCamSource::hide()
{
    BLOG_INFO("Source hidden - pausing stream");
    pause_streaming();
}

uint32_t BerryStreamCamSource::get_width()
{
    return width_;
}

uint32_t BerryStreamCamSource::get_height()
{
    return height_;
}

void BerryStreamCamSource::render(gs_effect_t *effect)
{
    if (!texture_) {
        return;
    }

    gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), texture_);
    gs_draw_sprite(texture_, 0, 0, 0);
}

obs_properties_t* BerryStreamCamSource::get_properties(void *data)
{
    obs_properties_t *props = obs_properties_create();

    // Device selection
    obs_property_t *device_list = obs_properties_add_list(
        props, "device_ip", "Device",
        OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    // Add manual entry option
    obs_property_list_add_string(device_list, "Enter IP manually...", "");

    // Add discovered devices
    if (data) {
        BerryStreamCamSource *source = static_cast<BerryStreamCamSource*>(data);
        std::lock_guard<std::mutex> lock(source->device_mutex_);
        for (const auto& device : source->discovered_devices_) {
            std::string label = device.name + " (" + device.ip_address + ")";
            obs_property_list_add_string(device_list, label.c_str(), device.ip_address.c_str());
        }
    }

    // Manual IP entry field with help text
    obs_property_t *manual_ip_prop = obs_properties_add_text(props, "manual_ip",
        "Or Enter IP Manually (e.g., 192.168.1.100)", OBS_TEXT_DEFAULT);
    obs_property_set_long_description(manual_ip_prop,
        "Enter the IP address of your Streamberry device. "
        "Leave empty to use the device selected above. "
        "Default ports: WebSocket=8080, HTTP=8081, RTSP=8554");

    // Protocol selection
    obs_property_t *protocol_list = obs_properties_add_list(
        props, "protocol", "Protocol",
        OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    obs_property_list_add_string(protocol_list, "WebSocket (Port 8080) - Recommended", "websocket");
    // obs_property_list_add_string(protocol_list, "RTSP (Port 8554) - Coming Soon", "rtsp");
    obs_property_list_add_string(protocol_list, "HTTP Raw H.264 (Port 8081)", "http_h264");
    obs_property_list_add_string(protocol_list, "MJPEG (Port 8081)", "mjpeg");

    // Refresh button
    obs_properties_add_button(props, "refresh_devices", "Refresh Devices",
        [](obs_properties_t *props, obs_property_t *property, void *data) -> bool {
            BLOG_INFO("Manual device refresh requested");
            return true;
        });

    // Status text
    obs_properties_add_text(props, "status", "Status", OBS_TEXT_INFO);

    return props;
}

void BerryStreamCamSource::get_defaults(obs_data_t *settings)
{
    obs_data_set_default_string(settings, "protocol", "websocket");
    obs_data_set_default_string(settings, "device_ip", "");
}

void BerryStreamCamSource::start_streaming()
{
    std::lock_guard<std::mutex> lock(streaming_mutex_);

    if (streaming_ || config_.device_ip.empty()) {
        return;
    }

    // Ensure previous thread is fully stopped
    if (streaming_thread_.joinable()) {
        try {
            streaming_thread_.join();
        } catch (...) {
            BLOG_WARNING("Exception joining previous streaming thread");
        }
    }

    BLOG_INFO("Starting stream from %s", config_.stream_url.c_str());
    streaming_ = true;
    stream_state_ = StreamState::STREAMING;

    // Start streaming thread
    try {
        streaming_thread_ = std::thread(&BerryStreamCamSource::streaming_thread_func, this);
    } catch (const std::exception& e) {
        BLOG_ERROR("Failed to create streaming thread: %s", e.what());
        streaming_ = false;
        stream_state_ = StreamState::STOPPED;
    }
}

void BerryStreamCamSource::pause_streaming()
{
    if (stream_state_ == StreamState::STREAMING) {
        BLOG_INFO("Stream state: \"paused\"");
        stream_state_ = StreamState::PAUSED;
        // Wait a bit for streaming thread to see paused state
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void BerryStreamCamSource::resume_streaming()
{
    if (stream_state_ == StreamState::PAUSED) {
        BLOG_INFO("Stream state: \"streaming\"");
        stream_state_ = StreamState::STREAMING;
    }
}

void BerryStreamCamSource::stop_streaming_safe()
{
    std::lock_guard<std::mutex> lock(streaming_mutex_);

    if (!streaming_) {
        return;
    }

    BLOG_INFO("Safely stopping stream");
    stream_state_ = StreamState::STOPPED;

    // Signal thread to stop
    streaming_ = false;

    // Close and reset handlers first to stop data flow and prevent use-after-free
    try {
        if (ws_handler_) {
            ws_handler_->disconnect_from_server();
            // DON'T reset here - will be reused or destroyed in destructor
        }
    } catch (const std::exception& e) {
        BLOG_WARNING("Exception while disconnecting WebSocket: %s", e.what());
    } catch (...) {
        BLOG_WARNING("Unknown exception while disconnecting WebSocket");
    }

    try {
        if (rtsp_handler_) {
            rtsp_handler_->disconnect();
            // DON'T reset here - will be destroyed in destructor
        }
    } catch (const std::exception& e) {
        BLOG_WARNING("Exception while disconnecting RTSP: %s", e.what());
    } catch (...) {
        BLOG_WARNING("Unknown exception while disconnecting RTSP");
    }

    try {
        if (http_handler_) {
            http_handler_->disconnect();
            // DON'T reset here - will be destroyed in destructor
        }
    } catch (const std::exception& e) {
        BLOG_WARNING("Exception while disconnecting HTTP: %s", e.what());
    } catch (...) {
        BLOG_WARNING("Unknown exception while disconnecting HTTP");
    }

    // Wait for streaming thread with timeout
    if (streaming_thread_.joinable()) {
        try {
            // Give thread 3 seconds to finish
            auto future = std::async(std::launch::async, [this]() {
                if (streaming_thread_.joinable()) {
                    streaming_thread_.join();
                }
            });

            if (future.wait_for(std::chrono::seconds(3)) == std::future_status::timeout) {
                BLOG_WARNING("Streaming thread did not stop gracefully, detaching");
                streaming_thread_.detach();
            }
        } catch (const std::exception& e) {
            BLOG_WARNING("Exception while joining streaming thread: %s", e.what());
            if (streaming_thread_.joinable()) {
                streaming_thread_.detach();
            }
        } catch (...) {
            BLOG_WARNING("Unknown exception while joining streaming thread");
            if (streaming_thread_.joinable()) {
                streaming_thread_.detach();
            }
        }
    }

    // Note: Texture and decoder are NOT cleared here anymore
    // They are only cleared when protocol changes in update()
    // This allows hide/show to work without destroying the texture

    BLOG_INFO("Stream stopped");
}

void BerryStreamCamSource::stop_streaming()
{
    stop_streaming_safe();
}

void BerryStreamCamSource::restart_streaming()
{
    if (!active_) {
        BLOG_WARNING("Source not active, skipping restart");
        return;
    }

    BLOG_INFO("Restarting stream");

    // Stop first (this locks the mutex)
    stop_streaming_safe();

    // Wait to ensure complete shutdown
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Start again (this also locks the mutex)
    if (active_) {
        start_streaming();
    }
}

void BerryStreamCamSource::fallback_to_websocket()
{
    if (config_.protocol == fallback_protocol_) {
        BLOG_WARNING("Already using fallback protocol, cannot fallback further");
        return;
    }

    BLOG_WARNING("Connection failed, falling back to WebSocket protocol");

    connection_failed_ = true;

    // Update to WebSocket protocol
    config_.protocol = fallback_protocol_;
    config_.stream_url = "ws://" + config_.device_ip + ":8080/stream";

    // Clear texture
    try {
        obs_enter_graphics();
        if (texture_) {
            gs_texture_destroy(texture_);
            texture_ = nullptr;
        }
        obs_leave_graphics();
    } catch (const std::exception& e) {
        BLOG_WARNING("Exception while clearing texture: %s", e.what());
    }

    if (decoder_) {
        decoder_->flush();
    }

    last_protocol_ = fallback_protocol_;

    // Restart with WebSocket
    restart_streaming();
}

void BerryStreamCamSource::discovery_thread_func()
{
    BLOG_INFO("Discovery thread started");

    int scan_count = 0;
    while (active_) {
        // Only scan every 3rd iteration (15 seconds) to reduce network/CPU load
        if (scan_count % 3 == 0) {
            auto devices = discovery_->scan_network();

            {
                std::lock_guard<std::mutex> lock(device_mutex_);
                discovered_devices_ = devices;
            }

            if (!devices.empty()) {
                BLOG_DEBUG("Found %zu Streamberry device(s)", devices.size());
            }
        }

        scan_count++;

        // Wait before next scan
        std::this_thread::sleep_for(std::chrono::milliseconds(DISCOVERY_INTERVAL_MS));
    }

    BLOG_INFO("Discovery thread stopped");
}

void BerryStreamCamSource::streaming_thread_func()
{
    BLOG_INFO("Streaming thread started");

    try {
        streaming_thread_impl();
    } catch (const std::exception& e) {
        BLOG_ERROR("Exception in streaming thread: %s", e.what());
        streaming_ = false;
    } catch (...) {
        BLOG_ERROR("Unknown exception in streaming thread");
        streaming_ = false;
    }

    BLOG_INFO("Streaming thread stopped");
}

void BerryStreamCamSource::streaming_thread_impl()
{
    bool connection_success = false;
    ProtocolType attempted_protocol = config_.protocol;

    // Connect using appropriate handler based on protocol
    // Handlers are already created in main thread (constructor)
    try {
        switch (config_.protocol) {
            case ProtocolType::WEBSOCKET_OBS_DROID:
                if (ws_handler_) {
                    if (!ws_handler_->connect_to_server(config_.stream_url)) {
                        BLOG_ERROR("Failed to connect via WebSocket");
                    } else {
                        connection_success = true;
                    }
                } else {
                    BLOG_ERROR("WebSocket handler not initialized");
                }
                break;

            /* RTSP - Coming Soon
            case ProtocolType::RTSP:
                if (rtsp_handler_) {
                    if (!rtsp_handler_->connect(config_.stream_url)) {
                        BLOG_ERROR("Failed to connect via RTSP");
                    } else {
                        connection_success = true;
                    }
                } else {
                    BLOG_ERROR("RTSP handler not initialized");
                }
                break;
            */

            case ProtocolType::HTTP_RAW_H264:
            case ProtocolType::HTTP_MJPEG:
                if (http_handler_) {
                    if (!http_handler_->connect(config_.stream_url, config_.protocol)) {
                        BLOG_ERROR("Failed to connect via HTTP");
                    } else {
                        connection_success = true;
                    }
                } else {
                    BLOG_ERROR("HTTP handler not initialized");
                }
                break;

            default:
                BLOG_ERROR("Unsupported protocol");
                break;
        }
    } catch (const std::exception& e) {
        BLOG_ERROR("Exception during connection: %s", e.what());
        connection_success = false;
    }

    // If connection failed and not already using WebSocket, fallback
    if (!connection_success) {
        streaming_ = false;
        connection_retry_count_++;

        if (attempted_protocol != fallback_protocol_ && connection_retry_count_ <= 3) {
            BLOG_WARNING("Connection failed (attempt %d/3), will fallback to WebSocket",
                        connection_retry_count_.load());
            // Schedule fallback (do it from main thread to avoid issues)
            // For now, just stop - user can manually switch or we trigger on next attempt
        }
        return;
    }

    BLOG_INFO("Connected to stream");
    connection_retry_count_ = 0;
    connection_failed_ = false;

    // Main streaming loop
    StreamState last_state = StreamState::STREAMING;

    while (streaming_) {
        StreamState current_state = stream_state_.load();

        // Handle state transitions
        if (current_state == StreamState::PAUSED && last_state == StreamState::STREAMING) {
            // Transitioning to PAUSED - disconnect handlers from streaming thread
            BLOG_INFO("Disconnecting handlers (paused)");
            if (ws_handler_ && ws_handler_->is_connected()) {
                ws_handler_->disconnect_from_server();
            }
            if (http_handler_ && http_handler_->is_connected()) {
                http_handler_->disconnect();
            }
            last_state = StreamState::PAUSED;
        }
        else if (current_state == StreamState::STREAMING && last_state == StreamState::PAUSED) {
            // Transitioning back to STREAMING - reconnect with current protocol
            BLOG_INFO("Reconnecting handlers (resumed)");

            std::string url = config_.device_ip.empty() ? "192.168.110.10" : config_.device_ip;

            if (config_.protocol == ProtocolType::WEBSOCKET_OBS_DROID) {
                if (ws_handler_ && !ws_handler_->is_connected()) {
                    std::string ws_url = "ws://" + url + ":8080/stream";
                    if (!ws_handler_->connect_to_server(ws_url)) {
                        BLOG_ERROR("Failed to reconnect WebSocket");
                    }
                } else if (!ws_handler_) {
                    BLOG_ERROR("WebSocket handler not initialized");
                }
            } else if (config_.protocol == ProtocolType::HTTP_RAW_H264) {
                if (http_handler_ && !http_handler_->is_connected()) {
                    std::string http_url = "http://" + url + ":8081/stream.h264";
                    if (!http_handler_->connect(http_url, ProtocolType::HTTP_RAW_H264)) {
                        BLOG_ERROR("Failed to reconnect HTTP");
                    }
                } else if (!http_handler_) {
                    BLOG_ERROR("HTTP handler not initialized");
                }
            } else if (config_.protocol == ProtocolType::HTTP_MJPEG) {
                if (http_handler_ && !http_handler_->is_connected()) {
                    std::string mjpeg_url = "http://" + url + ":8081/mjpeg";
                    if (!http_handler_->connect(mjpeg_url, ProtocolType::HTTP_MJPEG)) {
                        BLOG_ERROR("Failed to reconnect MJPEG");
                    }
                } else if (!http_handler_) {
                    BLOG_ERROR("HTTP handler not initialized");
                }
            }
            last_state = StreamState::STREAMING;
        }

        // If paused, sleep and continue
        if (current_state == StreamState::PAUSED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        VideoFrame frame = {};

        // Receive frame from active handler based on current protocol
        // Note: No need to call process_events() - WebSocket now uses dedicated thread
        bool got_frame = false;
        if (config_.protocol == ProtocolType::WEBSOCKET_OBS_DROID && ws_handler_) {
            got_frame = ws_handler_->receive_frame(frame);
        } /* else if (config_.protocol == ProtocolType::RTSP && rtsp_handler_) {
            got_frame = rtsp_handler_->receive_frame(frame);
        } */ else if ((config_.protocol == ProtocolType::HTTP_RAW_H264 ||
                      config_.protocol == ProtocolType::HTTP_MJPEG) && http_handler_) {
            got_frame = http_handler_->receive_frame(frame);
        }

        if (got_frame) {
            process_video_frame(frame);
            // Free frame data after processing to prevent memory leak
            delete[] frame.data;
            frame.data = nullptr;
        } else {
            // Brief sleep to avoid busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void BerryStreamCamSource::process_video_frame(const VideoFrame& frame)
{
    // Decode H.264 frame
    uint8_t *decoded_data = nullptr;
    int decoded_width = 0, decoded_height = 0;

    if (!decoder_->decode_frame(frame.data, frame.size,
                                 &decoded_data, &decoded_width, &decoded_height)) {
        return;
    }

    // Update dimensions
    width_ = decoded_width;
    height_ = decoded_height;

    // Upload to GPU texture
    obs_enter_graphics();

    if (!texture_ || gs_texture_get_width(texture_) != width_ ||
        gs_texture_get_height(texture_) != height_) {

        if (texture_) {
            gs_texture_destroy(texture_);
        }

        texture_ = gs_texture_create(width_, height_, GS_RGBA, 1, nullptr, GS_DYNAMIC);
    }

    if (texture_) {
        gs_texture_set_image(texture_, decoded_data, width_ * 4, false);
    }

    obs_leave_graphics();
}

void BerryStreamCamSource::process_audio_frame(const AudioFrame& frame)
{
    // Audio handling would be implemented here
    // For now, focusing on video
}

void register_berrystreamcam_source()
{
    obs_source_info info = {};
    info.id = "berrystreamcam_source";
    info.type = OBS_SOURCE_TYPE_INPUT;
    info.output_flags = OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_DO_NOT_DUPLICATE;

    info.get_name = berrystreamcam_get_name;
    info.create = berrystreamcam_create;
    info.destroy = berrystreamcam_destroy;
    info.update = berrystreamcam_update;
    info.activate = berrystreamcam_activate;
    info.deactivate = berrystreamcam_deactivate;
    info.show = berrystreamcam_show;
    info.hide = berrystreamcam_hide;
    info.get_width = berrystreamcam_get_width;
    info.get_height = berrystreamcam_get_height;
    info.video_render = berrystreamcam_render;
    info.get_properties = berrystreamcam_get_properties;
    info.get_defaults = berrystreamcam_get_defaults;

    obs_register_source(&info);

    BLOG_INFO("BerryStreamCam source registered");
}

} // namespace berrystreamcam
