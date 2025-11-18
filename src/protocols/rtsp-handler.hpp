#pragma once

#include "../common.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

namespace berrystreamcam {

class RtspHandler {
public:
    RtspHandler();
    ~RtspHandler();

    bool connect(const std::string& url);
    void disconnect();
    bool is_connected() const;

    bool receive_frame(VideoFrame& frame);

private:
    std::string url_;
    std::atomic<bool> connected_;

    AVFormatContext* format_ctx_;
    int video_stream_index_;
    std::mutex stream_mutex_;
};

} // namespace berrystreamcam
