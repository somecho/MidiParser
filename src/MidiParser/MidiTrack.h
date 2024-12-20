#pragma once

#include <vector>

#include "events.h"

namespace MidiParser {

struct MidiTrack {
  uint32_t length;
  std::vector<TrackEvent> events;
};

}  // namespace MidiParser
