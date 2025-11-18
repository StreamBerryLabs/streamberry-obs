#include "http-handler.hpp"
#include <cstring>

namespace berrystreamcam {

HttpHandler::HttpHandler()
    : connected_(false)
    , running_(false)
    , protocol_type_(ProtocolType::UNKNOWN)
    , format_context_(nullptr)
    , video_stream_index_(-1)
{
    BLOG_DEBUG("HTTP handler created (FFmpeg-based)");
}

HttpHandler::~HttpHandler()
{
    disconnect();
}

bool HttpHandler::connect(const std::string& url, ProtocolType type)
{
    BLOG_INFO("Connecting to HTTP: %s", url.c_str());

    url_ = url;
    protocol_type_ = type;

    try {
        // Allocate format context
        format_context_ = avformat_alloc_context();
        if (!format_context_) {
            BLOG_ERROR("Failed to allocate AVFormatContext");
            return false;
        }

        // Set HTTP options
        AVDictionary* options = nullptr;
        av_dict_set(&options, "timeout", "5000000", 0);  // 5 second timeout
        av_dict_set(&options, "reconnect", "1", 0);      // Auto-reconnect
        av_dict_set(&options, "reconnect_streamed", "1", 0);
        av_dict_set(&options, "reconnect_delay_max", "2", 0);

        // Open HTTP stream
        int ret = avformat_open_input(&format_context_, url.c_str(), nullptr, &options);
        av_dict_free(&options);

        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            BLOG_ERROR("Failed to open HTTP stream: %s", errbuf);
            avformat_free_context(format_context_);
            format_context_ = nullptr;
            return false;
        }

        // Find stream information
        ret = avformat_find_stream_info(format_context_, nullptr);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            BLOG_ERROR("Failed to find stream info: %s", errbuf);
            avformat_close_input(&format_context_);
            return false;
        }

        // Find video stream
        video_stream_index_ = -1;
        for (unsigned int i = 0; i < format_context_->nb_streams; i++) {
            if (format_context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                video_stream_index_ = i;
                BLOG_INFO("Found video stream at index %d", i);
                break;
            }
        }

        if (video_stream_index_ == -1) {
            BLOG_ERROR("No video stream found in HTTP stream");
            avformat_close_input(&format_context_);
            return false;
        }

        connected_ = true;
        running_ = true;

        // Start streaming thread
        streaming_thread_ = std::thread(&HttpHandler::stream_loop, this);

        BLOG_INFO("HTTP connection established successfully");
        return true;

    } catch (const std::exception& e) {
        BLOG_ERROR("Exception during HTTP connection: %s", e.what());
        if (format_context_) {
            avformat_close_input(&format_context_);
        }
        return false;
    } catch (...) {
        BLOG_ERROR("Unknown exception during HTTP connection");
        if (format_context_) {
            avformat_close_input(&format_context_);
        }
        return false;
    }
}

void HttpHandler::disconnect()
{
    BLOG_INFO("Disconnecting HTTP (FFmpeg)");

    connected_ = false;
    running_ = false;

    // Wait for streaming thread
    if (streaming_thread_.joinable()) {
        try {
            streaming_thread_.join();
        } catch (...) {
            BLOG_WARNING("Exception while joining HTTP streaming thread");
        }
    }

    // Close format context
    if (format_context_) {
        try {
            avformat_close_input(&format_context_);
            format_context_ = nullptr;
        } catch (...) {
            BLOG_WARNING("Exception while closing HTTP stream");
            format_context_ = nullptr;
        }
    }

    video_stream_index_ = -1;

    // Clear frame queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!frame_queue_.empty()) {
            if (frame_queue_.front().data) {
                delete[] frame_queue_.front().data;
            }
            frame_queue_.pop();
        }
    }
}

bool HttpHandler::is_connected() const
{
    return connected_;
}

bool HttpHandler::receive_frame(VideoFrame& frame)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (frame_queue_.empty()) {
        return false;
    }

    frame = frame_queue_.front();
    frame_queue_.pop();

    static int frame_count = 0;
    if (++frame_count % 100 == 0) {
        BLOG_DEBUG("HTTP received %d frames (queue size: %zu)", frame_count, frame_queue_.size());
    }

    return true;
}

void HttpHandler::stream_loop()
{
    BLOG_INFO("HTTP streaming thread started");

    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        BLOG_ERROR("Failed to allocate AVPacket");
        running_ = false;
        return;
    }

    while (running_ && connected_) {
        try {
            // Read frame from stream
            int ret = av_read_frame(format_context_, packet);

            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    BLOG_WARNING("HTTP stream ended (EOF)");
                    connected_ = false;
                    break;
                } else if (ret == AVERROR(EAGAIN)) {
                    // Would block, try again
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                } else {
                    char errbuf[AV_ERROR_MAX_STRING_SIZE];
                    av_strerror(ret, errbuf, sizeof(errbuf));
                    BLOG_WARNING("Error reading HTTP frame: %s", errbuf);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
            }

            // Check if this is a video packet
            if (packet->stream_index != video_stream_index_) {
                av_packet_unref(packet);
                continue;
            }

            // Copy packet data to VideoFrame
            VideoFrame video_frame;
            video_frame.size = packet->size;
            video_frame.data = new uint8_t[video_frame.size];
            memcpy(video_frame.data, packet->data, video_frame.size);
            video_frame.timestamp = packet->pts;

            // Add to queue
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);

                // Limit queue size to prevent memory buildup
                if (frame_queue_.size() >= 30) {
                    // Drop oldest frame
                    if (frame_queue_.front().data) {
                        delete[] frame_queue_.front().data;
                    }
                    frame_queue_.pop();
                }

                frame_queue_.push(video_frame);

                static int push_count = 0;
                if (++push_count % 100 == 0) {
                    BLOG_DEBUG("HTTP pushed %d frames (packet size: %d, queue size: %zu)",
                               push_count, packet->size, frame_queue_.size());
                }
            }

            av_packet_unref(packet);

        } catch (const std::exception& e) {
            BLOG_ERROR("Exception in HTTP streaming loop: %s", e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (...) {
            BLOG_ERROR("Unknown exception in HTTP streaming loop");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    av_packet_free(&packet);
    BLOG_INFO("HTTP streaming thread stopped");
}

} // namespace berrystreamcam
