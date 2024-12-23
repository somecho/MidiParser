#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace MidiParser {

struct MetaEvent {
  uint32_t deltaTime;
  uint8_t status;
  std::vector<uint8_t> data;
};

struct MIDIEvent {
  uint32_t deltaTime;
  uint8_t status;
  std::vector<uint8_t> data;
};

struct SysExEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

using TrackEvent = std::variant<MetaEvent, MIDIEvent, SysExEvent>;

}  // namespace MidiParser
