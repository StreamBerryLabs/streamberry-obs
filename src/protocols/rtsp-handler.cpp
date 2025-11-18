#include "rtsp-handler.hpp"
#include <cstring>
#include <regex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

namespace berrystreamcam {

RtspHandler::RtspHandler()
    : rtsp_socket_(-1)
    , rtp_socket_(-1)
    , rtcp_socket_(-1)
    , port_(8554)
    , cseq_(1)
    , connected_(false)
    , frame_assembly_buffer_(nullptr)
    , frame_assembly_size_(0)
    , frame_assembly_started_(false)
    , last_sequence_number_(0)
    , last_timestamp_(0)
{
    frame_assembly_buffer_ = new uint8_t[FRAME_BUFFER_SIZE];
    BLOG_DEBUG("RTSP handler created");
}

RtspHandler::~RtspHandler()
{
    disconnect();
    delete[] frame_assembly_buffer_;
}

bool RtspHandler::connect(const std::string& url)
{
    BLOG_INFO("Connecting to RTSP: %s", url.c_str());

    url_ = url;

    // Parse RTSP URL (rtsp://ip:port/path)
    std::regex url_regex(R"(rtsp://([^:]+):(\d+)(/.*))");
    std::smatch matches;

    if (std::regex_match(url, matches, url_regex)) {
        host_ = matches[1].str();
        port_ = std::stoi(matches[2].str());
        path_ = matches[3].str();

        BLOG_DEBUG("  Host: %s, Port: %d, Path: %s",
                   host_.c_str(), port_, path_.c_str());
    } else {
        BLOG_ERROR("Invalid RTSP URL format");
        return false;
    }

    // Create RTSP control socket
    rtsp_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (rtsp_socket_ < 0) {
        BLOG_ERROR("Failed to create RTSP socket");
        return false;
    }

    // Connect to RTSP server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);

    if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
        BLOG_ERROR("Invalid IP address: %s", host_.c_str());
#ifdef _WIN32
        closesocket(rtsp_socket_);
#else
        close(rtsp_socket_);
#endif
        rtsp_socket_ = -1;
        return false;
    }

    if (::connect(rtsp_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        BLOG_ERROR("Failed to connect to RTSP server");
#ifdef _WIN32
        closesocket(rtsp_socket_);
#else
        close(rtsp_socket_);
#endif
        rtsp_socket_ = -1;
        return false;
    }

    BLOG_INFO("Connected to RTSP server");

    // RTSP handshake
    if (!send_options()) {
        BLOG_ERROR("RTSP OPTIONS failed");
        disconnect();
        return false;
    }

    if (!send_describe()) {
        BLOG_ERROR("RTSP DESCRIBE failed");
        disconnect();
        return false;
    }

    if (!send_setup()) {
        BLOG_ERROR("RTSP SETUP failed");
        disconnect();
        return false;
    }

    if (!send_play()) {
        BLOG_ERROR("RTSP PLAY failed");
        disconnect();
        return false;
    }

    connected_ = true;
    BLOG_INFO("RTSP session established");

    return true;
}

void RtspHandler::disconnect()
{
    BLOG_INFO("Disconnecting RTSP");

    bool was_connected = connected_;
    connected_ = false;

    // Send TEARDOWN only if we were connected
    if (was_connected) {
        try {
            send_teardown();
        } catch (const std::exception& e) {
            BLOG_WARNING("Exception during RTSP teardown: %s", e.what());
        }
    }

    // Close sockets with error handling
    if (rtsp_socket_ >= 0) {
        try {
#ifdef _WIN32
            closesocket(rtsp_socket_);
#else
            close(rtsp_socket_);
#endif
        } catch (...) {
            BLOG_WARNING("Exception closing RTSP socket");
        }
        rtsp_socket_ = -1;
    }

    if (rtp_socket_ >= 0) {
        try {
#ifdef _WIN32
            closesocket(rtp_socket_);
#else
            close(rtp_socket_);
#endif
        } catch (...) {
            BLOG_WARNING("Exception closing RTP socket");
        }
        rtp_socket_ = -1;
    }

    if (rtcp_socket_ >= 0) {
        try {
#ifdef _WIN32
        closesocket(rtcp_socket_);
#else
        close(rtcp_socket_);
#endif
        } catch (...) {
            BLOG_WARNING("Exception closing RTCP socket");
        }
        rtcp_socket_ = -1;
    }

    // Clear frame queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!frame_queue_.empty()) {
        frame_queue_.pop();
    }
}

