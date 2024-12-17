#pragma once

#include "Scanner.h"
#include <array>
#include <cstdint>
#include <functional>
#include <netinet/in.h>
#include <set>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MidiParser {

struct pairHash {
  template <typename T1, typename T2>
  std::size_t operator()(const std::pair<T1, T2> &pair) const {
    return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
  }
};

inline std::array<uint8_t, 4> HeaderID{/*M*/ 0x4D, /*T*/ 0x54,
                                       /*h*/ 0x68, /*d*/ 0x64};

inline std::array<uint8_t, 4> TrackID{/*M*/ 0x4D, /*T*/ 0x54,
                                      /*r*/ 0x72, /*k*/ 0x6B};

class Parser {
public:
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
    MIDI_NOTE_ON = 0b10010000,
    MIDI_NOTE_OFF = 0b10000000,
    MIDI_PROGRAM_CHANGE = 0b11000000,
    MIDI_CONTROL_CHANGE = 0b10110000,
    MIDI_PITCH_BEND = 0b11100000,
  };

  explicit Parser(const std::string &midiFilePath);
  void parse();
  void processEvent(Event event);

  static inline uint32_t variableTo32(std::span<uint8_t> buffer) {
    uint32_t value = 0;
    for (size_t i = 0; i < buffer.size(); ++i) {
      auto byte = static_cast<uint32_t>(buffer[i]);
      auto shiftVal = (buffer.size() - 1 - i) * 8;
      value |= byte << shiftVal;
    }
    return value;
  }

private:
  Scanner m_scanner;
  State m_state;
  State m_prevState;
  Event m_prevEvent;
  Event m_nextEvent;
  std::set<std::pair<State, Event>> m_stateEvents;
  std::set<Event> m_midiMessages;
  std::unordered_map<Event, State> m_metaHandlers;
  std::unordered_map<Event, std::function<void()>> m_actions;
  Event m_eventRegister;
  Event m_messageRegister;
  uint8_t m_channelRegister;
  std::vector<uint8_t> m_bytesRegister;
  uint32_t m_variableLength;
  uint16_t trackCount;
  uint16_t numTracks;

  void setState(State state);
  void setNextEvent(Event event);
  void onIdentifier();
  void onFixedLength();
  void onFileFormat();
  void onNumTracks();
  void onTicks();
  void onVariableTime();
  void onMetaType();
  void onSetTempo();
  void onTimeSignature();
  void onEndOfTrack();
  void onText();
  void onMIDINoteOn();
  void onMIDIControlChange();
  void onMIDIProgramChange();
  void onMIDINoteOff();
  void onMIDIPitchBend();
};

} // namespace MidiParser
