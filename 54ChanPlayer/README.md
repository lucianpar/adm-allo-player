# 54-Channel Audio Player

A multichannel audio playback system for the Allosphere, built with [allolib](https://github.com/AlloSphere-Research-Group/allolib).

## Overview

This application plays back 54-channel audio files and maps them to the Allosphere's speaker array. It includes:

- Real-time dB meters for all 54 channels
- GUI controls for playback, pause, loop, and gain
- Channel remapping to match the Allosphere speaker layout

## Speaker Layout

The Allosphere has **54 speakers** arranged in three rings:

| Ring | Speakers | Allosphere Channels | Elevation |
|------|----------|---------------------|-----------|
| Upper | 12 | 1-12 | Positive (above ear level) |
| Middle | 30 | 17-46 | Zero (ear level) |
| Lower | 12 | 49-60 | Negative (below ear level) |

**Skipped channels**: 13-16, 47-48 (no physical speakers)

## Files

| File | Description |
|------|-------------|
| `mainplayer.cpp` | Main application with GUI and audio playback |
| `channelMapping.hpp` | Channel mapping configuration (file → speaker) |
| `CMakeLists.txt` | CMake build configuration |
| `sourceAudio/` | Directory for audio files |

---

## Usage

### 1. Build

```bash
cd 54ChanPlayer
mkdir -p build
cd build
cmake ..
cmake --build .
```

### 2. Add Audio File

Place your 54-channel audio file in the `sourceAudio/` directory.

Update `mainplayer.cpp` if your filename differs:
```cpp
std::string audioFileName = "YourFile.wav";
```

### 3. Run

```bash
./mainplayer
```

### GUI Controls

| Control | Function |
|---------|----------|
| **Play/Pause** | Start or pause playback |
| **Rewind** | Return to beginning |
| **Loop** | Toggle looping |
| **Gain** | Master volume (0.0 - 1.0) |
| **Show Meters** | Toggle dB meter display |

---

## Channel Mapping

Audio file channels map to Allosphere outputs as follows:

```
File Ch 1-12  → Allo Ch 1-12   (Upper Ring)
File Ch 13-42 → Allo Ch 17-46  (Middle Ring)
File Ch 43-54 → Allo Ch 49-60  (Lower Ring)
```

To modify mappings, edit `channelMapping.hpp`.

---

## Requirements

- CMake 3.5+
- C++17 compiler
- allolib (included as submodule in parent directory)

## License

See parent project license.
