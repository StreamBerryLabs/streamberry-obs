#pragma once

#include <obs-module.h>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include "common.hpp"
#include "discovery/device-discovery.hpp"
#include "protocols/websocket-handler.hpp"
#include "protocols/rtsp-handler.hpp"
#include "protocols/http-handler.hpp"
#include "decoder/h264-decoder.hpp"

namespace berrystreamcam {

class BerryStreamCamSource {
public:
    BerryStreamCamSource(obs_data_t *settings, obs_source_t *source);
    ~BerryStreamCamSource();

    // OBS callbacks
    void update(obs_data_t *settings);
    void activate();
    void deactivate();
    void show();
    void hide();
    uint32_t get_width();
    uint32_t get_height();
    void render(gs_effect_t *effect);

    // Properties
    static obs_properties_t* get_properties(void *data);
    static void get_defaults(obs_data_t *settings);

private:
    void start_streaming();
    void stop_streaming();
    void stop_streaming_safe();
    void pause_streaming();
    void resume_streaming();
    void restart_streaming();
    void fallback_to_websocket();
    void update_devices();
    void process_video_frame(const VideoFrame& frame);
    void process_audio_frame(const AudioFrame& frame);

    void discovery_thread_func();
    void streaming_thread_func();
    void streaming_thread_impl();

    obs_source_t *source_;
    gs_texture_t *texture_;

    std::unique_ptr<DeviceDiscovery> discovery_;
    std::unique_ptr<WebSocketHandler> ws_handler_;
    std::unique_ptr<RtspHandler> rtsp_handler_;
    std::unique_ptr<HttpHandler> http_handler_;
    std::unique_ptr<H264Decoder> decoder_;

    StreamConfig config_;
    std::vector<StreamDevice> discovered_devices_;

    enum class StreamState {
        STOPPED,
        PAUSED,
        STREAMING
    };

    std::atomic<bool> active_;
    std::atomic<bool> streaming_;
    std::atomic<StreamState> stream_state_;
    std::atomic<uint32_t> width_;
    std::atomic<uint32_t> height_;

    std::thread discovery_thread_;
    std::thread streaming_thread_;
    std::mutex device_mutex_;
    std::mutex frame_mutex_;
    std::mutex streaming_mutex_;

    uint8_t *frame_buffer_;
    size_t frame_buffer_size_;

    ProtocolType last_protocol_;
    ProtocolType fallback_protocol_;
    std::atomic<bool> connection_failed_;
    std::atomic<int> connection_retry_count_;
};

void register_berrystreamcam_source();

} // namespace berrystreamcam
