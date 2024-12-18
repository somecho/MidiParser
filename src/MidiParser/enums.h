#pragma once

#include <cstdint>

namespace MidiParser {

enum class State {
  NEW,
  HEADER_ID_FOUND,
  FIXED_LENGTH_FOUND,
  FILE_FORMAT_FOUND,
  NUM_TRACKS_FOUND,
  HEADER_CHUNK_READ,
  TRACK_ID_FOUND,
  READING_VARIABLE_TIME,
  VARIABLE_TIME_READ,
  META_FOUND,
  META_SET_TEMPO_FOUND,
  META_TIME_SIGNATURE_FOUND,
  META_TEXT_EVENT_FOUND,
  END_OF_TRACK_FOUND,
  EVENT_READ,
  TRACK_READ,
  MIDI_FOUND,
  FINISHED,
};

enum class Event : uint8_t {
  TEXT = 0x01,
  SET_TEMPO = 0x51,
  TIME_SIGNATURE = 0x58,
  END_OF_TRACK = 0x2F,
  IDENTIFIER,
  FIXED_LENGTH,
  FILE_FORMAT,
  NUM_TRACKS,
  TICKS,
  VARIABLE_TIME,
  META_TYPE,
  NO_OP,
  MIDI,
  MIDI_NOTE_OFF = 0b10000000,
  MIDI_NOTE_ON = 0b10010000,
  MIDI_POLY_AFTERTOUCH = 0b10100000,
  MIDI_CONTROL_CHANGE = 0b10110000,
  MIDI_PROGRAM_CHANGE = 0b11000000,
  MIDI_AFTERTOUCH = 0b11010000,
  MIDI_PITCH_BEND = 0b11100000,
};

}  // namespace MidiParser
