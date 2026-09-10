#include "NativeAudioPlayer.hpp"
#include "SoundPadSynth.hpp"
#include "WavWriter.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace {

void printUsage() {
  std::cerr << "usage: sound-pad-helper --play <pad-id> [asset-dir] | --render-assets <asset-dir> | --validate | --list\n";
}

std::filesystem::path defaultAssetDir() {
  return std::filesystem::current_path() / "assets" / "sound-pads";
}

std::filesystem::path assetPath(const std::string& padId, const std::filesystem::path& assetDir) {
  return assetDir / (padId + ".wav");
}

}  // namespace

int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--list") {
    for (const auto& pad : soundpad::supportedPads()) std::cout << pad << "\n";
    return 0;
  }

  if (argc == 2 && std::string(argv[1]) == "--validate") {
    for (const auto& pad : soundpad::supportedPads()) {
      const auto sound = soundpad::renderPad(pad);
      if (sound.interleaved.empty()) return 2;
    }
    std::cout << "sound-pad-helper validate ok\n";
    return 0;
  }

  if (argc == 3 && std::string(argv[1]) == "--render-assets") {
    const std::filesystem::path assetDir = argv[2];
    std::filesystem::create_directories(assetDir);
    for (const auto& pad : soundpad::supportedPads()) {
      if (!soundpad::writeWav(soundpad::renderPad(pad), assetPath(pad, assetDir))) {
        std::cerr << "failed to write sound pad asset: " << pad << "\n";
        return 1;
      }
    }
    std::cout << "sound-pad assets written to " << assetDir.string() << "\n";
    return 0;
  }

  if ((argc != 3 && argc != 4) || std::string(argv[1]) != "--play") {
    printUsage();
    return 64;
  }

  const std::string padId = argv[2];
  if (!soundpad::isSupportedPad(padId)) {
    std::cerr << "unsupported sound pad: " << padId << "\n";
    return 65;
  }

  const std::filesystem::path assetDir = argc == 4 ? std::filesystem::path(argv[3]) : defaultAssetDir();

  try {
    if (soundpad::playFileBlocking(assetPath(padId, assetDir))) return 0;
    std::cerr << soundpad::lastPlaybackError() << "\n";
    return 1;
  } catch (const std::exception& error) {
    std::cerr << error.what() << "\n";
    return 1;
  }
}
