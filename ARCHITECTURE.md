# BerryStreamCam Architecture Diagram

## System Overview

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

## Data Flow

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

### RTSP Protocol

```
Android App                          OBS Plugin
─────────────                        ──────────

1. Start Server
   (Port 8554) ────────────────────► Discover
                                      Connect to port

2. RTSP Handshake
   OPTIONS ◄────────────────────────
   200 OK ──────────────────────────►

   DESCRIBE ◄───────────────────────
   200 OK + SDP ────────────────────► Parse SDP
                                       Extract codec info

   SETUP ◄──────────────────────────
   200 OK + Session ────────────────► Create RTP sockets

   PLAY ◄───────────────────────────
   200 OK ──────────────────────────►

3. RTP Packets
   H.264 NAL
   ↓
   RTP Packetize
   ↓
   UDP Send ────────────────────────► RTP Receive
                                       Reassemble NALs
                                       ↓
                                       H.264 Decoder
                                       ↓
                                       Render

4. TEARDOWN ◄────────────────────────
   200 OK ──────────────────────────►
```

### HTTP Raw H.264

```
Android App                          OBS Plugin
─────────────                        ──────────

1. Start Server
   (Port 8081) ────────────────────► Discover
                                      GET /stream.h264

2. HTTP Response
   HTTP/1.1 200 OK
   Content-Type: video/h264

   Continuous stream:
   00 00 00 01 [SPS]
   00 00 00 01 [PPS]
   00 00 00 01 [IDR frame]
   00 00 00 01 [P frame]
   ... ─────────────────────────────► Parse NAL units
                                       Find start codes
                                       Extract frames
                                       ↓
                                       H.264 Decoder
                                       ↓
                                       Render
```

## Threading Model

```
┌─────────────────────────────────────────────────────────────┐
│                    OBS Main Thread                           │
│                                                               │
│  - GUI events                                                │
│  - Rendering                                                 │
│  - Texture updates                                           │
└─────────────────────────────────────────────────────────────┘
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

## Protocol Selection Logic

```
Device Discovery
       ↓
Check port 8080 (WebSocket)
  ├─ /health endpoint
  ├─ HTTP 200 OK? ✓ → Add WebSocket to available
  └─ Failed? ✗ → Skip
       ↓
Check port 8081 (HTTP)
  ├─ / endpoint
  ├─ HTTP 200 OK? ✓ → Add HTTP protocols
  │   ├─ /stream.h264 → Raw H.264
  │   ├─ /mjpeg → MJPEG
  │   └─ /dash/manifest.mpd → DASH
  └─ Failed? ✗ → Skip
       ↓
Check port 8554 (RTSP)
  ├─ TCP connect
  ├─ Success? ✓ → Add RTSP to available
  └─ Failed? ✗ → Skip
       ↓
Display device with available protocols
```

## Build System Flow

```
build.sh
   ↓
Check dependencies
   ├─ CMake
   ├─ Qt6
   ├─ FFmpeg
   ├─ libcurl
   └─ OpenSSL
   ↓
CMakeLists.txt
   ↓
Find OBS Studio
   ├─ OBS_SOURCE_DIR (headers)
   └─ OBS_BUILD_DIR (libraries)
   ↓
Compile sources
   ├─ Plugin core
   ├─ Discovery system
   ├─ Protocol handlers
   ├─ H.264 decoder
   └─ UI widgets
   ↓
Link libraries
   ├─ libobs
   ├─ Qt6::Core/Widgets/Network
   ├─ FFmpeg
   └─ libcurl
   ↓
Create plugin binary
   ├─ Linux: berrystreamcam.so
   ├─ macOS: berrystreamcam.plugin
   └─ Windows: berrystreamcam.dll
   ↓
Install
   ├─ Linux: /usr/lib/obs-plugins/
   ├─ macOS: ~/Library/Application Support/obs-studio/plugins/
   └─ Windows: C:\Program Files\obs-studio\obs-plugins\64bit\
```

## Memory Management

```
Frame Lifecycle
───────────────

1. Receive from network
   buffer = malloc(FRAME_BUFFER_SIZE);

2. Store in handler
   frame_queue.push(frame);  // Copy

3. Decoder processes
   decoded = decoder.decode(frame);  // FFmpeg manages

4. Convert to RGBA
   rgba_buffer = sws_scale(...);  // Reuse buffer

5. Upload to GPU
   gs_texture_set_image(texture, rgba_buffer);

6. Render in OBS
   gs_draw_sprite(texture);  // GPU only

Cleanup:
- Frame buffers: Freed when queue is cleared
- Decoder: av_frame_free(), avcodec_free_context()
- Texture: gs_texture_destroy()
- Network: close() sockets
```

This architecture provides:

- ✅ Automatic device discovery
- ✅ Multiple protocol support
- ✅ Low latency streaming
- ✅ Efficient resource usage
- ✅ Thread-safe operation
- ✅ Clean separation of concerns
