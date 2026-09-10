#include "engine/PluginRegistry.hpp"

#include <iostream>
#include <string>

namespace {

std::string argumentValue(int argc, char** argv, const std::string& name) {
  for (int index = 1; index + 1 < argc; index += 1) {
    if (argv[index] == name) return argv[index + 1];
  }
  return "";
}

}  // namespace

int main(int argc, char** argv) {
  const auto command = argc > 1 ? std::string(argv[1]) : "--help";
  if (command == "--self-test") {
    localmixer::engine::PluginRegistry registry;
    registry.allowRoot("/Library/Audio/Plug-Ins/VST3");
    const auto ok = registry.addScanRecord(localmixer::engine::PluginScanRecord{
      .descriptor = {
        .identifier = "self-test",
        .name = "Self Test",
        .version = "1.0",
        .path = "/Library/Audio/Plug-Ins/VST3/SelfTest.vst3",
      },
    });
    std::cout << "{\"scanner\":\"ok\",\"selfTest\":" << (ok ? "true" : "false") << "}\n";
    return ok ? 0 : 1;
  }

  if (command == "--scan") {
    localmixer::engine::PluginRegistry registry;
    registry.allowRoot("/Library/Audio/Plug-Ins/Components");
    registry.allowRoot("/Library/Audio/Plug-Ins/VST3");
    const auto path = argumentValue(argc, argv, "--path");
    const auto identifier = argumentValue(argc, argv, "--identifier");
    const auto ok = registry.addScanRecord(localmixer::engine::PluginScanRecord{
      .descriptor = {
        .identifier = identifier.empty() ? path : identifier,
        .name = path,
        .version = "",
        .path = path,
      },
    });
    const auto& record = registry.cache().back();
    std::cout << "{\"ok\":" << (ok ? "true" : "false")
              << ",\"identifier\":\"" << record.descriptor.identifier
              << "\",\"format\":\"" << localmixer::engine::pluginFormatName(record.descriptor.format)
              << "\",\"error\":\"" << localmixer::engine::pluginScanErrorName(record.error)
              << "\"}\n";
    return ok ? 0 : 2;
  }

  std::cerr << "usage: local-mixer-plugin-scanner [--self-test|--scan --path PATH --identifier ID]\n";
  return 64;
}