bool RtspHandler::is_connected() const
{
    return connected_;
}

bool RtspHandler::receive_frame(VideoFrame& frame)
{
    // Receive RTP packets and assemble frames
    receive_rtp_packets();

    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (frame_queue_.empty()) {
        return false;
    }

    frame = frame_queue_.front();
    frame_queue_.pop();

    return true;
}

bool RtspHandler::send_options()
{
    char request[512];
    snprintf(request, sizeof(request),
             "OPTIONS %s RTSP/1.0\r\n"
             "CSeq: %d\r\n"
             "User-Agent: BerryStreamCam/1.0\r\n"
             "\r\n",
             url_.c_str(), cseq_++);

    if (send(rtsp_socket_, request, strlen(request), 0) < 0) {
        return false;
    }

    // Receive response
    char response[1024];
    int received = recv(rtsp_socket_, response, sizeof(response) - 1, 0);
    if (received <= 0) {
        return false;
    }
    response[received] = '\0';

    BLOG_DEBUG("RTSP OPTIONS response received");
    return strstr(response, "200 OK") != nullptr;
}

bool RtspHandler::send_describe()
{
    char request[512];
    snprintf(request, sizeof(request),
             "DESCRIBE %s RTSP/1.0\r\n"
             "CSeq: %d\r\n"
             "Accept: application/sdp\r\n"
             "User-Agent: BerryStreamCam/1.0\r\n"
             "\r\n",
             url_.c_str(), cseq_++);

    if (send(rtsp_socket_, request, strlen(request), 0) < 0) {
        return false;
    }

    // Receive response with SDP
    char response[4096];
    int received = recv(rtsp_socket_, response, sizeof(response) - 1, 0);
    if (received <= 0) {
        return false;
    }
    response[received] = '\0';

    if (strstr(response, "200 OK") == nullptr) {
        return false;
    }

    // Parse SDP
    const char* sdp_start = strstr(response, "\r\n\r\n");
    if (sdp_start) {
        parse_sdp(sdp_start + 4);
    }

    BLOG_DEBUG("RTSP DESCRIBE completed");
    return true;
}

bool RtspHandler::send_setup()
{
    // Request RTP/UDP transport
    char request[512];
    snprintf(request, sizeof(request),
             "SETUP %s/trackID=0 RTSP/1.0\r\n"
             "CSeq: %d\r\n"
             "Transport: RTP/AVP;unicast;client_port=50000-50001\r\n"
             "User-Agent: BerryStreamCam/1.0\r\n"
             "\r\n",
             url_.c_str(), cseq_++);

    if (send(rtsp_socket_, request, strlen(request), 0) < 0) {
        return false;
    }

    // Receive response
    char response[1024];
    int received = recv(rtsp_socket_, response, sizeof(response) - 1, 0);
    if (received <= 0) {
        return false;
    }
    response[received] = '\0';

    if (strstr(response, "200 OK") == nullptr) {
        return false;
    }

    // Extract session ID
    const char* session_line = strstr(response, "Session: ");
    if (session_line) {
        char session[64];
        sscanf(session_line, "Session: %63s", session);
        session_id_ = session;
        BLOG_DEBUG("RTSP Session ID: %s", session_id_.c_str());
    }

    // Create RTP/RTCP sockets
    rtp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    rtcp_socket_ = socket(AF_INET, SOCK_DGRAM, 0);

    BLOG_DEBUG("RTSP SETUP completed");
    return true;
}

