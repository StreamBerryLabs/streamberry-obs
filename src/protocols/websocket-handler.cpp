#include "websocket-handler.hpp"
#include <QUrl>
#include <QJsonArray>
#include <QMetaObject>
#include <cstring>
#include <chrono>
#include <thread>

namespace berrystreamcam {

WebSocketHandler::WebSocketHandler()
    : websocket_(nullptr)
    , worker_thread_(nullptr)
    , connected_(false)
    , connection_attempted_(false)
    , cleanup_started_(false)
    , frame_count_(0)
    , keyframe_count_(0)
{
    BLOG_INFO("WebSocket handler created in thread %p", QThread::currentThread());

    // Create websocket in main thread context
    websocket_ = new QWebSocket();

    // Create worker thread
    worker_thread_ = new QThread(this);

    // Move websocket to worker thread
    websocket_->moveToThread(worker_thread_);

    // Connect cross-thread signals (queued connections for thread safety)
    QObject::connect(this, &WebSocketHandler::connectRequested,
                     this, &WebSocketHandler::doConnect,
                     Qt::QueuedConnection);

    QObject::connect(this, &WebSocketHandler::disconnectRequested,
                     this, &WebSocketHandler::doDisconnect,
                     Qt::QueuedConnection);

    // Connect WebSocket signals to our slots (queued for cross-thread)
    QObject::connect(websocket_, &QWebSocket::connected,
                     this, &WebSocketHandler::onConnected,
                     Qt::QueuedConnection);

    QObject::connect(websocket_, &QWebSocket::disconnected,
                     this, &WebSocketHandler::onDisconnected,
                     Qt::QueuedConnection);

    QObject::connect(websocket_, &QWebSocket::textMessageReceived,
                     this, &WebSocketHandler::onTextMessageReceived,
                     Qt::QueuedConnection);

    QObject::connect(websocket_, &QWebSocket::errorOccurred,
                     this, &WebSocketHandler::onError,
                     Qt::QueuedConnection);

    // Start worker thread with event loop
    worker_thread_->start();

    BLOG_INFO("WebSocket worker thread started");
}

WebSocketHandler::~WebSocketHandler()
{
    BLOG_INFO("WebSocket handler destructor called");

    // Set cleanup flag FIRST to stop all callbacks
    cleanup_started_.store(true);
    connected_.store(false);

    try {
        // Request disconnect from worker thread
        emit disconnectRequested();

        // Wait for worker thread to finish (with timeout)
        if (worker_thread_ && worker_thread_->isRunning()) {
            BLOG_DEBUG("Waiting for worker thread to stop...");
            if (!worker_thread_->wait(2000)) {
                BLOG_WARNING("Worker thread did not stop within timeout, forcing quit");
                worker_thread_->quit();
                if (!worker_thread_->wait(1000)) {
                    BLOG_ERROR("Worker thread still running after forced quit");
                }
            }
        }

        // Clear frame queue
        frame_queue_.clear();

        // Delete WebSocket (safe now that thread is stopped)
        if (websocket_) {
            delete websocket_;
            websocket_ = nullptr;
        }

        BLOG_INFO("WebSocket handler destroyed");

    } catch (...) {
        // Suppress all exceptions in destructor
        BLOG_ERROR("Exception in WebSocketHandler destructor");
        if (websocket_) {
            delete websocket_;
            websocket_ = nullptr;
        }
    }
}

bool WebSocketHandler::connect_to_server(const std::string& url)
{
    if (cleanup_started_.load()) {
        return false;
    }

    BLOG_INFO("Connecting to WebSocket: %s", url.c_str());

    url_ = url;

    QUrl qurl(QString::fromStdString(url));
    if (!qurl.isValid()) {
        BLOG_ERROR("Invalid WebSocket URL: %s", url.c_str());
        return false;
    }

    connection_attempted_.store(false);
    connected_.store(false);

    // Emit signal to worker thread to connect
    emit connectRequested(QString::fromStdString(url));

    // Wait for connection with timeout using sleep and polling
    // This avoids creating QEventLoop in streaming thread
    int timeout_ms = 5000;
    int elapsed_ms = 0;
    int poll_interval_ms = 50;

    while (elapsed_ms < timeout_ms && !connection_attempted_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
        elapsed_ms += poll_interval_ms;
    }

    if (!connected_.load()) {
        BLOG_ERROR("WebSocket connection timeout or failed");
    }

    return connected_.load();
}

void WebSocketHandler::process_events()
{
    if (cleanup_started_.load()) {
        return;
    }

    // Process Qt events to handle incoming WebSocket messages
    QCoreApplication::processEvents();
}

void WebSocketHandler::disconnect_from_server()
{
    if (cleanup_started_.load()) return;

    BLOG_INFO("Disconnecting WebSocket");

    connected_.store(false);

    // Emit signal to worker thread to disconnect
    emit disconnectRequested();

    // Wait for disconnection with timeout using sleep and polling
    // This avoids creating QEventLoop in streaming thread
    int timeout_ms = 2000;
    int elapsed_ms = 0;
    int poll_interval_ms = 50;

    while (elapsed_ms < timeout_ms && connected_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_interval_ms));
        elapsed_ms += poll_interval_ms;
    }

    // Clear frame queue
    frame_queue_.clear();

    BLOG_INFO("WebSocket disconnected (processed %d frames, %d keyframes)",
              frame_count_, keyframe_count_);
}

void WebSocketHandler::cleanup_websocket()
{
    // This method is now unused but kept for compatibility
    if (websocket_) {
        websocket_->blockSignals(true);
        delete websocket_;
        websocket_ = nullptr;
    }
}

bool WebSocketHandler::is_connected() const
{
    return connected_.load();
}

