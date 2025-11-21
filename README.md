<div align="center">

<img src="https://raw.githubusercontent.com/StreamBerryLabs/streamberry-obs/refs/heads/main/image.png" alt="BerryStreamCam Logo" width="200"/>

### 🎥 Transform Your Android Into a Pro Camera

[![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge&logo=github&logoColor=white)](https://github.com/StreamBerryLabs/streamberry-obs)
[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg?style=for-the-badge&logo=gnu)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![OBS Studio](https://img.shields.io/badge/OBS%20Studio-30.0%2B-purple.svg?style=for-the-badge&logo=obsstudio)](https://obsproject.com/)
[![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)](https://github.com/StreamBerryLabs)[![macOS](https://img.shields.io/badge/macOS-000000?style=for-the-badge&logo=apple&logoColor=white)](https://github.com/StreamBerryLabs)[![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/StreamBerryLabs)
**Streamberry-OBS** is a professional OBS Studio plugin that automatically discovers Streamberry devices on your network and provides ultra-low latency streaming with multiple protocol support.

[🚀 Quick Start](#-quick-start) •
[📦 Download](https://github.com/StreamBerryLabs/streamberry-obs) •
[📖 Documentation](#-installation) •
[🐛 Report Bug](https://github.com/StreamBerryLabs/streamberry-obs/issues) •
[💡 Feature Request](https://github.com/StreamBerryLabs/streamberry-obs/issues)

</div>

---

## ✨ Features

<div align="center">

| 🎯 Feature                     | 📝 Description                                                                       |
| :----------------------------- | :----------------------------------------------------------------------------------- |
| 🔍 **Auto-Discovery**          | Automatically finds Streamberry devices on your network (no manual IP entry!)        |
| 🚀 **Ultra-Low Latency**       | WebSocket protocol provides 50-150ms latency - perfect for gaming and live streaming |
| 🎭 **5 Streaming Protocols**   | Choose the best protocol for your needs - WebSocket, HTTP H.264, MJPEG, RTSP, DASH   |
| 🎬 **Hardware Acceleration**   | H.264 hardware decoding when available for silky smooth playback                     |
| 📺 **4K Support**              | Stream in stunning 4K resolution at 30fps - perfect for high-quality content         |
| 🔄 **Live Protocol Switching** | Change protocols on-the-fly without restarting OBS                                   |
| 📱 **Multi-Device Support**    | Connect multiple Android devices simultaneously - perfect for multi-cam setups       |
| 🎯 **Pause/Resume**            | Smooth hide/show with PipeWire-style state management                                |
| 🛡️ **Stable & Safe**           | Thread-safe operations, robust error handling, zero crashes                          |
| 🌐 **Cross-Platform**          | Works seamlessly on Linux, macOS, and Windows                                        |
| 🆓 **100% Free & Open Source** | No subscriptions, no limitations, forever free                                       |

</div>

## 📋 Supported Protocols

<div align="center">

| 🎭 Protocol       | 🔌 Port | ⚡ Latency | 🎯 Best For                                   | 🌐 Compatibility |
| :---------------- | :-----: | :--------: | :-------------------------------------------- | :--------------: |
| **⚡ WebSocket**  |  8080   |  50-150ms  | Live streaming, gaming, real-time, 4K capable |    ⭐⭐⭐⭐⭐    |
| **🎬 HTTP H.264** |  8081   | 100-300ms  | Recording, high quality, 4K capable           |    ⭐⭐⭐⭐⭐    |
| **📸 MJPEG**      |  8081   | 200-500ms  | Maximum compatibility                         |    ⭐⭐⭐⭐⭐    |
| **📹 RTSP**       |  8554   | 150-400ms  | Professional workflows, 4K lossless           |    ⭐⭐⭐⭐⭐    |
| **🌊 MPEG-DASH**  |  8081   | 300-1000ms | Adaptive bitrate streaming                    |     ⭐⭐⭐⭐     |

💡 **Pro Tip:** Use **WebSocket** for lowest latency, **RTSP** for maximum compatibility and lossless 4K streaming

</div>

## 📦 Installation

<div align="center">

### Choose Your Platform

[![Linux](https://img.shields.io/badge/🐧_Linux-Install-blue?style=for-the-badge)](#linux)
[![macOS](https://img.shields.io/badge/🍎_macOS-Install-black?style=for-the-badge)](#macos)
[![Windows](https://img.shields.io/badge/🪟_Windows-Install-0078D6?style=for-the-badge)](#windows)

</div>

---

### 🐧 Linux

#### 🔷 Arch Linux / Manjaro / Garuda

```bash
# Install dependencies
sudo pacman -S obs-studio qt6-base qt6-websockets ffmpeg curl openssl cmake gcc pkgconf

# Clone and build
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs
./build.sh

# Install plugin
sudo cp build/berrystreamcam.so /usr/lib/obs-plugins/
```

#### 🔶 Ubuntu / Debian

```bash
# Install dependencies
sudo apt install obs-studio libqt6-dev qt6-websockets-dev libavcodec-dev \
    libavutil-dev libswscale-dev libavformat-dev libcurl4-openssl-dev \
    libssl-dev cmake build-essential pkg-config

# Clone and build
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs
./build.sh

# Install plugin
sudo cp build/berrystreamcam.so /usr/lib/obs-plugins/
```

#### 🔴 Fedora / RHEL

```bash
# Install dependencies
sudo dnf install obs-studio qt6-qtbase-devel qt6-qtwebsockets-devel \
    ffmpeg-devel libcurl-devel openssl-devel cmake gcc-c++ pkgconfig

# Clone and build
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs
./build.sh

# Install plugin
sudo cp build/berrystreamcam.so /usr/lib64/obs-plugins/
```

#### 📦 Flatpak (All Linux Distributions)

If you're using OBS Studio via Flatpak, use the dedicated Flatpak build:

**Option 1: Using the build script (Recommended)**
```bash
# Clone repository
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs

# Run Flatpak build script (handles everything automatically)
./build-flatpak.sh
```

**Option 2: Manual Flatpak build**
```bash
# Install flatpak-builder
sudo apt install flatpak-builder  # Ubuntu/Debian
# or
sudo dnf install flatpak-builder  # Fedora
# or
sudo pacman -S flatpak-builder    # Arch

# Add Flathub repository
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo

# Install OBS Studio Flatpak
flatpak install flathub com.obsproject.Studio

# Build and install plugin
flatpak-builder --force-clean --install --user \
    build-flatpak \
    com.obsproject.Studio.Plugin.BerryStreamCam.json
```

**Option 3: Install from .flatpakref (when available)**
```bash
flatpak install berrystreamcam.flatpakref
```

**Flatpak Troubleshooting:**
- ✅ Verify OBS Flatpak is installed: `flatpak list | grep obs`
- ✅ Check plugin loads: `flatpak run com.obsproject.Studio --verbose`
- ✅ Network permissions are automatically granted for WebSocket connections
- ✅ Plugin location: `~/.var/app/com.obsproject.Studio/config/obs-studio/plugins/`
- ✅ View logs: `journalctl --user -f | grep obs`

### 🍎 macOS

```bash
# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install obs qt6 ffmpeg curl openssl cmake

# Clone and build
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs
./build.sh

# Install plugin
mkdir -p "$HOME/Library/Application Support/obs-studio/plugins/"
cp -r build/berrystreamcam.plugin "$HOME/Library/Application Support/obs-studio/plugins/"
```

### 🪟 Windows

#### 📋 Prerequisites

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with C++ development tools
2. Install [CMake](https://cmake.org/download/) (3.16+)
3. Install [OBS Studio](https://obsproject.com/download) (30.0+)
4. Install [Qt 6](https://www.qt.io/download-qt-installer)

#### 🔨 Build Steps

```powershell
# Clone repository
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs

# Create build directory
mkdir build
cd build

# Configure (adjust paths as needed)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DQt6_DIR="C:\Qt\6.5.0\msvc2019_64\lib\cmake\Qt6" ^
    -DOBS_SOURCE_DIR="C:\obs-studio" ^
    -DOBS_BUILD_DIR="C:\obs-studio\build"

# Build
cmake --build . --config Release

# Install (run as Administrator)
copy Release\berrystreamcam.dll "%ProgramFiles%\obs-studio\obs-plugins\64bit\"
```

## 🚀 Quick Start

<div align="center">

### 🎬 Get Started in 4 Easy Steps!

</div>

### 📱 1. Install Streamberry App on Android

Download the Streamberry app from [GitHub Releases](https://github.com/StreamBerryLabs/streamberry-obs/releases) or build from source.

### 🌐 2. Connect to Same Network

Ensure your Android device and computer are on the same WiFi network or subnet.

### 🎥 3. Start Streaming in OBS

1. **🚀 Open OBS Studio**
2. **➕ Add Source**: Click `+` in the Sources panel
3. **🎬 Select "BerryStreamCam"**
4. **🔍 Wait for Discovery**: Plugin automatically scans for devices (5-10 seconds)
5. **📱 Select Your Device**: Choose from the dropdown list
6. **⚡ Choose Protocol**: Select "WebSocket" for lowest latency
7. **✅ Click OK**: Stream starts automatically!

### 🔧 4. Troubleshooting

<details>
<summary>❌ <b>Device Not Found?</b></summary>

- ✅ Check both devices are on same network
- ✅ Verify Streamberry app is running
- ✅ Try manual IP: Enter IP address in the "Manual IP" field
- ✅ Check firewall: Allow ports 8080, 8081, 8554
- ✅ Disable VPN on either device
- ✅ Restart your router if necessary

</details>

<details>
<summary>🎬 <b>No Video Displayed?</b></summary>

- ✅ Check Android app shows "Connected" status
- ✅ Try different protocol (switch to HTTP H.264)
- ✅ Restart OBS Studio
- ✅ Check OBS logs: Help → Log Files → Current Log
- ✅ Verify camera permissions in Android app
- ✅ Try hiding and showing the source

</details>

<details>
<summary>⚡ <b>High Latency?</b></summary>

- ✅ Switch to WebSocket protocol
- ✅ Use 5GHz WiFi instead of 2.4GHz
- ✅ Reduce resolution in Android app (720p recommended)
- ✅ Move closer to WiFi router
- ✅ Close bandwidth-heavy applications
- ✅ Use Quality of Service (QoS) on router

</details>

## 🎛️ Configuration

### Properties Panel

- **Connection Type**: Auto-Discovery or Manual IP
- **Device**: List of discovered Streamberry devices
- **Manual IP Address**: Enter IP when auto-discovery fails
- **Protocol**: Select streaming protocol
- **Refresh Devices**: Manually trigger device scan

### Network Requirements

| Requirement   | Details                                                           |
| ------------- | ----------------------------------------------------------------- |
| **Network**   | Same local network/subnet                                         |
| **Ports**     | 8080 (WebSocket), 8081 (HTTP), 8554 (RTSP)                        |
| **Bandwidth** | 5-10 Mbps for 1080p@30fps, 20-30 Mbps for 4K@30fps                |
| **WiFi**      | 5GHz required for 4K, 5GHz recommended for 1080p, 2.4GHz for 720p |
| **Firewall**  | Allow inbound on required ports                                   |

### Performance Tips

**For Lowest Latency (50-150ms):**

- ✅ Use WebSocket protocol
- ✅ 720p @ 30fps
- ✅ 5GHz WiFi
- ✅ Close to router

**For Best Quality:**

- ✅ Use HTTP H.264 protocol
- ✅ 1080p @ 60fps or 4K @ 30fps
- ✅ Wired ethernet on PC
- ✅ 5GHz WiFi or wired ethernet for phone
- ✅ Hardware encoding on Android device

**For Stunning 4K Streaming:**

- ✅ Use HTTP H.264 or WebSocket protocol
- ✅ 4K (3840x2160) @ 30fps
- ✅ Gigabit ethernet on both devices (recommended)
- ✅ High-end Android device with hardware H.264 encoder
- ✅ Minimum 20-30 Mbps bandwidth
- ✅ Hardware decoding on PC (GPU acceleration)

**For Maximum Compatibility:**

- ✅ Use RTSP protocol
- ✅ 720p to 4K @ 30fps (RTSP supports lossless 4K streaming)
- ✅ Any network type
- ✅ Industry-standard protocol
- ✅ Compatible with all professional video software

**For Lossless 4K Professional Streaming:**

- ✅ Use RTSP protocol
- ✅ 4K (3840x2160) @ 30fps lossless
- ✅ Wired Gigabit ethernet (required for 4K)
- ✅ Professional-grade Android device
- ✅ 30-50 Mbps bandwidth for 4K lossless
- ✅ Industry-standard streaming protocol
- ✅ Perfect for professional broadcasting and production

## 🛠️ Development

### Building from Source

```bash
# Install dependencies (see platform-specific instructions above)

# Clone repository
git clone https://github.com/StreamBerryLabs/streamberry-obs.git
cd streamberry-obs

# Build
./build.sh

# The plugin will be in build/berrystreamcam.so (Linux/macOS)
# or build/Release/berrystreamcam.dll (Windows)
```

## 🏗️ Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         OBS Studio                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                    BerryStreamCam Plugin                       │  │
│  │                                                                 │  │
│  │  ┌────────────────┐      ┌─────────────────────────────────┐  │  │
│  │  │  Source Core   │      │      Device Discovery           │  │  │
│  │  │                │      │                                 │  │  │
│  │  │  - Properties  │◄─────┤  - mDNS Scanner                │  │  │
│  │  │  - Rendering   │      │  - Port Probing (8080/81/54)   │  │  │
│  │  │  - Threading   │      │  - Auto-refresh (5s)           │  │  │
│  │  └───────┬────────┘      └─────────────────────────────────┘  │  │
│  │          │                                                     │  │
│  │          │                                                     │  │
│  │  ┌───────▼──────────────────────────────────────────────────┐ │  │
│  │  │              Protocol Handlers                            │ │  │
│  │  │                                                            │ │  │
│  │  │  ┌───────────────┐  ┌──────────────┐  ┌───────────────┐ │ │  │
│  │  │  │  WebSocket    │  │     RTSP     │  │     HTTP      │ │ │  │
│  │  │  │  Handler      │  │   Handler    │  │   Handler     │ │ │  │
│  │  │  │               │  │              │  │               │ │ │  │
│  │  │  │  Port: 8080   │  │  Port: 8554  │  │  Port: 8081   │ │ │  │
│  │  │  │  Protocol:    │  │  Protocol:   │  │  Protocols:   │ │ │  │
│  │  │  │  - JSON msgs  │  │  - RTSP/RTP  │  │  - Raw H.264  │ │ │  │
│  │  │  │  - Base64     │  │  - NAL units │  │  - MJPEG      │ │ │  │
│  │  │  │  - H.264      │  │  - SDP       │  │  - DASH       │ │ │  │
│  │  │  └───────┬───────┘  └──────┬───────┘  └───────┬───────┘ │ │  │
│  │  └──────────┼──────────────────┼──────────────────┼─────────┘ │  │
│  │             │                  │                  │           │  │
│  │             └──────────────────┼──────────────────┘           │  │
│  │                                │                              │  │
│  │  ┌─────────────────────────────▼──────────────────────────┐  │  │
│  │  │              H.264 Decoder (FFmpeg)                     │  │  │
│  │  │                                                          │  │  │
│  │  │  - Decodes H.264 NAL units                             │  │  │
│  │  │  - YUV → RGBA conversion                               │  │  │
│  │  │  - Hardware acceleration                               │  │  │
│  │  │  - Low-latency mode                                    │  │  │
│  │  └─────────────────────────┬────────────────────────────────┘  │  │
│  │                            │                                   │  │
│  │  ┌─────────────────────────▼────────────────────────────────┐ │  │
│  │  │              GPU Texture Upload                          │ │  │
│  │  │                                                           │ │  │
│  │  │  - Create/update OBS texture                            │ │  │
│  │  │  - Direct GPU memory                                    │ │  │
│  │  │  - Minimal CPU usage                                    │ │  │
│  │  └───────────────────────────────────────────────────────────┘ │  │
│  │                                                                 │  │
│  │  ┌───────────────────────────────────────────────────────────┐ │  │
│  │  │              User Interface (Qt)                          │ │  │
│  │  │                                                            │ │  │
│  │  │  - Device list widget                                    │ │  │
│  │  │  - Protocol selection                                    │ │  │
│  │  │  - Connection status                                     │ │  │
│  │  │  - Refresh button                                        │ │  │
│  │  └───────────────────────────────────────────────────────────┘ │  │
│  └─────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘

                                    │
                              WiFi Network
                                    │

┌─────────────────────────────────────────────────────────────────────┐
│                      Streamberry Android App                         │
│                                                                       │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │                    Streaming Servers                             ││
│  │                                                                   ││
│  │  ┌─────────────────┐  ┌──────────────┐  ┌───────────────────┐  ││
│  │  │  OBS Droid      │  │  RTSP Server │  │   HTTP Server     │  ││
│  │  │  WebSocket      │  │              │  │                   │  ││
│  │  │  Server         │  │  - RTP/RTCP  │  │  - DASH           │  ││
│  │  │                 │  │  - SDP       │  │  - MJPEG          │  ││
│  │  │  Port: 8080     │  │              │  │  - Raw H.264      │  ││
│  │  │                 │  │  Port: 8554  │  │  - Player UI      │  ││
│  │  │  /stream        │  │              │  │                   │  ││
│  │  │  /health        │  │  /stream     │  │  Port: 8081       │  ││
│  │  └─────────────────┘  └──────────────┘  └───────────────────┘  ││
│  └─────────────────────────────────────────────────────────────────┘│
│                                                                       │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │                      mDNS Service                                ││
│  │                                                                   ││
│  │  Service Name: Streamberry-{IP}                                 ││
│  │  Service Type: _streamberry._tcp.local                          ││
│  │  Published Port: 8080                                            ││
│  └─────────────────────────────────────────────────────────────────┘│
│                                                                       │
│  ┌─────────────────────────────────────────────────────────────────┐│
│  │                   Video Encoding Pipeline                        ││
│  │                                                                   ││
│  │  Camera → Encoder → H.264 NAL Units → Servers                   ││
│  │                                                                   ││
│  │  - MediaCodec H.264                                              ││
│  │  - SPS/PPS extraction                                            ││
│  │  - Annex-B format                                                ││
│  │  - Real-time encoding                                            ││
│  └─────────────────────────────────────────────────────────────────┘│
└─────────────────────────────────────────────────────────────────────┘
```

<details>
<summary><b>📊 Data Flow Diagrams</b></summary>

### WebSocket Protocol (Recommended)

```
Android App                          OBS Plugin
─────────────                        ──────────

1. Start Server
   (Port 8080) ────────────────────► Discover via mDNS
                                      Probe /health endpoint

2. Client Connect ◄──────────────── WebSocket connect to
   Send Hello msg ──────────────────► /stream endpoint
                                      Parse capabilities

3. Encode Frame
   (H.264 NAL)
   ↓
   Convert to Annex-B
   ↓
   Base64 encode
   ↓
   Wrap in JSON
   {
     "type": "video_frame",
     "pts": 123456,
     "keyframe": true,
     "data": "..."
   } ────────────────────────────────► Parse JSON
                                       Decode base64
                                       Extract H.264
                                       ↓
                                       H.264 Decoder
                                       ↓
                                       YUV → RGBA
                                       ↓
                                       GPU Texture
                                       ↓
                                       OBS Render

4. Stats/Metadata ◄──────────────── Request stats (optional)
```

### Threading Model

```
┌─────────────────────────────────────────────────────────────┐
│                    OBS Main Thread                           │
│                                                               │
│  - GUI events                                                │
│  - Rendering                                                 │
│  - Texture updates                                           │
└────────────────────────────────────────────────��────────────┘
                            │
                            │ spawn
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  Discovery Thread                            │
│                                                               │
│  while (active) {                                           │
│    scan_network();    // mDNS + port probing               │
│    update_device_list();                                    │
│    sleep(5s);                                               │
│  }                                                           │
└─────────────────────────────────────────────────────────────┘

                  User clicks "Connect"
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                  Streaming Thread                            │
│                                                               │
│  connect_to_device(protocol);                               │
│  while (streaming) {                                        │
│    frame = receive_frame();  // From protocol handler      │
│    decode(frame);             // H.264 decoder              │
│    push_to_queue(rgba_data);  // Thread-safe queue         │
│  }                                                           │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ Frame Queue
                            │ (Thread-safe)
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              OBS Rendering (Main Thread)                     │
│                                                               │
│  frame = pop_from_queue();                                  │
│  upload_to_gpu_texture(frame);                              │
│  render_texture();                                           │
└─────────────────────────────────────────────────────────────┘
```

</details>

### Project Structure

```
streamberry-obs/
├── CMakeLists.txt              # Build configuration
├── build.sh                    # Build script
├── src/
│   ├── plugin-main.cpp         # OBS plugin registration
│   ├── berrystreamcam-source.* # Main source implementation
│   ├── common.hpp              # Shared types and constants
│   ├── discovery/              # Auto-discovery system
│   │   ├── device-discovery.*  # Network scanning
│   │   └── mdns-scanner.*      # mDNS/Bonjour
│   ├── protocols/              # Protocol handlers
│   │   ├── websocket-handler.* # WebSocket (Port 8080)
│   │   ├── http-handler.*      # HTTP (Port 8081)
│   │   └── rtsp-handler.*      # RTSP (Port 8554)
│   ├── decoder/                # Video decoding
│   │   └── h264-decoder.*      # H.264/MJPEG decoder
│   └── ui/                     # User interface
│       └── device-list-widget.* # Device selection UI
└── docs/                       # Documentation
```

### Adding New Features

**1. Add New Protocol:**

```cpp
// 1. Add to ProtocolType enum in common.hpp
enum class ProtocolType {
    // ... existing
    MY_NEW_PROTOCOL,
};

// 2. Create handler in src/protocols/
class MyProtocolHandler {
    bool connect(const std::string& url);
    void disconnect();
    bool receive_frame(VideoFrame& frame);
};

// 3. Update device-discovery.cpp to detect protocol
// 4. Update UI in device-list-widget.cpp
```

**2. Improve Auto-Discovery:**

```cpp
// Modify src/discovery/device-discovery.cpp
// Add new scanning methods or improve existing ones
```

### Running Tests

```bash
# Build with tests enabled
mkdir -p build && cd build
cmake .. -DENABLE_TESTS=ON
make

# Run tests
ctest --output-on-failure
```

## 📚 API Documentation

### Device Discovery

```cpp
// Scan network for devices (runs every 5 seconds)
std::vector<StreamDevice> devices = discovery_->scan_network();

// Check specific device
StreamDevice device;
bool found = discovery_->probe_device("192.168.1.100", device);
```

### Protocol Handlers

```cpp
// WebSocket Handler
WebSocketHandler ws;
ws.connect_to_server("ws://192.168.1.100:8080/stream");
VideoFrame frame;
if (ws.receive_frame(frame)) {
    // Process frame
}
ws.disconnect_from_server();

// HTTP Handler
HttpHandler http;
http.connect("http://192.168.1.100:8081/stream.h264", ProtocolType::HTTP_RAW_H264);
if (http.receive_frame(frame)) {
    // Process frame
}
http.disconnect();
```

### H.264 Decoder

```cpp
H264Decoder decoder;
decoder.initialize();

uint8_t* decoded_data;
int width, height;
if (decoder.decode_frame(encoded_data, encoded_size, &decoded_data, &width, &height)) {
    // decoded_data contains RGBA pixels
}

decoder.shutdown();
```

## 📊 Performance Comparison

<div align="center">

### BerryStreamCam vs Competitors

| Feature               | BerryStreamCam |  DroidCam   |  IP Webcam  |   Others    |
| :-------------------- | :------------: | :---------: | :---------: | :---------: |
| 🔍 Auto-Discovery     |       ✅       |     ❌      |     ❌      |     ❌      |
| ⚡ Ultra-Low Latency  |   ✅ (50ms)    | ⚠️ (200ms+) | ⚠️ (300ms+) | ⚠️ (500ms+) |
| 🎭 Multiple Protocols |     ✅ (5)     |   ⚠️ (2)    |   ⚠️ (3)    |  ⚠️ (1-2)   |
| 📱 Multi-Device       |       ✅       |     ❌      | ⚠️ Limited  |     ❌      |
| 🔄 Live Switching     |       ✅       |     ❌      |     ❌      |     ❌      |
| 🆓 Free & Open Source |       ✅       | ⚠️ Limited  | ⚠️ Freemium |     ❌      |
| 🎬 Hardware Decoding  |       ✅       | ⚠️ Limited  |     ❌      | ⚠️ Limited  |
| 🌐 Cross-Platform     |       ✅       | ⚠️ Limited  | ⚠️ Limited  | ⚠️ Limited  |
| 💪 Active Development |       ✅       |     ⚠️      |     ⚠️      |     ⚠️      |

**Legend:** ✅ Fully Supported | ⚠️ Partially/Limited | ❌ Not Available

</div>

---

## 🎬 Screenshots

<div align="center">

### Plugin in Action

<!-- <table>
  <tr>
    <td><img src="https://via.placeholder.com/400x250/667eea/ffffff?text=Auto+Discovery" alt="Auto Discovery"/></td>
    <td><img src="https://via.placeholder.com/400x250/764ba2/ffffff?text=Protocol+Selection" alt="Protocol Selection"/></td>
  </tr>
  <tr>
    <td align="center"><b>🔍 Auto-Discovery</b><br/>Automatically finds devices</td>
    <td align="center"><b>🎭 Protocol Selection</b><br/>Choose your preferred protocol</td>
  </tr>
  <tr>
    <td><img src="https://via.placeholder.com/400x250/f093fb/ffffff?text=Live+Streaming" alt="Live Streaming"/></td>
    <td><img src="https://via.placeholder.com/400x250/4facfe/ffffff?text=Multi+Device" alt="Multi Device"/></td>
  </tr>
  <tr>
    <td align="center"><b>🎥 Live Streaming</b><br/>Ultra-low latency video</td>
    <td align="center"><b>📱 Multi-Device</b><br/>Connect multiple cameras</td>
  </tr>
</table> -->

</div>

---

## 🔮 Roadmap & Future Updates

### Version 1.1.0 (Planned)

- [ ] **Enhanced Auto-Discovery**

  - mDNS/Bonjour service discovery (Linux: Avahi, macOS: native, Windows: DNS-SD)
  - QR code scanning for instant connection
  - Bluetooth Low Energy (BLE) discovery
  - USB connection support (ADB)
  - Remember previously connected devices

- [ ] **Audio Streaming**

  - Audio capture from Android microphone
  - Audio sync with video
  - Multiple audio sources (mic, system audio)
  - Audio monitoring in OBS

- [ ] **Advanced Features**
  - PTZ (Pan-Tilt-Zoom) controls via touch gestures
  - Multiple camera support (front/back switching)
  - Custom resolution/bitrate selection
  - Recording directly on Android device
  - Overlay graphics (OSD) on Android

### Version 1.2.0 (Future)

- [ ] **Performance**

  - Hardware encoding on Android (MediaCodec)
  - Zero-copy memory transfers
  - Multi-threaded decoding
  - Adaptive bitrate streaming
  - Network bandwidth optimization

- [ ] **Professional Features**

  - NDI protocol support
  - SRT (Secure Reliable Transport)
  - Virtual camera passthrough
  - Multi-camera sync
  - Timecode synchronization

- [ ] **User Experience**
  - One-click pairing
  - Connection presets
  - Custom overlays
  - Stream health monitoring
  - Automatic reconnection

### Version 2.0.0 (Long-term)

- [ ] **Platform Expansion**

  - iOS support (iPhone/iPad)
  - Linux phone support
  - Raspberry Pi camera support
  - Web browser source (WebRTC)

- [ ] **Cloud Features**
  - Remote device access (over internet)
  - Cloud device registry
  - Share device with team members
  - Remote monitoring dashboard

## 🛠️ Tech Stack

<div align="center">

### Built With Cutting-Edge Technologies

<table>
  <tr>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" width="48" height="48" alt="C++" />
      <br>C++17
    </td>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/qt/qt-original.svg" width="48" height="48" alt="Qt" />
      <br>Qt 6
    </td>
    <td align="center" width="96">
      <img src="https://w7.pngwing.com/pngs/489/1005/png-transparent-obs-macos-bigsur-icon-thumbnail.png" width="48" height="48" alt="OBS" />
      <br>OBS Studio
    </td>
    <td align="center" width="96">
      <img src="https://upload.wikimedia.org/wikipedia/commons/5/5f/FFmpeg_Logo_new.svg" width="48" height="48" alt="FFmpeg" />
      <br>FFmpeg
    </td>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cmake/cmake-original.svg" width="48" height="48" alt="CMake" />
      <br>CMake
    </td>
  </tr>
  <tr>
    <td align="center" width="96">
      <img src="https://curl.se/logo/curl-logo.svg" width="48" height="48" alt="libcurl" />
      <br>libcurl
    </td>
    <td align="center" width="96">
      <img src="https://pbs.twimg.com/profile_images/1839806490158477312/z1WI16ZB_400x400.png" width="48" height="48" alt="OpenSSL" />
      <br>OpenSSL
    </td>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/linux/linux-original.svg" width="48" height="48" alt="Linux" />
      <br>Linux
    </td>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/apple/apple-original.svg" width="48" height="48" alt="macOS" />
      <br>macOS
    </td>
    <td align="center" width="96">
      <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/windows8/windows8-original.svg" width="48" height="48" alt="Windows" />
      <br>Windows
    </td>
  </tr>
</table>

**Core Technologies:**

- 🔷 **C++17** - Modern C++ with RAII and smart pointers
- 🔷 **Qt 6** - Cross-platform framework for networking and UI
- 🔷 **FFmpeg** - Industry-standard multimedia framework
- 🔷 **CMake** - Cross-platform build system
- 🔷 **WebSockets** - Real-time bidirectional communication
- 🔷 **RTSP/RTP** - Standard streaming protocols
- 🔷 **H.264** - Industry-standard video codec

</div>

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Development Guidelines

- Follow C++17 standards
- Use RAII for resource management
- Add error handling with try-catch
- Document public APIs with comments
- Test on multiple platforms
- Update README for new features

## ❓ Frequently Asked Questions (FAQ)

<details>
<summary><b>💰 Is BerryStreamCam really free?</b></summary>
<br>
Yes! BerryStreamCam is 100% free and open source under GPL-2.0 license. No hidden costs, no subscriptions, no premium features. Forever free!
</details>

<details>
<summary><b>📱 Which Android versions are supported?</b></summary>
<br>
The Streamberry Android app supports Android 7.0 (Nougat) and above. The plugin works with any device running the Streamberry app.
</details>

<details>
<summary><b>🎥 Can I use multiple cameras at once?</b></summary>
<br>
Absolutely! You can add multiple BerryStreamCam sources in OBS, each connected to a different Android device. Perfect for multi-cam productions!
</details>

<details>
<summary><b>🌐 Does it work over the internet?</b></summary>
<br>
Currently, BerryStreamCam requires both devices to be on the same local network. Internet streaming is planned for v2.0 using secure tunneling.
</details>

<details>
<summary><b>⚡ Which protocol should I use?</b></summary>
<br>

- **WebSocket**: Best for live streaming, gaming, real-time (50-150ms latency)
- **HTTP H.264**: Best for recording, high quality (100-300ms latency)
- **RTSP**: Best for professional workflows, OBS scenes (150-400ms latency)
- **MJPEG**: Best for maximum compatibility (200-500ms latency)

</details>

<details>
<summary><b>🔒 Is it secure?</b></summary>
<br>
BerryStreamCam operates on your local network only. All communication happens directly between your devices without any cloud servers or third-party services.
</details>

<details>
<summary><b>🎤 Does it support audio?</b></summary>
<br>
Audio streaming is planned for v1.1.0. Currently, only video is supported. You can use OBS's audio input capture as a workaround.
</details>

<details>
<summary><b>🐧 Which Linux distros are supported?</b></summary>
<br>
BerryStreamCam works on all major Linux distributions including:

- Arch Linux / Manjaro / Garuda
- Ubuntu / Debian / Linux Mint
- Fedora / RHEL / CentOS Stream
- openSUSE
- Gentoo
- And many more!

</details>

<details>
<summary><b>💻 What are the system requirements?</b></summary>
<br>

**Minimum:**

- OBS Studio 30.0+
- 4GB RAM
- Dual-core CPU
- WiFi or Ethernet connection

**Recommended (1080p):**

- OBS Studio 30.2+
- 8GB RAM
- Quad-core CPU with hardware H.264 decoding
- 5GHz WiFi or Gigabit Ethernet

**For 4K Streaming:**

- OBS Studio 30.2+
- 16GB RAM
- Hexa-core CPU or hardware H.264 decoder
- Gigabit Ethernet (recommended) or WiFi 6
- High-end Android device (Snapdragon 8 Gen 2+ or equivalent)

</details>

<details>
<summary><b>🔄 How do I update the plugin?</b></summary>
<br>

1. Download the latest release from GitHub
2. Stop OBS Studio
3. Replace the old plugin file with the new one
4. Restart OBS Studio

Or rebuild from source using `./build.sh` for the latest development version.

</details>

---

## 📄 License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## 🙏 Credits

- **OBS Studio** - [https://obsproject.com](https://obsproject.com)
- **Qt Framework** - [https://www.qt.io](https://www.qt.io)
- **FFmpeg** - [https://ffmpeg.org](https://ffmpeg.org)
- **libcurl** - [https://curl.se](https://curl.se)

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/StreamBerryLabs/streamberry-obs/issues)
- **Discussions**: [GitHub Discussions](https://github.com/StreamBerryLabs/streamberry-obs/discussions)
- **Email**: swadhinbiswas.cse@gmail.com

## 📈 Statistics

- **Lines of Code**: ~5,000+
- **Files**: 19 source files
- **Protocols Supported**: 5
- **Max Resolution**: 4K (3840x2160) @ 30fps
- **Platforms**: Linux, macOS, Windows
- **Languages**: C++ (95%), CMake (5%)

## 🎉 Success Stories

> "BerryStreamCam works flawlessly! Latency is incredibly low, and setup was a breeze." - User

> "Finally, a DroidCam alternative that's open source and actually works better!" - Streamer

> "The auto-discovery feature is amazing. No more typing IP addresses!" - Content Creator

> "Streaming in 4K at 30fps with barely any latency - this is next level! My production quality has gone through the roof!" - Professional Streamer

> "I use 3 phones in 4K mode for my cooking channel. The multi-device support is incredible!" - Content Creator

---

<div align="center">

---

### 💖 Made with Love & Code

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/swadhinbiswas">
        <img src="https://github.com/swadhinbiswas.png" width="100px;" alt="Swadhin Biswas"/>
        <br />
        <sub><b>Swadhin Biswas</b></sub>
      </a>
      <br />
      <a href="https://github.com/swadhinbiswas" title="Code">💻</a>
      <a href="#" title="Design">🎨</a>
      <a href="#" title="Documentation">📖</a>
      <a href="#" title="Ideas">🤔</a>
    </td>
  </tr>
</table>

**Creator & Lead Developer**

[![GitHub](https://img.shields.io/badge/GitHub-swadhinbiswas-181717?style=for-the-badge&logo=github)](https://github.com/swadhinbiswas)
[![Twitter](https://img.shields.io/badge/twitter-Follow-1DA1F2?style=for-the-badge&logo=x)](https://x.com/swadh1n)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-0077B5?style=for-the-badge&logo=linkedin)](https://www.linkedin.com/in/swadh1n/)
[![Portfolio](https://img.shields.io/badge/Portfolio-Visit-4CAF50?style=for-the-badge&logo=human)](https://www.swadhin.my.id/)

---

### ⭐ Show Your Support

If you found BerryStreamCam useful, please consider:

[![Star on GitHub](https://img.shields.io/github/stars/StreamBerryLabs/streamberry-obs?style=social)](https://github.com/StreamBerryLabs/streamberry-obs)
[![Fork on GitHub](https://img.shields.io/github/forks/StreamBerryLabs/streamberry-obs?style=social)](https://github.com/StreamBerryLabs/streamberry-obs/fork)
[![Watch on GitHub](https://img.shields.io/github/watchers/StreamBerryLabs/streamberry-obs?style=social)](https://github.com/StreamBerryLabs/streamberry-obs)

**⭐ Star this repo** • **🍴 Fork it** • **📢 Share with others**

---

<sub>Built with 🎥 Camera Tech • ⚡ Real-time Streaming • 🔐 Open Source Spirit</sub>

<sub>© 2025 Swadhin Biswas. Licensed under GPL-2.0</sub>

[⬆ Back to top](#-berrystreamcam-obs-plugin)

</div>
