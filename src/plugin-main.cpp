/*
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                    BerryStreamCam OBS Plugin v1.0.0                      ║
 * ║                                                                          ║
 * ║  Transform your Android device into a professional wireless camera      ║
 * ║  for OBS Studio with ultra-low latency streaming                        ║
 * ║                                                                          ║
 * ║  Author:  Swadhin Biswas                                               ║
 * ║  GitHub:  https://github.com/StreamBerryLabs/streamberry-obs            ║
 * ║  License: GPL-2.0                                                       ║
 * ║                                                                          ║
 * ║  Features:                                                              ║
 * ║  • Auto-discovery of Streamberry devices                               ║
 * ║  • 5 streaming protocols (WebSocket, HTTP H.264, MJPEG, RTSP, DASH)   ║
 * ║  • Hardware-accelerated H.264 decoding                                 ║
 * ║  • Multi-device support                                                ║
 * ║  • Cross-platform (Linux, macOS, Windows)                              ║
 * ║                                                                          ║
 * ║  © 2025 Swadhin Biswas. All rights reserved.                           ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

#include <obs-module.h>
#include <obs-frontend-api.h>
#include "berrystreamcam-source.hpp"
#include "common.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("berrystreamcam", "en-US")

bool obs_module_load(void)
{
    BLOG_INFO("BerryStreamCam v1.0.0 loaded");
    BLOG_INFO("Auto-discovery enabled for Streamberry devices");

    // Register the source
    berrystreamcam::register_berrystreamcam_source();

    return true;
}

void obs_module_unload(void)
{
    BLOG_INFO("BerryStreamCam unloaded");
}

const char *obs_module_name(void)
{
    return "BerryStreamCam";
}

const char *obs_module_description(void)
{
    return "Auto-discover and stream from Streamberry Android app. "
           "Supports WebSocket, RTSP, HTTP (DASH/MJPEG/H.264) protocols.";
}
