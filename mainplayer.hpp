#ifndef MAINPLAYER_HPP
#define MAINPLAYER_HPP

/*
54-Channel Audio Playback System
Plays back a multichannel audio file with all channels mapped to individual outputs.
Includes GUI controls for playback, pause, loop, and rewind.
Includes real-time dB meters for all 54 channels.
*/

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <filesystem>
#include "al/app/al_App.hpp"
#include "al/io/al_File.hpp"
#include "al/io/al_Imgui.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "channelMapping.hpp"

using namespace al;

struct adm_player {
  SoundFile soundFile;
  uint64_t frameCounter = 0;
  std::vector<float> buffer;

  // Playback controls
  bool playing = false;
  bool loop = true;
  float gain = 0.5f;

  // Audio file info
  int numChannels = 56; //default 
  int expectedChannels = 60; //default
  std::string audioFolder;
  // std::string audioFolder = "../adm-allo-player/sourceAudio/";
  //std::string audioFileName = "1-swale-allo-render.wav";
  // selection is done via audioFiles + selectedFileIndex (no single audioFileName string)

  // Metering
  std::vector<float> channelLevels;  // Linear amplitude for each channel
  std::vector<float> channelPeaks;   // Peak hold for each channel
  int peakHoldFrames = 24;           // How long to hold peaks (in render frames)
  std::vector<int> peakHoldCounters; // Counter for peak hold
  float meterDecayRate = 0.95f;      // How fast meters decay
  bool showMeters = true;

  // File selection
  std::vector<std::string> audioFiles;  // List of available audio files
  int selectedFileIndex = 0;            // Currently selected file index

  //gui 
  bool displayGUI;
  public:
  void toggleGUI(bool toggle = false) {
    displayGUI = toggle;
  }

  void setSourceAudioFolder(const std::string& folder) {
    audioFolder = folder;
  }
  // Scan sourceAudio folder for .wav files
  void scanAudioFiles() {
    audioFiles.clear();
    std::string audioDir = al::File::currentPath() + audioFolder;

    std::cout << "Scanning for audio files in: " << audioDir << std::endl;

    try {
      for (const auto& entry : std::filesystem::directory_iterator(audioDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".wav") {
          audioFiles.push_back(entry.path().filename().string());
        }
      }
    } catch (const std::exception& e) {
      std::cerr << "Error scanning audio directory: " << e.what() << std::endl;
    }

    std::cout << "Found " << audioFiles.size() << " audio files" << std::endl;
  }

  // Load a new audio file
  bool loadAudioFile(const std::string& filename) {
    std::string audioPath = al::File::currentPath() + audioFolder + filename;

    std::cout << "\n=== Loading new audio file ===" << std::endl;
    std::cout << "File: " << audioPath << std::endl;

    // Stop playback during load
    bool wasPlaying = playing;
    playing = false;

    if (!soundFile.open(audioPath.c_str())) {
      std::cerr << "✗ ERROR: Could not open file: " << audioPath << std::endl;
      return false;
    }

    std::cout << "✓ Audio file loaded successfully" << std::endl;
    std::cout << "  Sample rate: " << soundFile.sampleRate << " Hz" << std::endl;
    std::cout << "  Channels: " << soundFile.channels << std::endl;
    std::cout << "  Frame count: " << soundFile.frameCount << std::endl;
    std::cout << "  Duration: " << (double)soundFile.frameCount / soundFile.sampleRate << " seconds" << std::endl;

    numChannels = soundFile.channels;
    // note: we don't store a single filename string; selection is tracked by audioFiles[selectedFileIndex]

    if (numChannels != expectedChannels) {
      std::cerr << "⚠ WARNING: Expected " << expectedChannels << " channels but file has "
                << numChannels << " channels." << std::endl;
    }

    // Reset playback position
    frameCounter = 0;

    // Resize buffers for new channel count
    int framesPerBuffer = 512;
    buffer.resize(framesPerBuffer * numChannels);
    channelLevels.resize(expectedChannels, 0.0f);
    channelPeaks.resize(expectedChannels, 0.0f);
    peakHoldCounters.resize(expectedChannels, 0);

    // Resume playback if was playing
    playing = wasPlaying;

    return true;
  }

