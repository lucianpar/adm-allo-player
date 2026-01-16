# 54ChanPlayer - Developer Reference

Technical reference for future development and AI agents.

---

## Project Structure

```
54ChanPlayer/
├── mainplayer.cpp      # Main application source
├── channelMapping.hpp  # Channel mapping header (0-indexed & 1-indexed)
├── CMakeLists.txt      # CMake build config
├── README.md           # User documentation
├── DEVELOPER.md        # This file
├── sourceAudio/        # Audio files directory
└── build/              # Build output (generated)
```

---

## CMake Configuration

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.5...4.1)
project(54ChanPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# allolib is in parent directory
set(ALLOLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../allolib)
add_subdirectory(${ALLOLIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/allolib)

add_executable(mainplayer mainplayer.cpp)
target_link_libraries(mainplayer PRIVATE al)
```

### Build Commands

```bash
# Clean build
rm -rf build && mkdir build && cd build
cmake ..
cmake --build .

# Or single line
cmake -S . -B build && cmake --build build
```

### Common CMake Errors

| Error | Solution |
|-------|----------|
| `cmake_minimum_required` version error | Update to `VERSION 3.5...4.1` |
| `CMakeLists.txt not found` | Ensure file is named `CMakeLists.txt` (not `CMakeCache.txt`) |
| `Parse error in cache file` | Delete `CMakeCache.txt` and `CMakeFiles/`, rebuild |

---

## allolib Integration

### Key Headers

```cpp
#include "al/app/al_App.hpp"       // Main application class
#include "al/io/al_File.hpp"       // File path utilities
#include "al/io/al_Imgui.hpp"      // ImGui integration
#include "al/sound/al_SoundFile.hpp" // Audio file loading
```

### App Structure

```cpp
struct MyApp : App {
  void onInit() override;     // Called once at startup
  void onCreate() override;   // Called after OpenGL context created
  void onSound(AudioIOData& io) override;  // Audio callback
  void onDraw(Graphics& g) override;       // Render callback
};

int main() {
  MyApp app;
  app.configureAudio(48000, 512, 60, 0);  // SR, bufferSize, outputs, inputs
  app.start();
}
```

### Audio Callback Pattern

```cpp
void onSound(AudioIOData& io) override {
  while (io()) {
    // Process each sample frame
    for (int ch = 0; ch < io.channelsOut(); ch++) {
      io.out(ch) = yourSample;
    }
  }
}
```

---

## Channel Mapping API

### Available Maps

```cpp
namespace ChannelMapping {
  // 0-indexed (for buffer access)
  constexpr std::array<std::pair<int, int>, 54> defaultChannelMap;
  
  // 1-indexed (matches speaker layout JSON)
  constexpr std::array<std::pair<int, int>, 54> oneIndexedChannelMap;
  
  // Alias for defaultChannelMap
  constexpr auto& channelMap = defaultChannelMap;
}
```

### Helper Functions

```cpp
// 0-indexed
int getOutputChannel(int audioFileChannel);
int getInputChannel(int allosphereChannel);

// 1-indexed
int getOutputChannel1Indexed(int audioFileChannel);
int getInputChannel1Indexed(int allosphereChannel);

// Conversion
int toOneIndexed(int zeroIndexed);   // +1
int toZeroIndexed(int oneIndexed);   // -1
```

### Usage Example

```cpp
#include "channelMapping.hpp"

// In audio callback
for (int i = 0; i < ChannelMapping::NUM_CHANNELS; i++) {
  int fileChannel = ChannelMapping::defaultChannelMap[i].first;
  int outputChannel = ChannelMapping::defaultChannelMap[i].second;
  
  if (outputChannel < io.channelsOut()) {
    io.out(outputChannel, frame) = buffer[frame * numChannels + fileChannel] * gain;
  }
}
```

---

## Allosphere Speaker Layout

### Physical Layout (54 speakers)

```
Upper Ring:  12 speakers (positive elevation)
             Allo outputs: 1-12

Middle Ring: 30 speakers (ear level, el=0)
             Allo outputs: 17-46
             
Lower Ring:  12 speakers (negative elevation)
             Allo outputs: 49-60

SKIPPED:     Outputs 13-16, 47-48 (no speakers)
```

### Speaker Layout JSON Format

```json
{
  "speakers": [
    {
      "channel": 1,
      "az": 1.355,     // Azimuth (radians)
      "el": 0.570,     // Elevation (radians)
      "radius": 5.929  // Distance from center (meters)
    }
  ]
}
```

---

## Audio File Requirements

| Property | Requirement |
|----------|-------------|
| Channels | 54 (will warn if different) |
| Format | WAV (libsndfile supported formats) |
| Sample Rate | 48000 Hz recommended |
| Bit Depth | 16/24/32-bit |

### Loading Audio

```cpp
SoundFile soundFile;
if (soundFile.open(audioPath.c_str())) {
  // soundFile.channels - number of channels
  // soundFile.sampleRate - sample rate
  // soundFile.frames - total frames
  // soundFile.data - pointer to interleaved samples
}
```

---

## ImGui Integration

### Setup

```cpp
void onCreate() override {
  imguiInit();
}

void onDraw(Graphics& g) override {
  imguiBeginFrame();
  // ImGui widgets here
  imguiEndFrame();
  imguiDraw();
}
```

### Common Widgets

```cpp
ImGui::Begin("Window Title");
ImGui::Checkbox("Enable", &boolVar);
ImGui::SliderFloat("Gain", &floatVar, 0.0f, 1.0f);
ImGui::Button("Click Me");
ImGui::Text("Info: %d", intVar);
ImGui::End();
```

---

## Debugging Tips

### Print Audio Info

```cpp
std::cout << "Channels: " << soundFile.channels << std::endl;
std::cout << "Sample Rate: " << soundFile.sampleRate << std::endl;
std::cout << "Frames: " << soundFile.frames << std::endl;
```

### Check Output Channels

```cpp
std::cout << "Output channels available: " << io.channelsOut() << std::endl;
```

### Verify Channel Mapping

```cpp
for (int i = 0; i < ChannelMapping::NUM_CHANNELS; i++) {
  auto& m = ChannelMapping::oneIndexedChannelMap[i];
  std::cout << "File " << m.first << " -> Allo " << m.second << std::endl;
}
```

---

## Dependencies

- **allolib**: Audio/graphics framework (submodule at `../allolib`)
- **libsndfile**: Audio file I/O (bundled with allolib)
- **ImGui**: GUI (bundled with allolib)
- **CMake**: Build system (3.5+)
- **C++17**: Required standard

---

## Related Files

| Path | Description |
|------|-------------|
| `../allolib/` | allolib framework |
| `../CMakeLists.txt` | Parent project CMake |
| `../run.sh` | Build script (parent project) |