bool WebSocketHandler::receive_frame(VideoFrame& frame)
{
    if (cleanup_started_.load()) {
        return false;
    }

    return frame_queue_.pop(frame);
}

void WebSocketHandler::doConnect(const QString& url)
{
    if (cleanup_started_.load()) {
        BLOG_DEBUG("doConnect: cleanup in progress, aborting");
        return;
    }

    BLOG_INFO("doConnect called in thread %p", QThread::currentThread());

    QUrl qurl(url);
    if (!qurl.isValid()) {
        BLOG_ERROR("Invalid WebSocket URL in doConnect");
        emit connectionStateChanged(false);
        emit errorOccurred("Invalid URL");
        return;
    }

    // If websocket is in a bad state, recreate it
    if (websocket_->state() == QAbstractSocket::ClosingState) {
        BLOG_DEBUG("WebSocket in closing state, waiting...");
        // Wait a bit for it to close
        QThread::msleep(100);
    }

    if (websocket_->state() != QAbstractSocket::UnconnectedState) {
        BLOG_WARNING("WebSocket not in unconnected state, aborting previous connection");
        websocket_->abort();
    }

    // Open connection from worker thread
    websocket_->open(qurl);
    BLOG_DEBUG("WebSocket open() called");
}

void WebSocketHandler::onConnected()
{
    if (cleanup_started_.load()) {
        return;
    }

    BLOG_INFO("WebSocket connected successfully");
    connected_.store(true);
    connection_attempted_.store(true);
    emit connectionStateChanged(true);
}

void WebSocketHandler::doDisconnect()
{
    BLOG_INFO("doDisconnect called in thread %p", QThread::currentThread());

    if (cleanup_started_.load()) {
        BLOG_DEBUG("doDisconnect: cleanup in progress, continuing with disconnect");
    }

    if (!websocket_) {
        BLOG_DEBUG("doDisconnect: websocket is null");
        emit connectionStateChanged(false);
        return;
    }

    // Block signals before calling close to prevent recursive calls
    websocket_->blockSignals(true);

    // Close the connection from worker thread
    if (websocket_->state() != QAbstractSocket::UnconnectedState) {
        BLOG_DEBUG("Closing WebSocket connection");
        websocket_->close();
    }

    // Unblock signals
    websocket_->blockSignals(false);

    connected_.store(false);
    emit connectionStateChanged(false);
}

void WebSocketHandler::onDisconnected()
{
    if (cleanup_started_.load()) {
        return;
    }

    BLOG_INFO("WebSocket disconnected");
    connected_.store(false);
    
    // Clear pending frames
    frame_queue_.clear();
    
    emit connectionStateChanged(false);
}

void WebSocketHandler::onTextMessageReceived(const QString& message)
{
    if (cleanup_started_.load() || !connected_.load()) {
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

void WebSocketHandler::onError(QAbstractSocket::SocketError error)
{
    if (cleanup_started_.load()) {
        return;
    }

    QString errorString;
    if (websocket_) {
        errorString = websocket_->errorString();
        BLOG_ERROR("WebSocket error %d: %s",
                   static_cast<int>(error),
                   errorString.toStdString().c_str());
    } else {
        errorString = "WebSocket is null";
        BLOG_ERROR("WebSocket error %d: WebSocket is null", static_cast<int>(error));
    }

    connected_.store(false);
    connection_attempted_.store(true);
    emit connectionStateChanged(false);
    emit errorOccurred(errorString);
}

void WebSocketHandler::handle_hello_message(const QJsonObject& json)
{
    if (cleanup_started_.load()) {
        return;
    }

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
    if (cleanup_started_.load()) {
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

    // Create VideoFrame with memory allocation error handling
    VideoFrame frame = {};
    try {
        frame.data = new uint8_t[decoded.size()];
        frame.size = decoded.size();
        frame.timestamp = timestamp;
        frame.pts = pts;
        frame.dts = dts;
        frame.is_keyframe = is_keyframe;

        memcpy(frame.data, decoded.data(), decoded.size());

        // Add to thread-safe queue (automatically handles size limit)
        frame_queue_.push(std::move(frame));
    } catch (const std::bad_alloc& e) {
        BLOG_ERROR("Failed to allocate memory for frame (%zu bytes): %s",
                   decoded.size(), e.what());
        // Clean up if allocation succeeded but something else failed
        if (frame.data) {
            delete[] frame.data;
            frame.data = nullptr;
        }
        // Skip this frame and continue streaming
        return;
    } catch (const std::exception& e) {
        BLOG_ERROR("Exception while processing frame: %s", e.what());
        // Clean up
        if (frame.data) {
            delete[] frame.data;
            frame.data = nullptr;
        }
        return;
    }

    frame_count_++;
    if (is_keyframe) {
        keyframe_count_++;
        BLOG_INFO("🔑 Keyframe #%d received (%zu bytes, seq=%d, queue=%zu)",
                  keyframe_count_, frame.size, sequence, frame_queue_.size());
    } else if (frame_count_ % 60 == 0) {
        BLOG_DEBUG("Frame #%d received (%zu bytes, seq=%d, queue=%zu)",
                   frame_count_, frame.size, sequence, frame_queue_.size());
    }
    
    // Log performance metrics every 300 frames (~10 seconds at 30fps)
    if (frame_count_ % 300 == 0) {
        BLOG_INFO("Performance: %d frames processed, %d keyframes, queue size: %zu",
                  frame_count_, keyframe_count_, frame_queue_.size());
    }
}

QByteArray WebSocketHandler::decode_base64(const QString& base64_str)
{
    // Qt handles base64 decoding
    return QByteArray::fromBase64(base64_str.toUtf8());
}

} // namespace berrystreamcam
