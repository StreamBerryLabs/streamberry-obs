#pragma once

#include "../common.hpp"
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
#include <queue>
#include <mutex>
#include <condition_variable>

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

private:
    void cleanup_websocket();

private slots:
    void on_connected();
    void on_disconnected();
    void on_text_message_received(const QString& message);
    void on_error_occurred(QAbstractSocket::SocketError error);

private:
    void handle_video_frame(const QJsonObject& json);
    void handle_hello_message(const QJsonObject& json);
    QByteArray decode_base64(const QString& base64_str);

    std::unique_ptr<QWebSocket> websocket_;
    std::string url_;

    std::atomic<bool> connected_;
    std::atomic<bool> connection_attempted_;
    std::atomic<bool> cleanup_started_;
    std::queue<VideoFrame> frame_queue_;
    std::mutex queue_mutex_;
    std::condition_variable frame_cv_;

    int frame_count_;
    int keyframe_count_;
};

} // namespace berrystreamcam
