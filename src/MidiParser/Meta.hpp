#pragma once

#include <cstdint>
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

}
