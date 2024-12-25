#pragma once

#include <vector>

#include "events.hpp"

namespace MidiParser {

/**
 * Created by parsing track chunk data and represents all the events found in a
 * track.
 */
struct MidiTrack {

  /**
   * The length of the track chunk in bytes as declared after the `MTrk`
   * identifier.
   */
  uint32_t length;

  /**
   * Contains all Events in this MIDI track.
   */
  std::vector<TrackEvent> events;
};

}  // namespace MidiParser
