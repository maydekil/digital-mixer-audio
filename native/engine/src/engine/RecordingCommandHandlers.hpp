#pragma once

#include "engine/Recording.hpp"

#include <string>

namespace localmixer::engine {

std::string recordingPlanJsonFields(const std::string& line);
std::string recordingStartJsonFields(const std::string& line, RecordingSession& session);
std::string recordingStopJsonFields(RecordingSession& session);

}  // namespace localmixer::engine