  void onInit()  {
    std::cout << "\n=== 54-Channel Audio Player ===" << std::endl;
    std::cout << "Current path: " << al::File::currentPath() << std::endl;

    // populate audioFiles from folder and pick selectedFileIndex
    scanAudioFiles();
    if (audioFiles.empty()) {
      std::cerr << "✗ ERROR: No audio files found in: " << al::File::currentPath() + audioFolder << std::endl;
      std::cerr << "Please update the audioFolder or add files." << std::endl;
      // quit();
      return;
    }
    if (selectedFileIndex < 0 || selectedFileIndex >= static_cast<int>(audioFiles.size())) selectedFileIndex = 0;

    // Load the selected file (loadAudioFile prints details)
    if (!loadAudioFile(audioFiles[selectedFileIndex])) {
      std::cerr << "✗ ERROR: Could not open selected audio file." << std::endl;
      // quit();
      return;
    }

    // Ensure buffers/meters sized (loadAudioFile already resizes but keep safe)
    int framesPerBuffer = 512;
    buffer.resize(framesPerBuffer * numChannels);
    channelLevels.resize(expectedChannels, 0.0f);
    channelPeaks.resize(expectedChannels, 0.0f);
    peakHoldCounters.resize(expectedChannels, 0);
    frameCounter = 0;
  }

  void onCreate() {
    if (displayGUI) {
      imguiInit();
      std::cout << "GUI initialized" << std::endl;
    }
    else {
      std::cout << "GUI disabled" << std::endl;
    }
  }

