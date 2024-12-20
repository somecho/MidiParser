#pragma once

#include <cstdint>
#include <string>
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

struct BaseMetaTextEvent : BaseTrackEvent {
  std::string text;
};

struct MetaTextEvent : BaseMetaTextEvent {};

struct MetaCopyrightNoticeEvent : BaseMetaTextEvent {};

struct MetaTrackNameEvent : BaseMetaTextEvent {};

struct MetaInstrumentNameEvent : BaseMetaTextEvent {};

struct MetaLyricEvent : BaseMetaTextEvent {};

struct MetaMarkerEvent : BaseMetaTextEvent {};

struct MetaCueEvent : BaseMetaTextEvent {};

struct MetaChannelEvent : BaseTrackEvent {
  uint8_t channel;
};

struct MetaSetTempoEvent : BaseTrackEvent {
  uint32_t tempo;
};

struct MetaSMPTEOffsetEvent : BaseTrackEvent {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t frames;
  uint8_t subframes;
};

struct MetaTimeSignatureEvent : BaseTrackEvent {
  uint8_t numerator;
  uint8_t denominator;
  uint8_t clocksPerClick;
  uint8_t quarterDivision;
};

struct MetaKeySignatureEvent : BaseTrackEvent {
  int8_t signature;
  uint8_t mode;
};

using MIDIEvent =
    std::variant<MIDIControlChangeEvent, MIDINoteOnEvent, MIDINoteOffEvent,
                 MIDIPolyAftertouchEvent, MIDIProgramChangeEvent,
                 MIDIAftertouchEvent, MIDIPitchBendEvent>;

using MetaEvent =
    std::variant<MetaTextEvent, MetaCopyrightNoticeEvent,
                 MetaInstrumentNameEvent, MetaTrackNameEvent, MetaLyricEvent,
                 MetaMarkerEvent, MetaCueEvent, MetaChannelEvent,
                 MetaSetTempoEvent, MetaSMPTEOffsetEvent,
                 MetaTimeSignatureEvent, MetaKeySignatureEvent>;

using TrackEvent = std::variant<MIDIEvent, MetaEvent>;

}  // namespace MidiParser