bool RtspHandler::send_play()
{
    char request[512];
    snprintf(request, sizeof(request),
             "PLAY %s RTSP/1.0\r\n"
             "CSeq: %d\r\n"
             "Session: %s\r\n"
             "Range: npt=0.000-\r\n"
             "User-Agent: BerryStreamCam/1.0\r\n"
             "\r\n",
             url_.c_str(), cseq_++, session_id_.c_str());

    if (send(rtsp_socket_, request, strlen(request), 0) < 0) {
        return false;
    }

    // Receive response
    char response[1024];
    int received = recv(rtsp_socket_, response, sizeof(response) - 1, 0);
    if (received <= 0) {
        return false;
    }
    response[received] = '\0';

    BLOG_DEBUG("RTSP PLAY started");
    return strstr(response, "200 OK") != nullptr;
}

bool RtspHandler::send_teardown()
{
    if (rtsp_socket_ < 0 || session_id_.empty()) {
        return false;
    }

    char request[512];
    snprintf(request, sizeof(request),
             "TEARDOWN %s RTSP/1.0\r\n"
             "CSeq: %d\r\n"
             "Session: %s\r\n"
             "User-Agent: BerryStreamCam/1.0\r\n"
             "\r\n",
             url_.c_str(), cseq_++, session_id_.c_str());

    send(rtsp_socket_, request, strlen(request), 0);

    BLOG_DEBUG("RTSP TEARDOWN sent");
    return true;
}

void RtspHandler::parse_sdp(const std::string& sdp)
{
    BLOG_DEBUG("Parsing SDP:\n%s", sdp.c_str());

    // Extract video format, codec info, etc. from SDP
    // This would parse fields like:
    // m=video 0 RTP/AVP 96
    // a=rtpmap:96 H264/90000
    // a=fmtp:96 packetization-mode=1;profile-level-id=42e01f;sprop-parameter-sets=...
}

void RtspHandler::receive_rtp_packets()
{
    if (rtp_socket_ < 0) return;

    // Receive RTP packet
    uint8_t rtp_buffer[2048];
    int received = recv(rtp_socket_, (char*)rtp_buffer, sizeof(rtp_buffer), MSG_DONTWAIT);

    if (received > 0) {
        // Parse RTP header
        // RTP header is 12 bytes minimum
        if (received < 12) return;

        bool marker = (rtp_buffer[1] & 0x80) != 0;
        uint16_t sequence = (rtp_buffer[2] << 8) | rtp_buffer[3];
        uint32_t timestamp = (rtp_buffer[4] << 24) | (rtp_buffer[5] << 16) |
                            (rtp_buffer[6] << 8) | rtp_buffer[7];

        // RTP payload starts after header
        const uint8_t* payload = rtp_buffer + 12;
        size_t payload_size = received - 12;

        assemble_frame(payload, payload_size, marker);
    }
}

void RtspHandler::assemble_frame(const uint8_t* rtp_payload, size_t length, bool marker)
{
    // Assemble H.264 NAL units from RTP payload
    // This handles fragmentation units (FU-A) and single NAL units

    if (frame_assembly_size_ + length > FRAME_BUFFER_SIZE) {
        BLOG_WARNING("Frame assembly buffer overflow, resetting");
        frame_assembly_size_ = 0;
        frame_assembly_started_ = false;
        return;
    }

    memcpy(frame_assembly_buffer_ + frame_assembly_size_, rtp_payload, length);
    frame_assembly_size_ += length;
    frame_assembly_started_ = true;

    // If marker bit is set, frame is complete
    if (marker && frame_assembly_started_) {
        VideoFrame frame = {};
        frame.data = new uint8_t[frame_assembly_size_];
        memcpy(frame.data, frame_assembly_buffer_, frame_assembly_size_);
        frame.size = frame_assembly_size_;
        frame.timestamp = last_timestamp_;
        frame.is_keyframe = false; // Would determine from NAL type

        std::lock_guard<std::mutex> lock(queue_mutex_);
        frame_queue_.push(frame);

        // Reset for next frame
        frame_assembly_size_ = 0;
        frame_assembly_started_ = false;
    }
}

} // namespace berrystreamcam
