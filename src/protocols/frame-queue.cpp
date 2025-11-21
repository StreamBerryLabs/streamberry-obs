#include "frame-queue.hpp"

namespace berrystreamcam {

FrameQueue::FrameQueue()
{
    BLOG_DEBUG("FrameQueue created");
}

FrameQueue::~FrameQueue()
{
    clear();
    BLOG_DEBUG("FrameQueue destroyed");
}

void FrameQueue::push(VideoFrame&& frame)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Enforce size limit - delete oldest frame if queue is full
    while (queue_.size() >= MAX_SIZE) {
        VideoFrame& old_frame = queue_.front();
        if (old_frame.data) {
            delete[] old_frame.data;
            old_frame.data = nullptr;
        }
        queue_.pop();
        BLOG_DEBUG("FrameQueue: Dropped old frame (queue full)");
    }

    // Add new frame to queue
    queue_.push(std::move(frame));
}

bool FrameQueue::pop(VideoFrame& frame)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty()) {
        return false;
    }

    frame = queue_.front();
    queue_.pop();

    return true;
}

void FrameQueue::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);

    while (!queue_.empty()) {
        VideoFrame& frame = queue_.front();
        if (frame.data) {
            delete[] frame.data;
            frame.data = nullptr;
        }
        queue_.pop();
    }

    BLOG_DEBUG("FrameQueue cleared");
}

size_t FrameQueue::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool FrameQueue::empty() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

} // namespace berrystreamcam
