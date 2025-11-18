cmake_minimum_required(VERSION 3.16)

project(berrystreamcam VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find OBS Studio - Try system installation first, then source build
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBOBS QUIET libobs)
endif()

if(LIBOBS_FOUND)
    # Use system-installed OBS
    message(STATUS "✓ Using system-installed OBS")
    message(STATUS "  OBS Headers: ${LIBOBS_INCLUDE_DIRS}")
    message(STATUS "  OBS Libraries: ${LIBOBS_LIBRARY_DIRS}")
    include_directories(${LIBOBS_INCLUDE_DIRS} ${LIBOBS_INCLUDE_DIRS}/obs)
    link_directories(${LIBOBS_LIBRARY_DIRS})
    set(OBS_LIBRARIES obs)
else()
    # Fall back to source build
    message(STATUS "System OBS not found, using source build")

    if(NOT DEFINED OBS_SOURCE_DIR)
        message(FATAL_ERROR "OBS_SOURCE_DIR not defined. Please set it to your OBS Studio source directory.")
    endif()

    if(NOT DEFINED OBS_BUILD_DIR)
        message(FATAL_ERROR "OBS_BUILD_DIR not defined. Please set it to your OBS Studio build directory.")
    endif()

    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${OBS_SOURCE_DIR}/cmake/Modules/")

    include_directories(
        ${OBS_SOURCE_DIR}/libobs
        ${OBS_SOURCE_DIR}/UI/obs-frontend-api
        ${OBS_BUILD_DIR}/libobs
    )

    set(OBS_LIBRARIES obs)
endif()

# Find required libraries
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network WebSockets)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Find FFmpeg libraries
pkg_check_modules(LIBAVCODEC REQUIRED libavcodec)
pkg_check_modules(LIBAVUTIL REQUIRED libavutil)
pkg_check_modules(LIBSWSCALE REQUIRED libswscale)
pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)

include_directories(${LIBAVCODEC_INCLUDE_DIRS} ${LIBAVUTIL_INCLUDE_DIRS} ${LIBSWSCALE_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS})

# Plugin source files
set(PLUGIN_SOURCES
    src/plugin-main.cpp
    src/berrystreamcam-source.cpp
    src/discovery/device-discovery.cpp
    src/discovery/mdns-scanner.cpp
    src/protocols/websocket-handler.cpp
    src/protocols/rtsp-handler-ffmpeg.cpp
    src/protocols/http-handler.cpp
    src/decoder/h264-decoder.cpp
    src/ui/device-list-widget.cpp
)

set(PLUGIN_HEADERS
    src/berrystreamcam-source.hpp
    src/discovery/device-discovery.hpp
    src/discovery/mdns-scanner.hpp
    src/protocols/websocket-handler.hpp
    src/protocols/rtsp-handler.hpp
    src/protocols/http-handler.hpp
    src/decoder/h264-decoder.hpp
    src/ui/device-list-widget.hpp
    src/common.hpp
)

# Create the plugin
add_library(berrystreamcam MODULE
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

target_link_libraries(berrystreamcam
    ${OBS_LIBRARIES}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    ${LIBAVCODEC_LIBRARIES}
    ${LIBAVUTIL_LIBRARIES}
    ${LIBSWSCALE_LIBRARIES}
    ${LIBAVFORMAT_LIBRARIES}
)

# Set plugin properties
set_target_properties(berrystreamcam PROPERTIES
    PREFIX ""
    FOLDER "plugins"
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(berrystreamcam PRIVATE _WIN32)
elseif(APPLE)
    target_compile_definitions(berrystreamcam PRIVATE __APPLE__)
    set_target_properties(berrystreamcam PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "plugin"
    )
elseif(UNIX)
    target_compile_definitions(berrystreamcam PRIVATE __linux__)
endif()

# Installation
if(WIN32)
    install(TARGETS berrystreamcam
        RUNTIME DESTINATION "obs-plugins/64bit"
    )
elseif(APPLE)
    install(TARGETS berrystreamcam
        LIBRARY DESTINATION "$ENV{HOME}/Library/Application Support/obs-studio/plugins/"
    )
else()
    install(TARGETS berrystreamcam
        LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/obs-plugins"
    )
endif()
