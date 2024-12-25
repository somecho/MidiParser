#pragma once

#include <vector>

#include "MidiTrack.hpp"

namespace MidiParser {

/**
 * A representation of a MIDI file used as the output of
 * MidiParser::Parser::parse.
 */
struct MidiFile {

  /**
   * The format of a MIDI file. Can be either `0`, `1` or `2`.
   *
   * `0` - single track
   * `1` - multi track
   * `2` - multi song
   */
  uint16_t fileFormat;

  /**
   * The number of track chunks to be found as declared in the header chunk.
   */
  uint16_t numTracks;

  /**
   * Unit of time used in the delta times. If positive, represents ticks per
   * beat. If negative, delta times are SMPTE compatible units.
   */
  uint16_t tickDivision;

  /**
   * A vector containing parsed MidiTracks, the size of which should match
   * `MidiFile.numTracks`.
   */
  std::vector<MidiTrack> tracks;
};

}  // namespace MidiParser
