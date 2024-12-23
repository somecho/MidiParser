#pragma once

#include <vector>

#include "events.hpp"

namespace MidiParser {

struct MidiTrack {
  uint32_t length;
  std::vector<TrackEvent> events;
};

}  // namespace MidiParser
