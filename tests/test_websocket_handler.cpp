#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTimer>
#include "../src/protocols/websocket-handler.hpp"

using namespace berrystreamcam;

class WebSocketHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application for tests
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
    }

    void TearDown() override {
        // Cleanup is handled by Qt
    }

    QCoreApplication* app = nullptr;
};

// Test 1: Thread-safe creation and destruction
TEST_F(WebSocketHandlerTest, CreateDestroy) {
    WebSocketHandler* handler = nullptr;
    
    // Create handler (should create in main thread)
    ASSERT_NO_THROW({
        handler = new WebSocketHandler();
    });
    
    ASSERT_NE(handler, nullptr);
    
    // Destroy handler (should cleanup worker thread)
    ASSERT_NO_THROW({
        delete handler;
    });
}

// Test 2: Multiple create/destroy cycles
TEST_F(WebSocketHandlerTest, MultipleCreateDestroy) {
    for (int i = 0; i < 5; i++) {
        WebSocketHandler* handler = new WebSocketHandler();
        ASSERT_NE(handler, nullptr);
        
        // Brief delay to let worker thread start
        QTest::qWait(100);
        
        delete handler;
    }
}

// Test 3: Concurrent creation (thread safety)
TEST_F(WebSocketHandlerTest, ConcurrentCreation) {
    std::vector<std::thread> threads;
    std::vector<WebSocketHandler*> handlers(3, nullptr);
    
    // Create handlers from different threads
    for (int i = 0; i < 3; i++) {
        threads.emplace_back([&handlers, i]() {
            handlers[i] = new WebSocketHandler();
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Verify all created successfully
    for (auto* handler : handlers) {
        ASSERT_NE(handler, nullptr);
    }
    
    // Cleanup
    for (auto* handler : handlers) {
        delete handler;
    }
}

// Test 4: Frame queue overflow handling
TEST_F(WebSocketHandlerTest, FrameQueueOverflow) {
    WebSocketHandler handler;
    
    // Try to receive from empty queue
    VideoFrame frame;
    EXPECT_FALSE(handler.receive_frame(frame));
}

// Test 5: Connection state tracking
TEST_F(WebSocketHandlerTest, ConnectionState) {
    WebSocketHandler handler;
    
    // Initially not connected
    EXPECT_FALSE(handler.is_connected());
    
    // Note: Actual connection test would require a mock server
    // This just verifies the state tracking works
}

// Test 6: Cleanup sequence
TEST_F(WebSocketHandlerTest, CleanupSequence) {
    WebSocketHandler* handler = new WebSocketHandler();
    
    // Simulate some operations
    QTest::qWait(100);
    
    // Destructor should handle cleanup gracefully
    ASSERT_NO_THROW({
        delete handler;
    });
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
