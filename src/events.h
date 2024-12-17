#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace Midi {

namespace Event {

enum class Type : uint8_t { META = 0xFF };

struct TrackBase {
  uint32_t deltaTime;
};

struct Meta : TrackBase {
  enum class Type : uint32_t {
    TEXT = 0x01,
    SET_TEMPO = 0x51,
    END_OF_TRACK = 0x2F
  };
  /// Variable Type
  uint32_t length;
};

struct SequenceNumber : Meta {
  uint16_t sequenceNumber;
};

struct Text : Meta {
  /// Can use any encoding
  std::string text;
};

struct CopyrightNotice : Meta {
  /// ASCII encoding
  std::string text;
};

/// Also sequence name
struct TrackName : Meta {
  std::string text;
};

struct InstrumentName : Meta {
  std::string text;
};

struct Lyric : Meta {
  std::string text;
};

struct Marker : Meta {
  std::string text;
};

struct CuePoint : Meta {
  std::string text;
};

struct ChannelPrefix : Meta {
  uint8_t channelNumber;
};

struct EndOfTrack : Meta {};

struct SetTempo : Meta {
  /// Microseconds per beat
  uint32_t tempo;
  friend std::ostream &operator<<(std::ostream &os, SetTempo e) {
    return os << "SetTempo Event" << std::endl
              << "--------------" << std::endl
              << "Length: " << e.length << std::endl
              << "DeltaTime: " << e.deltaTime << std::endl
              << "Tempo: " << e.tempo << " microseconds";
  }
};

struct SMPTEOffset : Meta {
  /// 4 bits SMPTE format and 4 bits hour
  uint8_t hour;
  uint8_t minute;
  uint8_t seconds;
  uint8_t frames;
  /// 1/100 of a frame
  uint8_t subframes;
};

struct TimeSignature : Meta {
  uint8_t numerator;
  /// negative power of 2
  uint8_t denominator;
  uint8_t numMidiClocks;
  uint8_t num32ndNotes;
};

struct KeySignature : Meta {
  enum class Mode { MAJOR = 0, MINOR = 1 };
  int8_t key;
  Mode mode;
};

struct SequencerSpecific : Meta {
  std::vector<uint8_t> data;
};

using Track = std::variant<SequenceNumber, Text, CopyrightNotice, TrackName,
                           InstrumentName, Lyric, Marker, CuePoint,
                           ChannelPrefix, EndOfTrack, SetTempo, SMPTEOffset,
                           TimeSignature, KeySignature, SequencerSpecific>;

} // namespace Event
} // namespace Midi
