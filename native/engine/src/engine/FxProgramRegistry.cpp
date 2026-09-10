#include "engine/FxProgramRegistry.hpp"

#include <algorithm>
#include <array>

namespace localmixer::engine {
namespace {

struct Row {
  FxProgramFamily family;
  std::string_view name;
  double p1;
  double p2;
};

constexpr std::array<Row, 99> kRows{{
  {FxProgramFamily::room, "Tiny Booth", 0.2, 0}, {FxProgramFamily::room, "Dry Studio", 0.3, 2},
  {FxProgramFamily::room, "Small Room", 0.4, 4}, {FxProgramFamily::room, "Vocal Room", 0.5, 6},
  {FxProgramFamily::room, "Warm Room", 0.65, 8}, {FxProgramFamily::room, "Bright Room", 0.8, 10},
  {FxProgramFamily::room, "Wood Room", 0.95, 12}, {FxProgramFamily::room, "Drum Room", 1.1, 14},
  {FxProgramFamily::room, "Medium Room", 1.3, 16}, {FxProgramFamily::room, "Wide Room", 1.5, 18},
  {FxProgramFamily::room, "Large Room", 1.8, 20}, {FxProgramFamily::plate, "Vocal Plate", 1.4, 20},
  {FxProgramFamily::plate, "Short Plate", 0.6, 5}, {FxProgramFamily::plate, "Soft Plate", 0.8, 10},
  {FxProgramFamily::plate, "Bright Plate", 1.0, 12}, {FxProgramFamily::plate, "Warm Plate", 1.2, 15},
  {FxProgramFamily::plate, "Classic Plate", 1.6, 22}, {FxProgramFamily::plate, "Wide Plate", 1.8, 25},
  {FxProgramFamily::plate, "Pop Plate", 2.0, 28}, {FxProgramFamily::plate, "Smooth Plate", 2.3, 30},
  {FxProgramFamily::plate, "Long Plate", 2.8, 35}, {FxProgramFamily::plate, "Epic Plate", 3.5, 40},
  {FxProgramFamily::hall, "Small Hall", 1.2, 10}, {FxProgramFamily::hall, "Vocal Hall", 1.6, 15},
  {FxProgramFamily::hall, "Warm Hall", 1.9, 20}, {FxProgramFamily::hall, "Bright Hall", 2.2, 25},
  {FxProgramFamily::hall, "Concert Hall", 2.5, 30}, {FxProgramFamily::hall, "Wide Hall", 2.8, 35},
  {FxProgramFamily::hall, "Deep Hall", 3.2, 40}, {FxProgramFamily::hall, "Long Hall", 3.8, 45},
  {FxProgramFamily::hall, "Grand Hall", 4.5, 50}, {FxProgramFamily::hall, "Cathedral", 5.5, 60},
  {FxProgramFamily::hall, "Ambient Hall", 7.0, 70}, {FxProgramFamily::slapback, "Micro Slap", 40, 0},
  {FxProgramFamily::slapback, "Tight Slap", 50, 0}, {FxProgramFamily::slapback, "Short Slap", 60, 0},
  {FxProgramFamily::slapback, "Vocal Slap", 75, 0}, {FxProgramFamily::slapback, "Vintage Slap", 90, 5},
  {FxProgramFamily::slapback, "Rock Slap", 100, 8}, {FxProgramFamily::slapback, "Double Slap", 115, 10},
  {FxProgramFamily::slapback, "Warm Slap", 130, 12}, {FxProgramFamily::slapback, "Wide Slap", 145, 15},
  {FxProgramFamily::slapback, "Long Slap", 160, 18}, {FxProgramFamily::slapback, "Echo Slap", 180, 20},
  {FxProgramFamily::stereoDelay, "Stereo 80", 80, 10}, {FxProgramFamily::stereoDelay, "Stereo 120", 120, 12},
  {FxProgramFamily::stereoDelay, "Stereo 160", 160, 15}, {FxProgramFamily::stereoDelay, "Stereo 200", 200, 18},
  {FxProgramFamily::stereoDelay, "Stereo 250", 250, 20}, {FxProgramFamily::stereoDelay, "Stereo 320", 320, 25},
  {FxProgramFamily::stereoDelay, "Stereo 400", 400, 30}, {FxProgramFamily::stereoDelay, "Stereo 500", 500, 35},
  {FxProgramFamily::stereoDelay, "Stereo 650", 650, 40}, {FxProgramFamily::stereoDelay, "Stereo 800", 800, 45},
  {FxProgramFamily::stereoDelay, "Stereo 1000", 1000, 50}, {FxProgramFamily::pingPong, "Ping 100", 100, 10},
  {FxProgramFamily::pingPong, "Ping 150", 150, 12}, {FxProgramFamily::pingPong, "Ping 200", 200, 15},
  {FxProgramFamily::pingPong, "Ping 250", 250, 18}, {FxProgramFamily::pingPong, "Ping 320", 320, 22},
  {FxProgramFamily::pingPong, "Ping 400", 400, 26}, {FxProgramFamily::pingPong, "Ping 500", 500, 30},
  {FxProgramFamily::pingPong, "Ping 600", 600, 35}, {FxProgramFamily::pingPong, "Ping 750", 750, 40},
  {FxProgramFamily::pingPong, "Ping 900", 900, 45}, {FxProgramFamily::pingPong, "Ping 1200", 1200, 50},
  {FxProgramFamily::chorus, "Subtle Chorus", 0.1, 10}, {FxProgramFamily::chorus, "Slow Chorus", 0.15, 12},
  {FxProgramFamily::chorus, "Warm Chorus", 0.2, 15}, {FxProgramFamily::chorus, "Vocal Chorus", 0.3, 18},
  {FxProgramFamily::chorus, "Wide Chorus", 0.4, 22}, {FxProgramFamily::chorus, "Soft Motion", 0.5, 25},
  {FxProgramFamily::chorus, "Pop Chorus", 0.6, 30}, {FxProgramFamily::chorus, "Bright Motion", 0.8, 35},
  {FxProgramFamily::chorus, "Deep Chorus", 1.0, 40}, {FxProgramFamily::chorus, "Fast Chorus", 1.5, 45},
  {FxProgramFamily::chorus, "Liquid Chorus", 2.0, 50}, {FxProgramFamily::phaser, "Subtle Phase", 0.1, 10},
  {FxProgramFamily::phaser, "Slow Phase", 0.15, 15}, {FxProgramFamily::phaser, "Warm Phase", 0.2, 20},
  {FxProgramFamily::phaser, "Vocal Phase", 0.3, 25}, {FxProgramFamily::phaser, "Wide Phase", 0.4, 30},
  {FxProgramFamily::phaser, "Soft Sweep", 0.5, 35}, {FxProgramFamily::phaser, "Classic Phase", 0.6, 40},
  {FxProgramFamily::phaser, "Bright Sweep", 0.8, 45}, {FxProgramFamily::phaser, "Deep Phase", 1.0, 50},
  {FxProgramFamily::phaser, "Fast Phase", 1.5, 55}, {FxProgramFamily::phaser, "Liquid Phase", 2.0, 60},
  {FxProgramFamily::delayPlate, "Vocal Space", 100, 0.6}, {FxProgramFamily::delayPlate, "Short Space", 150, 0.8},
  {FxProgramFamily::delayPlate, "Warm Space", 200, 1.0}, {FxProgramFamily::delayPlate, "Pop Space", 250, 1.2},
  {FxProgramFamily::delayPlate, "Wide Space", 320, 1.4}, {FxProgramFamily::delayPlate, "Ballad Space", 400, 1.8},
  {FxProgramFamily::delayPlate, "Dream Space", 500, 2.2}, {FxProgramFamily::delayPlate, "Long Space", 600, 2.8},
  {FxProgramFamily::delayPlate, "Deep Space", 750, 3.5}, {FxProgramFamily::delayPlate, "Ambient Space", 900, 4.5},
  {FxProgramFamily::delayPlate, "Infinite Mood", 1200, 6.0},
}};

FxProgramRecipe recipeFor(const Row& row) {
  switch (row.family) {
    case FxProgramFamily::room:
      return {"reverb", {{"algorithm", 0, "room"}, {"decay_s", row.p1, "s"}, {"predelay_ms", row.p2, "ms"}, {"damping_hz", 6500, "Hz"}, {"width_pct", 80, "%"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::plate:
      return {"reverb", {{"algorithm", 1, "plate"}, {"decay_s", row.p1, "s"}, {"predelay_ms", row.p2, "ms"}, {"damping_hz", 6000, "Hz"}, {"width_pct", 100, "%"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::hall:
      return {"reverb", {{"algorithm", 2, "hall"}, {"decay_s", row.p1, "s"}, {"predelay_ms", row.p2, "ms"}, {"damping_hz", 5000, "Hz"}, {"width_pct", 100, "%"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::slapback:
      return {"slapback_delay", {{"time_ms", row.p1, "ms"}, {"feedback_pct", row.p2, "%"}, {"hpf_hz", 150, "Hz"}, {"lpf_hz", 6000, "Hz"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::stereoDelay:
      return {"stereo_delay", {{"time_ms", row.p1, "ms"}, {"feedback_pct", row.p2, "%"}, {"hpf_hz", 150, "Hz"}, {"lpf_hz", 6000, "Hz"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::pingPong:
      return {"ping_pong_delay", {{"time_ms", row.p1, "ms"}, {"feedback_pct", row.p2, "%"}, {"hpf_hz", 180, "Hz"}, {"lpf_hz", 5500, "Hz"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::chorus:
      return {"chorus", {{"rate_hz", row.p1, "Hz"}, {"depth_pct", row.p2, "%"}, {"centre_delay_ms", 12, "ms"}, {"feedback_pct", 0, "%"}, {"phase_offset_deg", 90, "deg"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::phaser:
      return {"phaser", {{"rate_hz", row.p1, "Hz"}, {"depth_pct", row.p2, "%"}, {"stages", 6, ""}, {"centre_hz", 900, "Hz"}, {"feedback_pct", 20, "%"}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
    case FxProgramFamily::delayPlate:
      return {"delay_plate_parallel", {{"delay_time_ms", row.p1, "ms"}, {"delay_feedback_pct", 20, "%"}, {"plate_decay_s", row.p2, "s"}, {"plate_predelay_ms", 20, "ms"}, {"plate_damping_hz", 6000, "Hz"}, {"delay_branch_gain", 0.5, ""}, {"plate_branch_gain", 0.5, ""}, {"wet_pct", 100, "%"}, {"output_db", 0, "dB"}}};
  }
  return {};
}

std::string_view macro1Label(FxProgramFamily family) {
  if (family == FxProgramFamily::chorus || family == FxProgramFamily::phaser) return "Rate";
  if (family == FxProgramFamily::delayPlate) return "Delay";
  if (family == FxProgramFamily::slapback || family == FxProgramFamily::stereoDelay || family == FxProgramFamily::pingPong) return "Time";
  return "Decay";
}

std::string_view macro2Label(FxProgramFamily family) {
  if (family == FxProgramFamily::chorus || family == FxProgramFamily::phaser) return "Depth";
  if (family == FxProgramFamily::delayPlate) return "Plate";
  if (family == FxProgramFamily::slapback || family == FxProgramFamily::stereoDelay || family == FxProgramFamily::pingPong) return "Feedback";
  return "Pre-delay";
}

std::string_view macro1Unit(FxProgramFamily family) {
  if (family == FxProgramFamily::chorus || family == FxProgramFamily::phaser) return "Hz";
  if (family == FxProgramFamily::slapback || family == FxProgramFamily::stereoDelay || family == FxProgramFamily::pingPong || family == FxProgramFamily::delayPlate) return "ms";
  return "s";
}

std::string_view macro2Unit(FxProgramFamily family) {
  if (family == FxProgramFamily::chorus || family == FxProgramFamily::phaser || family == FxProgramFamily::slapback || family == FxProgramFamily::stereoDelay || family == FxProgramFamily::pingPong) return "%";
  if (family == FxProgramFamily::delayPlate) return "s";
  return "ms";
}

}  // namespace

std::vector<FxFactoryProgram> buildFxFactoryBank() {
  std::vector<FxFactoryProgram> bank;
  bank.reserve(kRows.size());
  for (std::size_t index = 0; index < kRows.size(); index += 1) {
    const auto& row = kRows[index];
    bank.push_back(FxFactoryProgram{
      .programId = static_cast<std::uint32_t>(index + 1),
      .name = row.name,
      .family = row.family,
      .macro1Label = macro1Label(row.family),
      .macro1Value = row.p1,
      .macro1Unit = macro1Unit(row.family),
      .macro2Label = macro2Label(row.family),
      .macro2Value = row.p2,
      .macro2Unit = macro2Unit(row.family),
      .recipe = recipeFor(row),
    });
  }
  return bank;
}

std::optional<FxFactoryProgram> findFxFactoryProgram(std::uint32_t programId) {
  const auto bank = buildFxFactoryBank();
  const auto it = std::find_if(bank.begin(), bank.end(), [&](const auto& program) {
    return program.programId == programId;
  });
  if (it == bank.end()) return std::nullopt;
  return *it;
}

std::string_view fxProgramFamilyName(FxProgramFamily family) noexcept {
  switch (family) {
    case FxProgramFamily::room: return "Room";
    case FxProgramFamily::plate: return "Plate";
    case FxProgramFamily::hall: return "Hall";
    case FxProgramFamily::slapback: return "Slapback";
    case FxProgramFamily::stereoDelay: return "Stereo Delay";
    case FxProgramFamily::pingPong: return "Ping-pong";
    case FxProgramFamily::chorus: return "Chorus";
    case FxProgramFamily::phaser: return "Phaser";
    case FxProgramFamily::delayPlate: return "Delay + Plate";
  }
  return "Unknown";
}

}  // namespace localmixer::engine
