#pragma once

#include "../common.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include <thread>
#include <functional>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace berrystreamcam {

class HttpHandler {
public:
    HttpHandler();
    ~HttpHandler();

    bool connect(const std::string& url, ProtocolType type);
    void disconnect();
    bool is_connected() const;

    bool receive_frame(VideoFrame& frame);

private:
    void stream_loop();

    std::string url_;
    ProtocolType protocol_type_;

    std::atomic<bool> connected_;
    std::atomic<bool> running_;
    std::queue<VideoFrame> frame_queue_;
    std::mutex queue_mutex_;

    std::thread streaming_thread_;

    // FFmpeg components
    AVFormatContext* format_context_;
    int video_stream_index_;
};

} // namespace berrystreamcam
