#pragma once

#include "../common.hpp"
#include <queue>
#include <mutex>

namespace berrystreamcam {

/**
 * Thread-safe frame queue for transferring video frames between threads.
 * Automatically manages frame memory and enforces size limits.
 */
class FrameQueue {
public:
    FrameQueue();
    ~FrameQueue();

    // Disable copy and move
    FrameQueue(const FrameQueue&) = delete;
    FrameQueue& operator=(const FrameQueue&) = delete;
    FrameQueue(FrameQueue&&) = delete;
    FrameQueue& operator=(FrameQueue&&) = delete;

    /**
     * Push a frame to the queue.
     * If queue is full, oldest frame is deleted automatically.
     * Takes ownership of frame.data.
     */
    void push(VideoFrame&& frame);

    /**
     * Pop a frame from the queue.
     * Returns true if a frame was available, false if queue is empty.
     * Caller takes ownership of frame.data and must delete it.
     */
    bool pop(VideoFrame& frame);

    /**
     * Clear all frames from the queue.
     * Deletes all frame data.
     */
    void clear();

    /**
     * Get current queue size.
     * Thread-safe.
     */
    size_t size() const;

    /**
     * Check if queue is empty.
     * Thread-safe.
     */
    bool empty() const;

private:
    static constexpr size_t MAX_SIZE = 30;

    std::queue<VideoFrame> queue_;
    mutable std::mutex mutex_;
};

} // namespace berrystreamcam
