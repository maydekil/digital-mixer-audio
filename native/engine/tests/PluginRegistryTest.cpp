#include "engine/PluginRegistry.hpp"

#include <cassert>
#include <iostream>

int main() {
  localmixer::engine::PluginRegistry registry;
  assert(registry.allowRoot("/Library/Audio/Plug-Ins/Components"));
  assert(registry.allowRoot("/Library/Audio/Plug-Ins/VST3"));

  assert(registry.addScanRecord(localmixer::engine::PluginScanRecord{
    .descriptor = {
      .identifier = "com.example.Room",
      .name = "Room",
      .version = "1.0",
      .path = "/Library/Audio/Plug-Ins/Components/Room.component",
    },
  }));
  auto found = registry.find("com.example.Room");
  assert(found.has_value());
  assert(found->format == localmixer::engine::PluginFormat::au);

  assert(registry.addScanRecord(localmixer::engine::PluginScanRecord{
    .descriptor = {
      .identifier = "com.example.Delay",
      .name = "Delay",
      .version = "1.0",
      .path = "/Library/Audio/Plug-Ins/VST3/Delay.vst3",
    },
  }));
  assert(registry.find("com.example.Delay")->format == localmixer::engine::PluginFormat::vst3);

  assert(!registry.addScanRecord(localmixer::engine::PluginScanRecord{
    .descriptor = {
      .identifier = "com.bad.Path",
      .name = "Bad",
      .path = "/tmp/Bad.vst3",
    },
  }));
  assert(registry.cache().back().error == localmixer::engine::PluginScanError::pathNotAllowed);

  assert(!registry.addScanRecord(localmixer::engine::PluginScanRecord{
    .descriptor = {
      .identifier = "com.bad.Format",
      .name = "Bad Format",
      .path = "/Library/Audio/Plug-Ins/VST3/Bad.txt",
    },
  }));
  assert(registry.cache().back().error == localmixer::engine::PluginScanError::unsupportedFormat);

  registry.blacklist("com.example.Crashed");
  assert(!registry.addScanRecord(localmixer::engine::PluginScanRecord{
    .descriptor = {
      .identifier = "com.example.Crashed",
      .name = "Crashed",
      .path = "/Library/Audio/Plug-Ins/VST3/Crashed.vst3",
    },
  }));
  assert(registry.cache().back().error == localmixer::engine::PluginScanError::blacklisted);

  const auto missing = registry.missingPlaceholder("com.example.Missing", "2.0");
  assert(missing.missing);
  assert(missing.identifier == "com.example.Missing");
  assert(missing.version == "2.0");

  std::cout << "plugin registry ok\n";
  return 0;
}
