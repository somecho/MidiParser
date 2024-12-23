#pragma once

#include <cstdint>
#include <set>

namespace MidiParser {

enum class Meta : uint8_t {
  SEQUENCE_NUMBER = 0x00,
  TEXT = 0x01,
  COPYRIGHT_NOTICE = 0x02,
  TRACK_NAME = 0x03,
  INSTRUMENT_NAME = 0x04,
  LYRIC = 0x05,
  MARKER = 0x06,
  CUE = 0x07,
  CHANNEl_PREFIX = 0x20,
  MIDI_PORT = 0x21,
  END_OF_TRACK = 0x2F,
  SET_TEMPO = 0x51,
  SMPTE_OFFSET = 0x54,
  TIME_SIGNATURE = 0x58,
  KEY_SIGNATURE = 0x59,
  SEQUENCER_SPECIFIC = 0x7F
};

inline const std::set<uint8_t> StatusOnlyMIDI{
    0b11110001, 0b11110100, 0b11110101, 0b11110110, 0b11111000, 0b11111001,
    0b11111010, 0b11111011, 0b11111100, 0b11111101, 0b11111110};

inline const std::set<uint8_t> SingleByteMIDI{
    0b11110011,
    0b11000000,
    0b11010000,
};

inline const std::set<uint8_t> DoubleByteMIDI{
    0b10000000, 0b10010000, 0b10100000, 0b11110010, 0b10110000, 0b11100000,
};

}  // namespace MidiParser
