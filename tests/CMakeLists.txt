cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    message(STATUS "Google Test not found, tests will not be built")
    message(STATUS "To enable tests, install Google Test:")
    message(STATUS "  Ubuntu/Debian: sudo apt-get install libgtest-dev")
    message(STATUS "  Fedora: sudo dnf install gtest-devel")
    message(STATUS "  Arch: sudo pacman -S gtest")
    return()
endif()

message(STATUS "Building tests with Google Test")

# Enable testing
enable_testing()

# Include parent directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${LIBOBS_INCLUDE_DIRS})

# Unit tests for WebSocketHandler
add_executable(test_websocket_handler
    test_websocket_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/websocket-handler.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/frame-queue.cpp
)

target_link_libraries(test_websocket_handler
    GTest::GTest
    GTest::Main
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Test
    ${OBS_LIBRARIES}
)

# Integration tests
add_executable(test_integration
    test_integration.cpp
)

target_link_libraries(test_integration
    GTest::GTest
    GTest::Main
    Qt6::Core
    ${OBS_LIBRARIES}
)

# Add tests to CTest
add_test(NAME WebSocketHandlerTests COMMAND test_websocket_handler)
add_test(NAME IntegrationTests COMMAND test_integration)

# Set test properties
set_tests_properties(WebSocketHandlerTests PROPERTIES
    TIMEOUT 30
    LABELS "unit"
)

set_tests_properties(IntegrationTests PROPERTIES
    TIMEOUT 60
    LABELS "integration"
)

message(STATUS "Tests configured. Run with: ctest --output-on-failure")
