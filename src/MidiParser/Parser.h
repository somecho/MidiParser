#pragma once

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <array>
#include <cstdint>
#include <functional>
#include <set>
#include <span>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Scanner.h"
#include "enums.h"

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
  std::set<Event> m_midiMessages;
  std::unordered_map<Event, State> m_metaHandlers;
  std::unordered_map<Event, std::function<void()>> m_actions;
  Event m_eventRegister;
  Event m_messageRegister;
  uint8_t m_channelRegister;
  std::vector<uint8_t> m_bytesRegister;
  uint32_t m_variableLength;
  uint16_t m_trackCount;
  uint16_t m_numTracks;

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

}  // namespace MidiParser
