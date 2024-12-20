#pragma once

#include <vector>

#include "MidiTrack.h"

namespace MidiParser {

struct MidiFile {
  uint16_t fileFormat;
  uint16_t numTracks;
  uint16_t tickDivision;
  std::vector<MidiTrack> tracks;
};

}  // namespace MidiParser
