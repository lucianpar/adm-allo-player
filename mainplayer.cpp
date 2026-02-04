/*
54-Channel Audio Playback System
Plays back a multichannel audio file with all channels mapped to individual outputs.
Includes GUI controls for playback, pause, loop, and rewind.
Includes real-time dB meters for all 54 channels.
*/

#include "mainplayer.hpp"

int main() {
  MyApp app;

  // Configure audio for 54 output channels, 0 input channels
  // Adjust sample rate and buffer size as needed
  app.configureAudio(48000,  // Sample rate (adjust to match your file)
                     512,    // Buffer size
                     60,     // Output channels
                     0);     // Input channels

  std::cout << "\n=== Audio Configuration ===" << std::endl;
  std::cout << "Output channels: 60" << std::endl;
  std::cout << "Sample rate: 48000 Hz" << std::endl;
  std::cout << "Buffer size: 512 frames" << std::endl;
  std::cout << "\nKeyboard shortcuts:" << std::endl;
  std::cout << "  SPACE - Play/Pause" << std::endl;
  std::cout << "  R - Rewind" << std::endl;
  std::cout << "  L - Toggle Loop" << std::endl;
  std::cout << "\n" << std::endl;

  app.start();
  return 0;
}
