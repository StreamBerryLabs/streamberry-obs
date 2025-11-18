#include "rtsp-handler.hpp"
#include <cstring>

namespace berrystreamcam {

RtspHandler::RtspHandler()
    : connected_(false)
    , format_ctx_(nullptr)
    , video_stream_index_(-1)
{
    BLOG_DEBUG("RTSP handler created (FFmpeg-based)");
}

RtspHandler::~RtspHandler()
{
    disconnect();
}

bool RtspHandler::connect(const std::string& url)
{
    std::lock_guard<std::mutex> lock(stream_mutex_);

    BLOG_INFO("Connecting to RTSP (FFmpeg): %s", url.c_str());
    url_ = url;

    try {
        // Allocate format context
        format_ctx_ = avformat_alloc_context();
        if (!format_ctx_) {
            BLOG_ERROR("Failed to allocate AVFormatContext");
            return false;
        }

        // Set RTSP options for lower latency
        AVDictionary* options = nullptr;
        av_dict_set(&options, "rtsp_transport", "tcp", 0);  // Use TCP for reliability
        av_dict_set(&options, "max_delay", "500000", 0);    // 0.5 seconds max delay
        av_dict_set(&options, "timeout", "5000000", 0);     // 5 second timeout

        // Open RTSP stream
        int ret = avformat_open_input(&format_ctx_, url.c_str(), nullptr, &options);
        av_dict_free(&options);

        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            BLOG_ERROR("Failed to open RTSP stream: %s", errbuf);
            avformat_free_context(format_ctx_);
            format_ctx_ = nullptr;
            return false;
        }

        // Retrieve stream information
        ret = avformat_find_stream_info(format_ctx_, nullptr);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            BLOG_ERROR("Failed to find stream info: %s", errbuf);
            avformat_close_input(&format_ctx_);
            return false;
        }

        // Find video stream
        video_stream_index_ = -1;
        for (unsigned int i = 0; i < format_ctx_->nb_streams; i++) {
            if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_index_ = i;
                BLOG_INFO("Found video stream at index %d", i);
                BLOG_INFO("Video codec: %s", avcodec_get_name(format_ctx_->streams[i]->codecpar->codec_id));
                break;
            }
        }

        if (video_stream_index_ == -1) {
            BLOG_ERROR("No video stream found in RTSP stream");
            avformat_close_input(&format_ctx_);
            return false;
        }

        connected_ = true;
        BLOG_INFO("RTSP connection established successfully");
        return true;

    } catch (const std::exception& e) {
        BLOG_ERROR("Exception during RTSP connection: %s", e.what());
        if (format_ctx_) {
            avformat_close_input(&format_ctx_);
        }
        return false;
    } catch (...) {
        BLOG_ERROR("Unknown exception during RTSP connection");
        if (format_ctx_) {
            avformat_close_input(&format_ctx_);
        }
        return false;
    }
}

void RtspHandler::disconnect()
{
    std::lock_guard<std::mutex> lock(stream_mutex_);

    BLOG_INFO("Disconnecting RTSP (FFmpeg)");
    connected_ = false;

    if (format_ctx_) {
        try {
            avformat_close_input(&format_ctx_);
            format_ctx_ = nullptr;
        } catch (...) {
            BLOG_WARNING("Exception while closing RTSP stream");
            format_ctx_ = nullptr;
        }
    }

    video_stream_index_ = -1;
}

bool RtspHandler::is_connected() const
{
    return connected_;
}

bool RtspHandler::receive_frame(VideoFrame& frame)
{
    if (!connected_ || !format_ctx_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(stream_mutex_);

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        BLOG_ERROR("Failed to allocate AVPacket");
        return false;
    }

    try {
        // Read frame from stream
        int ret = av_read_frame(format_ctx_, packet);

        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                BLOG_WARNING("RTSP stream ended (EOF)");
                connected_ = false;
            } else if (ret == AVERROR(EAGAIN)) {
                // Would block, try again later
                av_packet_free(&packet);
                return false;
            } else {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                BLOG_WARNING("Error reading RTSP frame: %s", errbuf);
                connected_ = false;
            }
            av_packet_free(&packet);
            return false;
        }

        // Check if this is a video packet
        if (packet->stream_index != video_stream_index_) {
            av_packet_free(&packet);
            return false;
        }

        // Copy packet data to VideoFrame
        frame.size = packet->size;
        frame.data = new uint8_t[frame.size];
        memcpy(frame.data, packet->data, frame.size);
        frame.timestamp = packet->pts;

        av_packet_free(&packet);
        return true;

    } catch (const std::exception& e) {
        BLOG_ERROR("Exception while receiving RTSP frame: %s", e.what());
        av_packet_free(&packet);
        return false;
    } catch (...) {
        BLOG_ERROR("Unknown exception while receiving RTSP frame");
        av_packet_free(&packet);
        return false;
    }
}

} // namespace berrystreamcam
