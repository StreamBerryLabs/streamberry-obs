#!/bin/bash

# Quick test to show system OBS works

echo "==================================="
echo "Testing System OBS Installation"
echo "==================================="
echo

# Check if OBS headers exist
echo "1. Checking OBS headers..."
if [ -f /usr/include/obs/obs-module.h ]; then
    echo "   ✓ obs-module.h found"
else
    echo "   ✗ obs-module.h NOT found"
    exit 1
fi

if [ -f /usr/include/obs/obs-frontend-api.h ]; then
    echo "   ✓ obs-frontend-api.h found"
else
    echo "   ✗ obs-frontend-api.h NOT found"
    exit 1
fi

# Check if OBS library exists
echo
echo "2. Checking OBS library..."
if [ -f /usr/lib/libobs.so ]; then
    echo "   ✓ libobs.so found"
    echo "   Version: $(readlink -f /usr/lib/libobs.so)"
else
    echo "   ✗ libobs.so NOT found"
    exit 1
fi

# Check pkg-config
echo
echo "3. Checking pkg-config..."
if pkg-config --exists libobs; then
    echo "   ✓ pkg-config support found"
    echo "   Version: $(pkg-config --modversion libobs)"
    echo "   CFLAGS: $(pkg-config --cflags libobs)"
    echo "   LIBS: $(pkg-config --libs libobs)"
else
    echo "   ✗ pkg-config NOT configured"
    exit 1
fi

# Test simple compile
echo
echo "4. Testing simple OBS plugin compilation..."
cat > /tmp/test-obs.c << 'EOF'
#include <obs-module.h>

OBS_DECLARE_MODULE()

bool obs_module_load(void) {
    return true;
}

void obs_module_unload(void) {
}

const char *obs_module_name(void) {
    return "test";
}
EOF

gcc -fPIC -shared -o /tmp/test-obs.so /tmp/test-obs.c $(pkg-config --cflags --libs libobs) 2>/dev/null

if [ -f /tmp/test-obs.so ]; then
    echo "   ✓ Test plugin compiled successfully!"
    ls -lh /tmp/test-obs.so
    rm /tmp/test-obs.so /tmp/test-obs.c
else
    echo "   ✗ Compilation failed"
    exit 1
fi

echo
echo "==================================="
echo "✓ System OBS is fully functional!"
echo "==================================="
echo
echo "You can now build the plugin WITHOUT setting OBS paths:"
echo "  cd plugin"
echo "  ./build.sh"
echo
echo "The build script will automatically detect and use system OBS!"
