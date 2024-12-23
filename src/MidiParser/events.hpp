#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace MidiParser {

struct MetaSequenceNumberEvent {
  uint32_t deltaTime;
  uint16_t number;
  bool numberOmmited;
};

struct MetaTextEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaCopyrightNoticeEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaTrackNameEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaInstrumentNameEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaLyricEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaMarkerEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaCueEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MetaChannelPrefixEvent {
  uint32_t deltaTime;
  uint8_t channel;
};

struct MetaMIDIPortEvent {
  uint32_t deltaTime;
  uint8_t port;
};

struct MetaSetTempoEvent {
  uint32_t deltaTime;
  uint32_t tempo;
};

struct MetaSMPTEOffsetEvent {
  uint32_t deltaTime;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t frames;
  uint8_t subframes;
};

struct MetaTimeSignatureEvent {
  uint32_t deltaTime;
  uint8_t numerator;
  uint8_t denominator;
  uint8_t clocksPerClick;
  uint8_t quarterDivision;
};

struct MetaKeySignatureEvent {
  uint32_t deltaTime;
  int8_t signature;
  uint8_t mode;
};

struct MetaEndOfTrackEvent {
  uint32_t deltaTime;
};

struct MetaSequencerSpecificEvent {
  uint32_t deltaTime;
  std::vector<uint8_t> data;
};

struct MIDIEvent {
  uint32_t deltaTime;
  uint8_t status;
  std::vector<uint8_t> data;
};

using TrackEvent = std::variant<
    // Meta Events
    MetaSequenceNumberEvent,     //
    MetaTextEvent,               //
    MetaCopyrightNoticeEvent,    //
    MetaTrackNameEvent,          //
    MetaInstrumentNameEvent,     //
    MetaLyricEvent,              //
    MetaMarkerEvent,             //
    MetaCueEvent,                //
    MetaEndOfTrackEvent,         //
    MetaChannelPrefixEvent,      //
    MetaMIDIPortEvent,           //
    MetaSetTempoEvent,           //
    MetaSMPTEOffsetEvent,        //
    MetaTimeSignatureEvent,      //
    MetaKeySignatureEvent,       //
    MetaSequencerSpecificEvent,  //
    // Midi Events
    MIDIEvent>;

}  // namespace MidiParser
