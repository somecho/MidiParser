#pragma once

#include <vector>

#include "events.h"

namespace MidiParser {

struct MidiTrack {
  std::vector<TrackEvent> events;
};

}  // namespace MidiParser
