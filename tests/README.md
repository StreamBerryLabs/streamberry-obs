# BerryStreamCam Plugin Tests

This directory contains unit and integration tests for the BerryStreamCam OBS plugin.

## Prerequisites

### Install Google Test

**Ubuntu/Debian:**
```bash
sudo apt-get install libgtest-dev cmake
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/*.a /usr/lib
```

**Fedora:**
```bash
sudo dnf install gtest-devel
```

**Arch Linux:**
```bash
sudo pacman -S gtest
```

**macOS:**
```bash
brew install googletest
```

## Building Tests

```bash
cd plugin
mkdir build && cd build

# Configure with tests enabled
cmake .. -DBUILD_TESTS=ON

# Build
make

# Run all tests
ctest --output-on-failure

# Or run specific test
./tests/test_websocket_handler
./tests/test_integration
```

## Test Categories

### Unit Tests (`test_websocket_handler`)

Tests for WebSocketHandler class focusing on:
- Thread-safe creation and destruction
- Concurrent operations
- Frame queue handling
- Connection state tracking
- Cleanup sequence

**Run unit tests only:**
```bash
ctest -L unit --output-on-failure
```

### Integration Tests (`test_integration`)

Tests for full OBS source lifecycle:
- Source create/destroy
- Source show/hide (pause/resume)
- Source activate/deactivate
- Rapid state changes
- Camera disconnect scenarios
- Long-running streams

**Run integration tests only:**
```bash
ctest -L integration --output-on-failure
```

## Test Coverage

The tests cover the critical threading fixes:
- ✅ Thread-safe WebSocket handler creation
- ✅ Proper worker thread lifecycle
- ✅ Cleanup sequence without crashes
- ✅ Frame queue thread safety
- ✅ Concurrent access handling

## Manual Testing

For comprehensive validation, also perform manual testing:

1. **Hide source while streaming** - Should pause gracefully
2. **Deactivate source while streaming** - Should stop without crash
3. **Camera disconnect** - Should handle gracefully
4. **Close OBS while streaming** - Should cleanup properly
5. **Rapid show/hide cycles** - Should remain stable

## Continuous Integration

To run tests in CI:

```bash
# Build with tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build

# Run tests with verbose output
cd build
ctest --output-on-failure --verbose
```

## Troubleshooting

**Tests fail to build:**
- Ensure Google Test is installed
- Check Qt6 is properly configured
- Verify OBS headers are available

**Tests timeout:**
- Increase timeout in CMakeLists.txt
- Check for deadlocks in threaded code

**Segfaults in tests:**
- Run with debugger: `gdb ./tests/test_websocket_handler`
- Check for memory leaks: `valgrind ./tests/test_websocket_handler`

## Adding New Tests

1. Create test file in `tests/` directory
2. Add to `tests/CMakeLists.txt`
3. Follow Google Test conventions
4. Focus on core functionality
5. Keep tests minimal and fast