  void onDraw(Graphics& g) {
    if (displayGUI) {
      imguiBeginFrame();

    ImGui::Begin("54-Channel Audio Player");

    // File selector dropdown
    ImGui::Text("Audio File:");
    if (!audioFiles.empty()) {
      // Create combo box with available files
      const char* preview = audioFiles[selectedFileIndex].c_str();
      if (ImGui::BeginCombo("##fileselect", preview)) {
        for (int i = 0; i < static_cast<int>(audioFiles.size()); i++) {
          bool isSelected = (selectedFileIndex == i);
          if (ImGui::Selectable(audioFiles[i].c_str(), isSelected)) {
            if (i != selectedFileIndex) {
              selectedFileIndex = i;
              loadAudioFile(audioFiles[i]);
            }
          }
          if (isSelected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
      if (ImGui::Button("↻ Refresh")) {
        scanAudioFiles();
      }
    } else {
      ImGui::Text("No audio files found in sourceAudio/");
      if (ImGui::Button("Scan for Files")) {
        scanAudioFiles();
      }
    }

    ImGui::Separator();
    ImGui::Text("File Info:");
    ImGui::Text("  File Channels: %d", numChannels);
    ImGui::Text("  Output Channels: %d", expectedChannels);
    ImGui::Text("  Sample Rate: %d Hz", soundFile.sampleRate);
    ImGui::Text("  Duration: %.2f seconds", (double)soundFile.frameCount / soundFile.sampleRate);

    ImGui::Separator();
    ImGui::Text("Playback:");
    ImGui::Text("  Current Frame: %llu / %llu", frameCounter, soundFile.frameCount);
    ImGui::Text("  Current Time: %.2f / %.2f seconds",
                (double)frameCounter / soundFile.sampleRate,
                (double)soundFile.frameCount / soundFile.sampleRate);

    ImGui::Separator();
    ImGui::Text("Controls:");

    if (ImGui::Button(playing ? "⏸ Pause" : "▶ Play")) {
      playing = !playing;
    }

    ImGui::SameLine();
    if (ImGui::Button("⏹ Stop")) {
      playing = false;
      frameCounter = 0;
    }

    ImGui::SameLine();
    if (ImGui::Button("⏮ Rewind")) {
      frameCounter = 0;
    }

    if (ImGui::Checkbox("Loop", &loop)) {
      std::cout << "Loop: " << (loop ? "ON" : "OFF") << std::endl;
    }

    if (ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f)) {
      std::cout << "Gain: " << gain << std::endl;
    }

    ImGui::Separator();
    ImGui::Checkbox("Show Channel Meters", &showMeters);

    if (showMeters) {
      ImGui::Text("Channel Levels (dB):");

      // Display meters in a scrollable area
      ImGui::BeginChild("Meters", ImVec2(0, 400), true);

      for (int ch = 0; ch < expectedChannels; ch++) {
        // Convert linear amplitude to dB
        float levelDB = -120.0f; // Silence floor
        if (channelLevels[ch] > 0.0f) {
          levelDB = 20.0f * log10f(channelLevels[ch]);
        }

        float peakDB = -120.0f;
        if (channelPeaks[ch] > 0.0f) {
          peakDB = 20.0f * log10f(channelPeaks[ch]);
        }

        // Clamp to reasonable display range
        levelDB = (levelDB < -60.0f) ? -60.0f : levelDB;
        peakDB = (peakDB < -60.0f) ? -60.0f : peakDB;

        // Normalize to 0-1 range for display (-60dB to 0dB)
        float levelNorm = (levelDB + 60.0f) / 60.0f;
        float peakNorm = (peakDB + 60.0f) / 60.0f;

        // Color based on level (green -> yellow -> red)
        ImVec4 color;
        if (levelNorm < 0.5f) {
          color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        } else if (levelNorm < 0.85f) {
          color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
        } else {
          color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
        }

        // Channel label and dB value
        ImGui::Text("Ch %2d:", ch + 1);
        ImGui::SameLine(60);

        // Progress bar for meter
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(levelNorm, ImVec2(200, 0), "");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Peak indicator (small vertical line)
        if (peakNorm > 0.01f) {
          ImGui::Text("|");
        } else {
          ImGui::Text(" ");
        }

        ImGui::SameLine();

        // Show dB value
        if (levelDB > -60.0f) {
          ImGui::Text("%5.1f dB", levelDB);
        } else {
          ImGui::Text("  -inf");
        }
      }

      ImGui::EndChild();
    }

    ImGui::End();

    imguiEndFrame();
    g.clear(0, 0, 0);
    imguiDraw();
  }
  }

  void onSound(AudioIOData& io) {
    if (!soundFile.data.size()) {
      // No file loaded, output silence
      while (io()) {
        for (int ch = 0; ch < io.channelsOut(); ch++) {
          io.out(ch) = 0.0f;
        }
      }
      return;
    }

    uint64_t numFrames = io.framesPerBuffer();

    // Resize buffer if needed
    if (buffer.size() < numFrames * numChannels) {
      buffer.resize(numFrames * numChannels);
    }

    // If not playing, output silence
    if (!playing) {
      while (io()) {
        for (int ch = 0; ch < io.channelsOut(); ch++) {
          io.out(ch) = 0.0f;
        }
      }
      return;
    }

    // Check if we're at the end
    if (frameCounter >= soundFile.frameCount) {
      if (loop) {
        frameCounter = 0;
      } else {
        playing = false;
        while (io()) {
          for (int ch = 0; ch < io.channelsOut(); ch++) {
            io.out(ch) = 0.0f;
          }
        }
        return;
      }
    }

    // Adjust numFrames if we're near the end
    if (frameCounter + numFrames > soundFile.frameCount) {
      numFrames = soundFile.frameCount - frameCounter;
    }

    // Get pointer to current frame in the soundfile
    float* frames = soundFile.getFrame(frameCounter);

    // Copy interleaved data to buffer
    for (uint64_t i = 0; i < numFrames * numChannels; i++) {
      buffer[i] = frames[i];
    }

    // Deinterleave and output to all channels WITH REMAPPING
    int outputChannels = (numChannels < io.channelsOut()) ? numChannels : io.channelsOut();
    
    // Reset channel levels for this buffer (size to output channels)
    std::vector<float> maxLevels(io.channelsOut(), 0.0f);    for (uint64_t frame = 0; frame < numFrames; frame++) {
      // Clear all outputs first
      for (int ch = 0; ch < io.channelsOut(); ch++) {
        io.out(ch, frame) = 0.0f;
      }

      // Apply channel mapping
      for (int i = 0; i < ChannelMapping::NUM_CHANNELS && i < numChannels; i++) {
        int fileChannel = ChannelMapping::channelMap[i].first;
        int outputChannel = ChannelMapping::channelMap[i].second;

        // Bounds check
        if (fileChannel < numChannels && outputChannel < io.channelsOut()) {
          float sample = buffer[frame * numChannels + fileChannel] * gain;
          io.out(outputChannel, frame) = sample;

          // Track max level for metering (use output channel index for display)
          float absSample = fabsf(sample);
          if (absSample > maxLevels[outputChannel]) {
            maxLevels[outputChannel] = absSample;
          }
        }
      }
    }

    // Update meters with max levels from this buffer
    for (int ch = 0; ch < io.channelsOut(); ch++) {
      // Smooth decay for current level
      channelLevels[ch] = channelLevels[ch] * meterDecayRate;

      // Update with new max if higher
      if (maxLevels[ch] > channelLevels[ch]) {
        channelLevels[ch] = maxLevels[ch];
      }

      // Update peak hold
      if (maxLevels[ch] > channelPeaks[ch]) {
        channelPeaks[ch] = maxLevels[ch];
        peakHoldCounters[ch] = peakHoldFrames;
      } else {
        // Decay peak hold
        if (peakHoldCounters[ch] > 0) {
          peakHoldCounters[ch]--;
        } else {
          channelPeaks[ch] = channelPeaks[ch] * meterDecayRate;
        }
      }
    }

    // Fill remaining frames with silence if we read fewer frames
    for (uint64_t frame = numFrames; frame < io.framesPerBuffer(); frame++) {
      for (int ch = 0; ch < io.channelsOut(); ch++) {
        io.out(ch, frame) = 0.0f;
      }
    }

    frameCounter += numFrames;
  }

  bool onKeyDown(const Keyboard& k) {
    // Play/pause
    if (k.key() == ' ') {
      playing = !playing;
      std::cout << (playing ? "▶ Playing" : "⏸ Paused") << std::endl;
      return true;
    }
    // Rewind
    if (k.key() == 'r' || k.key() == 'R') {
      frameCounter = 0;
      std::cout << "⏮ Rewound to beginning" << std::endl;
      return true;
    }
    // Toggle loop
    if (k.key() == 'l' || k.key() == 'L') {
      loop = !loop;
      std::cout << "Loop: " << (loop ? "ON" : "OFF") << std::endl;
      return true;
    }

    // Select audio file via keys '1'..'9' (1 selects first file)
    char c = k.key();
    if (c >= '1' && c <= '9') {
      int idx = static_cast<int>(c - '1'); // '1'->0, '2'->1, ...
      if (idx < static_cast<int>(audioFiles.size())) {
        if (idx != selectedFileIndex) {
          selectedFileIndex = idx;
          if (loadAudioFile(audioFiles[selectedFileIndex])) {
            std::cout << "Loaded file [" << selectedFileIndex + 1 << "]: " << audioFiles[selectedFileIndex] << std::endl;
          } else {
            std::cerr << "Failed to load file: " << audioFiles[selectedFileIndex] << std::endl;
          }
        } else {
          std::cout << "Already selected file " << selectedFileIndex + 1 << std::endl;
        }
      } else {
        std::cerr << "No audio file for key '" << c << "' (index " << idx << " out of range)" << std::endl;
      }
      return true;
    }

    return false;
  }

  void onExit() {
    if (displayGUI) imguiShutdown();
  }
};

#endif // MAINPLAYER_HPP