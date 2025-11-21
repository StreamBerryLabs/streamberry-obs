#pragma once

#include "../common.hpp"
#include "frame-queue.hpp"
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <string>
#include <memory>
#include <atomic>

namespace berrystreamcam {

class WebSocketHandler : public QObject {
    Q_OBJECT

public:
    WebSocketHandler();
    ~WebSocketHandler();

    bool connect_to_server(const std::string& url);
    void disconnect_from_server();
    bool is_connected() const;

    bool receive_frame(VideoFrame& frame);
    void process_events(); // Process Qt events in the streaming thread

signals:
    // Signals for cross-thread communication
    void connectRequested(const QString& url);
    void disconnectRequested();
    void frameReady(const VideoFrame& frame);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    // Slots executed in WebSocket worker thread
    void doConnect(const QString& url);
    void doDisconnect();
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);

private:
    void cleanup_websocket();
    void handle_video_frame(const QJsonObject& json);
    void handle_hello_message(const QJsonObject& json);
    QByteArray decode_base64(const QString& base64_str);

    QWebSocket* websocket_;              // Created in main thread, moved to worker
    QThread* worker_thread_;             // Dedicated thread for Qt event loop
    std::string url_;

    std::atomic<bool> connected_;
    std::atomic<bool> connection_attempted_;
    std::atomic<bool> cleanup_started_;
    FrameQueue frame_queue_;             // Thread-safe frame queue

    int frame_count_;
    int keyframe_count_;
};

} // namespace berrystreamcam
