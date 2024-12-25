#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace MidiParser {

/**
 * An event whose status is `FF`.
 */
struct MetaEvent {
  uint32_t deltaTime;

  /**
   * The byte that comes after `FF` and determines the Meta Event type
   */
  uint8_t status;

  /**
   * A vector containing the bytes representing the meta event's data. E.g.
   * the size of this vector for an event with the status `0x51` would be 3.
   */
  std::vector<uint8_t> data;
};

/**
 * An event whose status is `0x8n` - `0xEn`, `n` being the 4 bit value
 * representing the channel.
 */
struct MIDIEvent {
  uint32_t deltaTime;

  /**
   * A value between `0x8n` - `0xEn`. System Common Messages have status bytes
   * `0xF1` - `0xF6` and System Real Time Messages have status bytes `0xF8` -
   * `0xFE`. Though the latter two kinds of messages do not usually occur
   * in general MIDI files, it is still possible to encounter them.
   */
  uint8_t status;

  /**
   * For normal MIDI voice messages, `data` is a vector of either size 2 or,
   * less commonly, 1. System Real Time Messages may have `data` vectors
   * longer than this.
   */
  std::vector<uint8_t> data;
};

/**
 * An event whose status is either `F0` or `F7`.
 */
struct SysExEvent {
  uint32_t deltaTime;

  /**
   * A vector of bytes containing the bytes found between the events initial
   * status byte and the end byte `F7`.
   */
  std::vector<uint8_t> data;
};

/**
 * Represents any event found within a MIDI track. Can either be a
 * `MidiParser::MetaEvent`, `MidiParser::MIDIEvent` or `MidiParser::SysExEvent`.
 */
using TrackEvent = std::variant<MetaEvent, MIDIEvent, SysExEvent>;

}  // namespace MidiParser
