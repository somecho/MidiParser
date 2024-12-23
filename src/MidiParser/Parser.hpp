#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <optional>
#include <stack>
#include <string>
#include <vector>

#include "MidiFile.hpp"
#include "MidiTrack.hpp"
#include "events.hpp"

namespace MidiParser {

using byte = uint8_t;

class Parser {
 public:
  Parser() = default;

  MidiFile parse(const std::string& path);

  static uint32_t vlqto32(std::stack<byte>& s);

 private:
  std::ifstream m_file;

  std::array<byte, 14> m_headerData;
  std::vector<std::vector<byte>> m_trackData;

  uint16_t m_fileFormat;
  uint16_t m_numTracks;
  uint16_t m_tickDivision;
  std::vector<MidiTrack> m_midiTracks;

  void readHeaderData();
  void readTrackData();

  void parseTrackData();
  std::vector<TrackEvent> parseTrackData(size_t trackIndex);

  uint32_t readvlq(std::vector<byte>::iterator& it) const;
  TrackEvent readMetaEvent(std::vector<byte>::iterator& it,
                           uint32_t deltaTime) const;
  SysExEvent readSysExEvent(std::vector<byte>::iterator& it,
                            uint32_t deltaTime) const;
  std::optional<MIDIEvent> readMidiEvent(std::vector<byte>::iterator& it,
                                         uint32_t deltaTime) const;
  std::optional<MIDIEvent> readMidiEvent(std::vector<byte>::iterator& it,
                                         uint32_t deltaTime,
                                         uint8_t runningStatus) const;
};

}  // namespace MidiParser
