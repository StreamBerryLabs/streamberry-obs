#!/bin/bash
# BerryStreamCam Release Builder
# Builds plugin for multiple platforms and creates release packages

set -e

VERSION="1.0.0"
PLUGIN_NAME="berrystreamcam"
BUILD_DIR="build"
RELEASE_DIR="release"

echo "🎬 BerryStreamCam Release Builder v${VERSION}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    PLUGIN_EXT="so"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    PLUGIN_EXT="plugin"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="windows"
    PLUGIN_EXT="dll"
else
    echo "❌ Unsupported platform: $OSTYPE"
    exit 1
fi

echo "📦 Platform: $PLATFORM"
echo "🔧 Building plugin..."

# Clean previous builds
rm -rf "$BUILD_DIR"
rm -rf "$RELEASE_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p "$RELEASE_DIR"

# Build plugin
cd "$BUILD_DIR"

if [[ "$PLATFORM" == "windows" ]]; then
    cmake .. -G "Visual Studio 17 2022" -A x64
    cmake --build . --config Release
    BINARY_PATH="Release/${PLUGIN_NAME}.${PLUGIN_EXT}"
else
    cmake ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
    BINARY_PATH="${PLUGIN_NAME}.${PLUGIN_EXT}"
fi

cd ..

echo "✅ Build complete!"

# Create release package
echo "📦 Creating release package..."

RELEASE_NAME="${PLUGIN_NAME}-v${VERSION}-${PLATFORM}"
RELEASE_PATH="${RELEASE_DIR}/${RELEASE_NAME}"

mkdir -p "$RELEASE_PATH"

# Copy plugin binary
if [[ "$PLATFORM" == "macos" ]]; then
    cp -r "${BUILD_DIR}/${BINARY_PATH}" "$RELEASE_PATH/"
else
    cp "${BUILD_DIR}/${BINARY_PATH}" "$RELEASE_PATH/"
fi

# Copy documentation
cp README.md "$RELEASE_PATH/"
cp QUICKSTART.md "$RELEASE_PATH/" 2>/dev/null || true

# Create installation instructions
cat > "$RELEASE_PATH/INSTALL.txt" << EOF
BerryStreamCam OBS Plugin v${VERSION}
Installation Instructions for ${PLATFORM^}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

LINUX:
  1. Copy ${PLUGIN_NAME}.so to /usr/lib/obs-plugins/
     sudo cp ${PLUGIN_NAME}.so /usr/lib/obs-plugins/

  2. Restart OBS Studio

MACOS:
  1. Copy ${PLUGIN_NAME}.plugin to ~/Library/Application Support/obs-studio/plugins/
     cp -r ${PLUGIN_NAME}.plugin ~/Library/Application Support/obs-studio/plugins/

  2. Restart OBS Studio

WINDOWS:
  1. Copy ${PLUGIN_NAME}.dll to C:\Program Files\obs-studio\obs-plugins\64bit\

  2. Restart OBS Studio

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

For detailed instructions, see README.md

Support: https://github.com/StreamBerryLabs/streamberry-obs/issues
EOF

# Create version file
cat > "$RELEASE_PATH/VERSION" << EOF
${VERSION}
EOF

# Create changelog
cat > "$RELEASE_PATH/CHANGELOG.md" << EOF
# Changelog

## v${VERSION} ($(date +%Y-%m-%d))

### Features
- ✨ Auto-discovery of Streamberry devices
- 🚀 5 streaming protocols (WebSocket, HTTP H.264, MJPEG, RTSP, DASH)
- 🎬 Hardware-accelerated H.264 decoding
- 🔄 Live protocol switching
- 🎯 PipeWire-style pause/resume state management
- 📱 Multi-device support

### Improvements
- 🛡️ Thread-safe operations
- 💪 Robust error handling
- 🔧 Improved connection stability
- 📊 Better logging

### Platforms
- ✅ Linux (tested on Arch, Ubuntu, Fedora)
- ✅ macOS (tested on 13+)
- ✅ Windows (tested on 10/11)

### Known Issues
- RTSP protocol temporarily disabled (coming in v1.1)
- Audio streaming not yet implemented

### Requirements
- OBS Studio 30.0+
- Qt 6.2+
- FFmpeg 6.0+
EOF

# Create archive
echo "🗜️  Creating archive..."
cd "$RELEASE_DIR"

if command -v zip &> /dev/null; then
    zip -r "${RELEASE_NAME}.zip" "$RELEASE_NAME"
    echo "✅ Created: ${RELEASE_NAME}.zip"
else
    tar -czf "${RELEASE_NAME}.tar.gz" "$RELEASE_NAME"
    echo "✅ Created: ${RELEASE_NAME}.tar.gz"
fi

cd ..

# Calculate checksums
echo "🔐 Calculating checksums..."
cd "$RELEASE_DIR"

if [[ "$PLATFORM" == "linux" ]] || [[ "$PLATFORM" == "macos" ]]; then
    if command -v sha256sum &> /dev/null; then
        sha256sum "${RELEASE_NAME}".* > SHA256SUMS
    else
        shasum -a 256 "${RELEASE_NAME}".* > SHA256SUMS
    fi
else
    certutil -hashfile "${RELEASE_NAME}.zip" SHA256 > SHA256SUMS
fi

cd ..

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Release package created successfully!"
echo ""
echo "📦 Package: release/${RELEASE_NAME}.zip"
echo "📁 Contents:"
ls -lh "release/${RELEASE_NAME}/"
echo ""
echo "🔐 Checksums: release/SHA256SUMS"
cat "release/SHA256SUMS"
echo ""
echo "🚀 Ready to upload to GitHub Releases!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
