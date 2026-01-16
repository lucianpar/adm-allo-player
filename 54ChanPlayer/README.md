# 54-Channel Audio Player

A multichannel audio playback system for the Allosphere, built with [allolib](https://github.com/AlloSphere-Research-Group/allolib).

## Overview

This application plays back 54-channel audio files and maps them to the Allosphere's speaker array. It includes:

- **Runtime file selection** - switch audio files from the GUI dropdown
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

### 2. Add Audio Files

Place your 54-channel audio files (`.wav`, `.aiff`, `.flac`) in the `sourceAudio/` directory.

### 3. Run

```bash
./mainplayer
```

### 4. Select Audio File

Use the **dropdown menu** at the top of the GUI to switch between audio files. No rebuild required!

Click **↻ Refresh** to rescan the folder if you add new files while the app is running.

### GUI Controls

| Control | Function |
|---------|----------|
| **File Dropdown** | Select audio file from sourceAudio/ |
| **↻ Refresh** | Rescan folder for new files |
| **Play/Pause** | Start or pause playback |
| **Stop** | Stop and reset to beginning |
| **Rewind** | Return to beginning |
| **Loop** | Toggle looping |
| **Gain** | Master volume (0.0 - 1.0) |
| **Show Meters** | Toggle dB meter display |

### Supported Audio Formats

- `.wav` (recommended)
- `.aiff` / `.aif`
- `.flac`

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
