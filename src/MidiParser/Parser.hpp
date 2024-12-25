#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "MidiFile.hpp"
#include "MidiTrack.hpp"

namespace MidiParser {

using byte = uint8_t;

/**
 * The parser provided by MidiParser.
 *
 * Example usage:
 *
 * `MidiParser::Parser parser;`
 * `MidiParser::MidiFile f = parser.parse("path/to/file.mid")`
 * 
 */
class Parser {
 public:
  Parser() = default;

  /**
   * Parses the MIDI file located at `path`. Throws `std::runtime_error` if
   * the MIDI file is invalid or does not exist.
   */
  MidiFile parse(const std::string& path);

 private:
  std::ifstream m_file;
  std::vector<std::thread> m_threadPool;

  std::array<byte, 14> m_headerData;
  std::vector<std::vector<byte>> m_trackData;

  uint16_t m_fileFormat;
  uint16_t m_numTracks;
  uint16_t m_tickDivision;
  std::vector<MidiTrack> m_midiTracks;

  void readHeaderData();
  void readTrackData();

  void parseAllTrackData();
  std::vector<TrackEvent> parseTrackData(size_t trackIndex);
};

}  // namespace MidiParser
