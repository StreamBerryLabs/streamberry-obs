#include "websocket-handler.hpp"
#include <QUrl>
#include <QJsonArray>
#include <QMetaObject>
#include <cstring>

namespace berrystreamcam {

WebSocketHandler::WebSocketHandler()
    : websocket_(nullptr)
    , connected_(false)
    , connection_attempted_(false)
    , cleanup_started_(false)
    , frame_count_(0)
    , keyframe_count_(0)
{
    // Create websocket in main thread context
    websocket_ = std::make_unique<QWebSocket>();

    // Connect Qt signals (using QObject::connect, not our connect method)
    QObject::connect(websocket_.get(), &QWebSocket::connected,
                     this, &WebSocketHandler::on_connected);
    QObject::connect(websocket_.get(), &QWebSocket::disconnected,
                     this, &WebSocketHandler::on_disconnected);
    QObject::connect(websocket_.get(), &QWebSocket::textMessageReceived,
                     this, &WebSocketHandler::on_text_message_received);
    QObject::connect(websocket_.get(), &QWebSocket::errorOccurred,
                     this, &WebSocketHandler::on_error_occurred);

    BLOG_INFO("WebSocket handler created");
}

WebSocketHandler::~WebSocketHandler()
{
    // Set cleanup flag FIRST to stop all callbacks
    cleanup_started_ = true;
    connected_ = false;

    try {
        // Clear frame queue first
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            while (!frame_queue_.empty()) {
                if (frame_queue_.front().data) {
                    delete[] frame_queue_.front().data;
                }
                frame_queue_.pop();
            }
        }

        // Clean up websocket without any Qt calls
        if (websocket_) {
            // Block all signals immediately
            websocket_->blockSignals(true);

            // Delete the websocket directly
            websocket_.reset();
        }

    } catch (...) {
        // Suppress all exceptions in destructor
        websocket_.reset();
    }
}

bool WebSocketHandler::connect_to_server(const std::string& url)
{
    if (cleanup_started_) {
        return false;
    }

    BLOG_INFO("Connecting to WebSocket: %s", url.c_str());

    url_ = url;

    QUrl qurl(QString::fromStdString(url));
    if (!qurl.isValid()) {
        BLOG_ERROR("Invalid WebSocket URL: %s", url.c_str());
        return false;
    }

    // If websocket is in a bad state, recreate it
    if (!websocket_ || websocket_->state() == QAbstractSocket::ClosingState) {
        BLOG_DEBUG("Recreating WebSocket");
        websocket_.reset();
        websocket_ = std::make_unique<QWebSocket>();

        // Reconnect Qt signals
        QObject::connect(websocket_.get(), &QWebSocket::connected,
                         this, &WebSocketHandler::on_connected);
        QObject::connect(websocket_.get(), &QWebSocket::disconnected,
                         this, &WebSocketHandler::on_disconnected);
        QObject::connect(websocket_.get(), &QWebSocket::textMessageReceived,
                         this, &WebSocketHandler::on_text_message_received);
        QObject::connect(websocket_.get(), &QWebSocket::errorOccurred,
                         this, &WebSocketHandler::on_error_occurred);
    }

    connection_attempted_ = false;
    connected_ = false;

    // Create a local event loop to wait for connection
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(5000); // 5 second timeout

    // Connect signals to event loop
    QObject::connect(websocket_.get(), &QWebSocket::connected, &loop, &QEventLoop::quit);
    QObject::connect(websocket_.get(), &QWebSocket::errorOccurred, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    // Start connection and timeout
    websocket_->open(qurl);
    timeout.start();

    // Wait for connection or timeout
    loop.exec();

    if (!connected_) {
        BLOG_ERROR("WebSocket connection timeout or failed");
    }

    return connected_;
}

void WebSocketHandler::process_events()
{
    // Process Qt events to handle incoming WebSocket messages
    QCoreApplication::processEvents();
}

void WebSocketHandler::disconnect_from_server()
{
    if (!websocket_ || cleanup_started_) return;

    BLOG_INFO("Disconnecting WebSocket");

    connected_ = false;

    // Abort connection immediately
    try {
        if (websocket_->state() != QAbstractSocket::UnconnectedState) {
            websocket_->blockSignals(true);
            websocket_->abort();
        }
    } catch (...) {
        // Ignore all errors
    }

    // Clear frame queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!frame_queue_.empty()) {
            VideoFrame& frame = frame_queue_.front();
            if (frame.data) {
                delete[] frame.data;
            }
            frame_queue_.pop();
        }
    }

    BLOG_INFO("WebSocket disconnected (processed %d frames, %d keyframes)",
              frame_count_, keyframe_count_);
}

void WebSocketHandler::cleanup_websocket()
{
    // This method is now unused but kept for compatibility
    if (websocket_) {
        websocket_->blockSignals(true);
        websocket_.reset();
    }
}

