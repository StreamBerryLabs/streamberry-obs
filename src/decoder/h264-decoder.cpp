#include "h264-decoder.hpp"
#include <cstring>

namespace berrystreamcam {

H264Decoder::H264Decoder()
    : codec_(nullptr)
    , codec_context_(nullptr)
    , frame_(nullptr)
    , packet_(nullptr)
    , sws_context_(nullptr)
    , rgba_buffer_(nullptr)
    , rgba_buffer_size_(0)
    , last_width_(0)
    , last_height_(0)
{
}

H264Decoder::~H264Decoder()
{
    shutdown();
}

bool H264Decoder::initialize()
{
    BLOG_INFO("Initializing video decoder");

    // Suppress FFmpeg's internal logging to prevent error spam
    av_log_set_level(AV_LOG_QUIET);

    // Start with H.264 decoder (will auto-switch to MJPEG if needed)
    codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec_) {
        BLOG_ERROR("H.264 decoder not found");
        return false;
    }

    // Create codec context
    codec_context_ = avcodec_alloc_context3(codec_);
    if (!codec_context_) {
        BLOG_ERROR("Failed to allocate codec context");
        return false;
    }

    // Configure codec context for low latency
    codec_context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
    codec_context_->flags2 |= AV_CODEC_FLAG2_FAST;
    codec_context_->thread_count = 4;
    codec_context_->thread_type = FF_THREAD_SLICE;

    // Open codec
    if (avcodec_open2(codec_context_, codec_, nullptr) < 0) {
        BLOG_ERROR("Failed to open codec");
        avcodec_free_context(&codec_context_);
        return false;
    }

    // Allocate frame
    frame_ = av_frame_alloc();
    if (!frame_) {
        BLOG_ERROR("Failed to allocate frame");
        avcodec_free_context(&codec_context_);
        return false;
    }

    // Allocate packet
    packet_ = av_packet_alloc();
    if (!packet_) {
        BLOG_ERROR("Failed to allocate packet");
        av_frame_free(&frame_);
        avcodec_free_context(&codec_context_);
        return false;
    }

    // Allocate initial RGBA buffer (will be resized as needed)
    rgba_buffer_size_ = 1920 * 1080 * 4;
    rgba_buffer_ = new uint8_t[rgba_buffer_size_];

    BLOG_INFO("H.264 decoder initialized successfully");
    return true;
}

void H264Decoder::shutdown()
{
    BLOG_INFO("Shutting down H.264 decoder");

    if (sws_context_) {
        sws_freeContext(sws_context_);
        sws_context_ = nullptr;
    }

    if (packet_) {
        av_packet_free(&packet_);
    }

    if (frame_) {
        av_frame_free(&frame_);
    }

    if (codec_context_) {
        avcodec_free_context(&codec_context_);
    }

    if (rgba_buffer_) {
        delete[] rgba_buffer_;
        rgba_buffer_ = nullptr;
    }
}

void H264Decoder::flush()
{
    if (codec_context_) {
        avcodec_flush_buffers(codec_context_);
    }
}

