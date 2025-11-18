#!/bin/bash

# Build script for BerryStreamCam OBS Plugin
# Usage: ./build.sh [install]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}  BerryStreamCam OBS Plugin Build Script${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo

# Check for OBS - try system installation first
echo
echo -e "${YELLOW}Checking for OBS Studio...${NC}"

if pkg-config --exists libobs 2>/dev/null; then
    echo -e "${GREEN}✓ Found system-installed OBS Studio${NC}"
    OBS_VERSION=$(pkg-config --modversion libobs)
    echo -e "  Version: $OBS_VERSION"
    echo -e "  Headers: $(pkg-config --variable=includedir libobs)"
    echo -e "  Libraries: $(pkg-config --variable=libdir libobs)"
    USE_SYSTEM_OBS=true
else
    echo -e "${YELLOW}System OBS not found, checking for source build...${NC}"
    USE_SYSTEM_OBS=false

    # Check for OBS source directory
    if [ -z "$OBS_SOURCE_DIR" ]; then
        echo -e "${RED}Error: OBS not found in system and OBS_SOURCE_DIR not set${NC}"
        echo
        echo "Option 1 - Install OBS development files (recommended):"
        echo "  sudo apt install libobs-dev obs-studio  # Ubuntu/Debian"
        echo "  sudo dnf install obs-studio-devel       # Fedora"
        echo "  brew install obs                        # macOS"
        echo
        echo "Option 2 - Build from source:"
        echo "  export OBS_SOURCE_DIR=/path/to/obs-studio"
        echo "  export OBS_BUILD_DIR=/path/to/obs-studio/build"
        exit 1
    fi

    if [ ! -d "$OBS_SOURCE_DIR" ]; then
        echo -e "${RED}Error: OBS_SOURCE_DIR does not exist: $OBS_SOURCE_DIR${NC}"
        exit 1
    fi

    echo -e "${YELLOW}OBS Source:${NC} $OBS_SOURCE_DIR"

    # Check for OBS build directory
    if [ -z "$OBS_BUILD_DIR" ]; then
        OBS_BUILD_DIR="$OBS_SOURCE_DIR/build"
        echo -e "${YELLOW}OBS Build:${NC} $OBS_BUILD_DIR (default)"
    else
        echo -e "${YELLOW}OBS Build:${NC} $OBS_BUILD_DIR"
    fi

    if [ ! -d "$OBS_BUILD_DIR" ]; then
        echo -e "${RED}Error: OBS_BUILD_DIR does not exist: $OBS_BUILD_DIR${NC}"
        exit 1
    fi
fi

# Check dependencies
echo
echo -e "${YELLOW}Checking dependencies...${NC}"

check_command() {
    if command -v $1 &> /dev/null; then
        echo -e "  ✓ $1"
        return 0
    else
        echo -e "  ${RED}✗ $1 not found${NC}"
        return 1
    fi
}

DEPS_OK=true
check_command cmake || DEPS_OK=false
check_command make || DEPS_OK=false
check_command pkg-config || DEPS_OK=false

# Check for required libraries
echo
echo -e "${YELLOW}Checking libraries...${NC}"

check_lib() {
    if pkg-config --exists $1 2>/dev/null; then
        echo -e "  ✓ $1"
        return 0
    else
        echo -e "  ${RED}✗ $1 not found${NC}"
        return 1
    fi
}

check_lib libavcodec || DEPS_OK=false
check_lib libavutil || DEPS_OK=false
check_lib libswscale || DEPS_OK=false
check_lib libcurl || DEPS_OK=false
check_lib openssl || DEPS_OK=false
check_lib Qt6Core || DEPS_OK=false

if [ "$DEPS_OK" = false ]; then
    echo
    echo -e "${RED}Missing dependencies. Please install them first.${NC}"
    echo
    echo "On Ubuntu/Debian:"
    echo "  sudo apt install cmake build-essential pkg-config"
    echo "  sudo apt install libavcodec-dev libavutil-dev libswscale-dev"
    echo "  sudo apt install libcurl4-openssl-dev libssl-dev"
    echo "  sudo apt install qt6-base-dev"
    echo
    echo "On Fedora:"
    echo "  sudo dnf install cmake gcc-c++ pkg-config"
    echo "  sudo dnf install ffmpeg-devel libcurl-devel openssl-devel"
    echo "  sudo dnf install qt6-qtbase-devel"
    echo
    echo "On macOS:"
    echo "  brew install cmake pkg-config ffmpeg curl openssl qt@6"
    exit 1
fi

# Create build directory
echo
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake
echo
echo -e "${YELLOW}Running CMake...${NC}"
if [ "$USE_SYSTEM_OBS" = true ]; then
    # Use system OBS (no need to specify paths)
    cmake .. -DCMAKE_BUILD_TYPE=Release
else
    # Use source build OBS
    cmake .. \
        -DOBS_SOURCE_DIR="$OBS_SOURCE_DIR" \
        -DOBS_BUILD_DIR="$OBS_BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release
fi

# Build
echo
echo -e "${YELLOW}Building plugin...${NC}"
if [ "$(uname)" = "Darwin" ]; then
    make -j$(sysctl -n hw.ncpu)
else
    make -j$(nproc)
fi

echo
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${GREEN}  Build completed successfully!${NC}"
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

# Install if requested
if [ "$1" = "install" ]; then
    echo
    echo -e "${YELLOW}Installing plugin...${NC}"

    if [ "$(uname)" = "Darwin" ]; then
        # macOS
        PLUGIN_DIR="$HOME/Library/Application Support/obs-studio/plugins"
        mkdir -p "$PLUGIN_DIR"
        cp -v berrystreamcam.plugin "$PLUGIN_DIR/"
    else
        # Linux
        sudo make install
    fi

    echo
    echo -e "${GREEN}Plugin installed successfully!${NC}"
    echo -e "${YELLOW}Please restart OBS Studio to load the plugin.${NC}"
else
    echo
    echo -e "${YELLOW}Plugin built but not installed.${NC}"
    echo "To install, run:"
    echo "  ./build.sh install"
    echo
    echo "Or manually copy the plugin:"
    if [ "$(uname)" = "Darwin" ]; then
        echo "  cp build/berrystreamcam.plugin ~/Library/Application\\ Support/obs-studio/plugins/"
    else
        echo "  sudo cp build/berrystreamcam.so /usr/lib/obs-plugins/"
    fi
fi

echo
echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
