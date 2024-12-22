#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <stack>
#include <string>
#include <vector>

namespace MidiParser {

using byte = uint8_t;

class Parser {
 public:
  Parser() = default;

  void parse(const std::string& path);

  static uint32_t vlqto32(std::stack<byte>& s);

 private:
  std::ifstream m_file;

  std::array<byte, 14> m_headerData;
  std::vector<std::vector<byte>> m_trackData;

  void readHeaderData();
  void readTrackData();
  void parseTrackData();
  void parseTrackData(const std::vector<byte>& data);
};

}  // namespace MidiParser
