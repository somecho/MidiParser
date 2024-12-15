#pragma once

#include "File.h"
#include <array>
#include <cstdint>

namespace Midi {

struct HeaderChunk {
  static constexpr std::array<uint8_t, 4> Identifier{/*M*/ 0x4D, /*T*/ 0x54,
                                                     /*h*/ 0x68, /*d*/ 0x64};
  uint32_t length;
  Midi::Format format;
  uint16_t numTracks;
  uint16_t division;
};

} // namespace Midi
