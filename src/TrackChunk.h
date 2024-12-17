#pragma once

#include <array>
#include <cstdint>

namespace Midi {

struct TrackChunk {
  static constexpr std::array<uint8_t, 4> Identifier{/*M*/ 0x4D, /*T*/ 0x54,
                                                     /*r*/ 0x72, /*k*/ 0x6B};
};

} // namespace Midi