bool H264Decoder::decode_frame(const uint8_t* encoded_data, size_t encoded_size,
                              uint8_t** decoded_data, int* width, int* height)
{
    if (!codec_context_ || !frame_ || !packet_) {
        return false;
    }

    // Auto-detect codec type and switch if needed
    bool is_mjpeg = (encoded_size >= 2 && encoded_data[0] == 0xFF && encoded_data[1] == 0xD8);
    bool is_h264 = (encoded_size >= 4 &&
                    encoded_data[0] == 0x00 && encoded_data[1] == 0x00 &&
                    (encoded_data[2] == 0x00 || encoded_data[2] == 0x01));

    if (is_mjpeg && codec_context_->codec_id != AV_CODEC_ID_MJPEG) {
        // Switch to MJPEG decoder
        BLOG_INFO("Detected MJPEG stream, switching decoder");

        // Close current codec
        avcodec_free_context(&codec_context_);

        // Open MJPEG decoder
        codec_ = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
        if (!codec_) {
            BLOG_ERROR("MJPEG decoder not found");
            return false;
        }

        codec_context_ = avcodec_alloc_context3(codec_);
        if (!codec_context_) {
            BLOG_ERROR("Failed to allocate MJPEG codec context");
            return false;
        }

        codec_context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codec_context_->flags2 |= AV_CODEC_FLAG2_FAST;

        if (avcodec_open2(codec_context_, codec_, nullptr) < 0) {
            BLOG_ERROR("Failed to open MJPEG codec");
            avcodec_free_context(&codec_context_);
            return false;
        }

        BLOG_INFO("Switched to MJPEG decoder");
    } else if (is_h264 && codec_context_->codec_id != AV_CODEC_ID_H264) {
        // Switch to H.264 decoder
        BLOG_INFO("Detected H.264 stream, switching decoder");

        // Close current codec
        avcodec_free_context(&codec_context_);

        // Open H.264 decoder
        codec_ = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec_) {
            BLOG_ERROR("H.264 decoder not found");
            return false;
        }

        codec_context_ = avcodec_alloc_context3(codec_);
        if (!codec_context_) {
            BLOG_ERROR("Failed to allocate H.264 codec context");
            return false;
        }

        codec_context_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codec_context_->flags2 |= AV_CODEC_FLAG2_FAST;

        if (avcodec_open2(codec_context_, codec_, nullptr) < 0) {
            BLOG_ERROR("Failed to open H.264 codec");
            avcodec_free_context(&codec_context_);
            return false;
        }

        BLOG_INFO("Switched to H.264 decoder");
    }

    // Prepare packet
    packet_->data = const_cast<uint8_t*>(encoded_data);
    packet_->size = encoded_size;

    // Send packet to decoder
    int ret = avcodec_send_packet(codec_context_, packet_);
    if (ret < 0) {
        // Silently ignore errors - FFmpeg logging is suppressed
        // Most errors are EAGAIN which means decoder needs more data
        return false;
    }    // Receive decoded frame
    ret = avcodec_receive_frame(codec_context_, frame_);
    if (ret == AVERROR(EAGAIN)) {
        // Need more data - this is normal, happens frequently
        return false;
    } else if (ret < 0) {
        // Decode error - silently ignore
        return false;
    }

    // Frame decoded successfully
    *width = frame_->width;
    *height = frame_->height;

    // Check if we need to reallocate RGBA buffer
    size_t required_size = frame_->width * frame_->height * 4;
    if (required_size > rgba_buffer_size_) {
        delete[] rgba_buffer_;
        rgba_buffer_size_ = required_size;
        rgba_buffer_ = new uint8_t[rgba_buffer_size_];
        BLOG_DEBUG("Reallocated RGBA buffer to %zu bytes", rgba_buffer_size_);
    }

    // Check if we need to recreate swscale context
    if (!sws_context_ ||
        frame_->width != last_width_ ||
        frame_->height != last_height_) {

        if (sws_context_) {
            sws_freeContext(sws_context_);
        }

        sws_context_ = sws_getContext(
            frame_->width, frame_->height, static_cast<AVPixelFormat>(frame_->format),
            frame_->width, frame_->height, AV_PIX_FMT_RGBA,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
        );

        if (!sws_context_) {
            BLOG_ERROR("Failed to create swscale context");
            return false;
        }

        last_width_ = frame_->width;
        last_height_ = frame_->height;
    }

    // Convert YUV to RGBA
    uint8_t* dst_data[1] = { rgba_buffer_ };
    int dst_linesize[1] = { frame_->width * 4 };

    sws_scale(
        sws_context_,
        frame_->data, frame_->linesize,
        0, frame_->height,
        dst_data, dst_linesize
    );

    *decoded_data = rgba_buffer_;

    return true;
}

} // namespace berrystreamcam
