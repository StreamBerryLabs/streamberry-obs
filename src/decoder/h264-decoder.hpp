#pragma once

#include "../common.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace berrystreamcam {

class H264Decoder {
public:
    H264Decoder();
    ~H264Decoder();

    bool initialize();
    void shutdown();
    void flush();

    bool decode_frame(const uint8_t* encoded_data, size_t encoded_size,
                     uint8_t** decoded_data, int* width, int* height);

private:
    const AVCodec* codec_;
    AVCodecContext* codec_context_;
    AVFrame* frame_;
    AVPacket* packet_;
    SwsContext* sws_context_;

    uint8_t* rgba_buffer_;
    size_t rgba_buffer_size_;

    int last_width_;
    int last_height_;
};

} // namespace berrystreamcam
