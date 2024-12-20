#pragma once

#include <cstdint>
#include <variant>

namespace MidiParser {

struct BaseTrackEvent {
  uint32_t deltaTime;
};

struct BaseMidiEvent : BaseTrackEvent {
  uint8_t channel;
};

struct MIDIControlChangeEvent : BaseMidiEvent {
  uint8_t controller;
  uint8_t value;
};

struct MIDINoteOnEvent : BaseMidiEvent {
  uint8_t key;
  uint8_t velocity;
};

struct MIDINoteOffEvent : BaseMidiEvent {
  uint8_t key;
  uint8_t velocity;
};

struct MIDIPolyAftertouchEvent : BaseMidiEvent {
  uint8_t key;
  uint8_t value;
};

struct MIDIProgramChangeEvent : BaseMidiEvent {
  uint8_t program;
};

struct MIDIAftertouchEvent : BaseMidiEvent {
  uint8_t value;
};

struct MIDIPitchBendEvent : BaseMidiEvent {
  uint8_t value;
};

using MIDIEvent =
    std::variant<MIDIControlChangeEvent, MIDINoteOnEvent, MIDINoteOffEvent,
                 MIDIPolyAftertouchEvent, MIDIProgramChangeEvent,
                 MIDIAftertouchEvent, MIDIPitchBendEvent>;

using TrackEvent = std::variant<MIDIEvent>;

}  // namespace MidiParser