bool WebSocketHandler::is_connected() const
{
    return connected_;
}

bool WebSocketHandler::receive_frame(VideoFrame& frame)
{
    if (cleanup_started_) {
        return false;
    }

    std::lock_guard<std::mutex> lock(queue_mutex_);

    if (frame_queue_.empty()) {
        return false;
    }

    frame = frame_queue_.front();
    frame_queue_.pop();

    return true;
}

void WebSocketHandler::on_connected()
{
    BLOG_INFO("WebSocket connected successfully");
    connected_ = true;
    connection_attempted_ = true;
}

void WebSocketHandler::on_disconnected()
{
    BLOG_INFO("WebSocket disconnected");
    connected_ = false;
}

void WebSocketHandler::on_text_message_received(const QString& message)
{
    if (cleanup_started_ || !connected_) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        BLOG_ERROR("Received invalid JSON message");
        return;
    }

    QJsonObject json = doc.object();
    QString type = json["type"].toString();

    if (type == "hello") {
        handle_hello_message(json);
    } else if (type == "video_frame") {
        handle_video_frame(json);
    } else if (type == "audio_frame") {
        // Audio frames not yet supported
    } else if (type == "metadata") {
        // Metadata not yet used
    } else {
        BLOG_DEBUG("Received message type: %s", type.toStdString().c_str());
    }
}

void WebSocketHandler::on_error_occurred(QAbstractSocket::SocketError error)
{
    if (cleanup_started_) {
        return;
    }

    if (websocket_) {
        QString errorString = websocket_->errorString();
        BLOG_ERROR("WebSocket error %d: %s",
                   static_cast<int>(error),
                   errorString.toStdString().c_str());
    }
    connected_ = false;
    connection_attempted_ = true;
}

void WebSocketHandler::handle_hello_message(const QJsonObject& json)
{
    QString version = json["version"].toString();
    QString client = json["client"].toString();

    BLOG_INFO("Received hello from %s (version %s)",
              client.toStdString().c_str(),
              version.toStdString().c_str());

    // Parse capabilities
    if (json.contains("capabilities")) {
        QJsonObject caps = json["capabilities"].toObject();
        QJsonArray videoCodecs = caps["videoCodecs"].toArray();

        QString codecsStr;
        for (const auto& codec : videoCodecs) {
            if (!codecsStr.isEmpty()) codecsStr += ", ";
            codecsStr += codec.toString();
        }

        BLOG_INFO("  Video codecs: %s", codecsStr.toStdString().c_str());
        BLOG_INFO("  Max resolution: %s",
                  caps["maxResolution"].toString().toStdString().c_str());
        BLOG_INFO("  Max framerate: %d", caps["maxFramerate"].toInt());
    }
}

void WebSocketHandler::handle_video_frame(const QJsonObject& json)
{
    if (cleanup_started_) {
        return;
    }

    // Extract frame metadata
    qint64 timestamp = json["timestamp"].toInteger();
    qint64 pts = json["pts"].toInteger();
    qint64 dts = json["dts"].toInteger();
    int sequence = json["sequence"].toInt();
    bool is_keyframe = json["keyframe"].toBool();
    QString codec = json["codec"].toString();
    QString base64_data = json["data"].toString();

    if (base64_data.isEmpty()) {
        BLOG_ERROR("Video frame has no data");
        return;
    }

    // Decode base64 to get H.264 NAL units in Annex-B format
    QByteArray decoded = decode_base64(base64_data);
    if (decoded.isEmpty()) {
        BLOG_ERROR("Failed to decode base64 video data");
        return;
    }

    // Create VideoFrame
    VideoFrame frame = {};
    frame.data = new uint8_t[decoded.size()];
    frame.size = decoded.size();
    frame.timestamp = timestamp;
    frame.pts = pts;
    frame.dts = dts;
    frame.is_keyframe = is_keyframe;

    memcpy(frame.data, decoded.data(), decoded.size());

    // Add to queue with size limit
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        // Limit queue to 30 frames
        while (frame_queue_.size() >= 30) {
            VideoFrame& old = frame_queue_.front();
            delete[] old.data;
            frame_queue_.pop();
        }

        frame_queue_.push(frame);
    }

    frame_count_++;
    if (is_keyframe) {
        keyframe_count_++;
        BLOG_INFO("🔑 Keyframe #%d received (%zu bytes, seq=%d)",
                  keyframe_count_, frame.size, sequence);
    } else if (frame_count_ % 60 == 0) {
        BLOG_DEBUG("Frame #%d received (%zu bytes, seq=%d)",
                   frame_count_, frame.size, sequence);
    }
}

QByteArray WebSocketHandler::decode_base64(const QString& base64_str)
{
    // Qt handles base64 decoding
    return QByteArray::fromBase64(base64_str.toUtf8());
}

} // namespace berrystreamcam
