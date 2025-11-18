# BerryStreamCam OBS Plugin - Quick Start Guide

## What is BerryStreamCam?

BerryStreamCam is an OBS Studio plugin that automatically discovers your Streamberry Android device and adds it as a video source in OBS, just like DroidCam!

## Features

✨ **Auto-Discovery**: Finds your phone automatically on the network
📱 **Multiple Protocols**: WebSocket, RTSP, HTTP (MJPEG, H.264, DASH)
⚡ **Low Latency**: Optimized for real-time streaming
🎥 **High Quality**: Supports up to 4K streaming
🔌 **Simple Setup**: Just connect and stream!

## Quick Start

### 1. Build the Plugin

```bash
cd streamberry-obs

# Set OBS directories
export OBS_SOURCE_DIR=/path/to/obs-studio
export OBS_BUILD_DIR=/path/to/obs-studio/build

# Build and install
./build.sh install
```

### 2. Start Streamberry App

Open the Streamberry app on your Android device and start streaming.

### 3. Add Source in OBS

1. Open OBS Studio
2. Click "+" in Sources panel
3. Select "BerryStreamCam"
4. Your device will appear automatically!
5. Select it and click "Connect"

## Protocols Explained

### WebSocket (Recommended) ⚡

- **Port**: 8080
- **Best for**: Lowest latency, real-time streaming
- **Quality**: Excellent
- **Use when**: You want the fastest, most responsive stream

### RTSP 📹

- **Port**: 8554
- **Best for**: Standard compatibility
- **Quality**: Good
- **Use when**: You need a standard protocol

### HTTP Raw H.264 🎬

- **Port**: 8081
- **Best for**: Direct H.264 streaming
- **Quality**: Good
- **Use when**: You want simple HTTP streaming

### MJPEG 📸

- **Port**: 8081
- **Best for**: Maximum compatibility
- **Quality**: Good (higher bandwidth)
- **Use when**: Other protocols don't work

## Network Setup

Both devices must be on the same WiFi network:

```
[Streamberry App]  ←→  WiFi Router  ←→  [OBS Computer]
   (Android)                              (Windows/Mac/Linux)
```

### Port Requirements

The plugin connects to these ports on your Android device:

- **8080**: WebSocket (OBS Droid protocol)
- **8081**: HTTP server (MJPEG, H.264, DASH)
- **8554**: RTSP server

Make sure no firewall is blocking these ports!

## Troubleshooting

### "No devices found"

**Check:**

- ✓ Streamberry app is running
- ✓ Both devices on same WiFi network
- ✓ Click "Refresh Devices" button
- ✓ Check phone shows "Streaming" status

**Fix:**

```bash
# On Android, check IP address in Streamberry app
# In OBS, wait 5-10 seconds for auto-discovery
# Or manually add device by IP
```

### "Connection failed"

**Check:**

- ✓ Selected protocol is available (shown in device list)
- ✓ No firewall blocking ports
- ✓ WiFi is stable

**Fix:**

- Try different protocol (switch to WebSocket)
- Restart Streamberry app
- Move closer to WiFi router

### "Poor quality or lag"

**Fix:**

- ✅ Use WebSocket protocol (lowest latency)
- ✅ Switch to 5GHz WiFi (much faster than 2.4GHz)
- ✅ Reduce resolution in Streamberry app settings
- ✅ Close other apps using network
- ✅ Move closer to WiFi router

## Advanced Usage

### Manual Device Entry

If auto-discovery doesn't work, you can manually enter the IP:

1. Find IP in Streamberry app (shown on screen)
2. In OBS, add BerryStreamCam source
3. Enter IP address manually
4. Select protocol
5. Connect

### Multiple Devices

You can add multiple Streamberry devices:

1. Add multiple BerryStreamCam sources
2. Each will show in device list
3. Connect to different devices
4. Use different protocols if needed

### Protocol Switching

Switch protocols without restarting:

1. Disconnect current stream
2. Select different protocol from dropdown
3. Click Connect
4. Stream continues with new protocol

## Performance Tips

### Best Quality

- Use WebSocket protocol
- 5GHz WiFi connection
- 1080p @ 60fps
- 8 Mbps bitrate

### Best Latency

- WebSocket protocol
- 720p @ 30fps
- 4 Mbps bitrate
- Disable AI features in app

### Maximum Compatibility

- RTSP or MJPEG protocol
- 720p @ 30fps
- 3 Mbps bitrate

## Building from Source

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt install cmake build-essential pkg-config
sudo apt install libavcodec-dev libavutil-dev libswscale-dev
sudo apt install libcurl4-openssl-dev libssl-dev
sudo apt install qt6-base-dev

# Build
export OBS_SOURCE_DIR=~/obs-studio
export OBS_BUILD_DIR=~/obs-studio/build
cd streamberry-obs
./build.sh install
```

### macOS

```bash
# Install dependencies
brew install cmake pkg-config ffmpeg curl openssl qt@6

# Build
export OBS_SOURCE_DIR=~/obs-studio
export OBS_BUILD_DIR=~/obs-studio/build
cd streamberry-obs
./build.sh install
```

### Windows

Use Visual Studio 2022 with CMake support:

```cmd
set OBS_SOURCE_DIR=C:\obs-studio
set OBS_BUILD_DIR=C:\obs-studio\build
cd streamberry-obs
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
cmake --install . --config Release
```

## Technical Details

### Auto-Discovery

The plugin uses multiple discovery methods:

1. **mDNS/Bonjour**: Automatic service discovery
2. **Port Scanning**: Fallback subnet scanning
3. **Health Checks**: Verifies each protocol endpoint

### Supported Video Formats

- H.264 (AVC)
- Resolution: Up to 3840×2160 (4K)
- Framerate: Up to 60 fps
- Bitrate: Adaptive, up to 20 Mbps

### Supported Audio Formats

- AAC
- Sample Rate: 48 kHz
- Channels: Stereo
- Bitrate: 128 kbps

## FAQ

**Q: Does this work with regular DroidCam?**
A: No, this plugin is specifically for the Streamberry app.

**Q: Can I use USB connection?**
A: Currently WiFi only. USB support may be added later.

**Q: Does it work with other streaming apps?**
A: No, it's designed for Streamberry's protocol.

**Q: What's the latency?**
A: With WebSocket: 50-150ms. With RTSP: 100-300ms.

**Q: Can I stream to multiple OBS instances?**
A: Yes! The app supports multiple simultaneous connections.

**Q: Does it work on all OBS versions?**
A: Requires OBS Studio 28.0 or later.

## Support

- **Issues**: Check OBS logs (`Help` → `Log Files` → `View Current Log`)
- **Logs**: Look for `[BerryStreamCam]` entries
- **Testing**: Use VLC to test RTSP: `rtsp://phone-ip:8554/stream`

## Next Steps

1. ✅ Build and install the plugin
2. ✅ Start Streamberry app
3. ✅ Add BerryStreamCam source in OBS
4. ✅ Start streaming!

Enjoy your wireless camera! 📱→🎥
