#include <netinet/in.h>
#include <format>
#include <iostream>
#include <set>
#include <stack>
#include <stdexcept>

#include "Parser.hpp"

namespace MidiParser {

void Parser::parse(const std::string& path) {
  m_file = std::ifstream(path, std::ios::binary);
  readHeaderData();
  readTrackData();
  m_file.ignore(1);
  if (!m_file.eof()) {
    throw std::runtime_error(
        "Error reading midi file. There seems to be a length mismatch.");
  }
  m_file.close();
  parseTrackData();
}  // Parser::parse

uint32_t Parser::vlqto32(std::stack<byte>& s) {
  uint32_t out = 0;
  size_t size = s.size();
  for (size_t i = 0; i < size; i++) {
    byte in = s.top();
    out |= (in & 0b01111111) << (7 * i);
    s.pop();
  }
  return out;
}  // Parser::vlqto32

void Parser::readHeaderData() {
  m_file.read(reinterpret_cast<char*>(m_headerData.data()),
              m_headerData.size());
  static size_t numTrackOffset = 10;
  uint16_t numTracks = 0 | (m_headerData[numTrackOffset] << 8) |
                       m_headerData[numTrackOffset + 1];
  m_trackData.resize(numTracks);
}  // Parser::readHeaderData

void Parser::readTrackData() {
  for (size_t i = 0; i < m_trackData.size(); i++) {
    std::array<byte, 4> identifier;
    m_file.read(reinterpret_cast<char*>(identifier.data()), identifier.size());

    uint32_t trackDataLength;
    m_file.read(reinterpret_cast<char*>(&trackDataLength),
                sizeof(trackDataLength));
    trackDataLength = ntohl(trackDataLength);

    auto& track = m_trackData.at(i);
    track.resize(trackDataLength);
    m_file.read(reinterpret_cast<char*>(track.data()), track.size());
  }
}  // Parser::readTrackData

void Parser::parseTrackData() {
  for (size_t i = 0; i < m_trackData.size(); i++) {
    parseTrackData(m_trackData.at(i));
  }

}  // Parser::parseTrackData

void Parser::parseTrackData(const std::vector<byte>& data) {
  size_t curr = 0;
  bool endOfTrackFound = false;
  uint8_t runningStatus = 0;
  uint8_t maskedRunning = runningStatus & 0b11110000;
  while (!endOfTrackFound) {

    //FIND DELTA TIME

    bool deltaFound = false;
    while (!deltaFound) {
      uint8_t currByte = data[curr];
      uint8_t masked = currByte & 0b10000000;
      bool isMSBSet = masked != 0x0;
      if (!isMSBSet) {
        deltaFound = true;
      }
      curr++;
    }

    // CHECK WHAT EVENT
    uint8_t identifier = data[curr];
    curr++;
    switch (identifier) {
      case 0xFF: {
        uint8_t metaType = data[curr];
        if (metaType == 0x2F) {
          std::cout << "END OF TRACK" << std::endl;
          endOfTrackFound = true;
        }
        std::cout << std::format("META: {:02X}", metaType) << std::endl;
        curr++;
        uint32_t length = 0;
        bool lenFound = false;
        /* size_t j = 0; */
        std::stack<uint8_t> s;
        while (!lenFound) {
          uint8_t b = data[curr];
          s.push(b);
          /* length |= b << (j * 7); */
          if ((b & 0b10000000) == 0x0) {
            lenFound = true;
            curr++;
            break;
          }
          curr++;
        }
        const auto size = s.size();
        for (int i = 0; i < size; i++) {
          length |= (s.top() & 0b01111111) << (7 * i);
          s.pop();
        }
        std::cout << length << std::endl;
        curr += length;
        break;
      }  // case 0xFF
      case 0xF7: {
        std::cout << "SYSEX" << std::endl;
        uint8_t next = data[curr];
        while (next != 0xF7) {
          curr++;
          next = data[curr];
        }
        curr++;
        break;
      }
      case 0xF0: {
        std::cout << "SYSEX" << std::endl;
        uint8_t next = data[curr];
        while (next != 0xF7) {
          curr++;
          next = data[curr];
        }
        curr++;
        break;
      }
      default:  // find midi
        static std::set<uint8_t> noLength{0b11110001, 0b11110100, 0b11110101,
                                          0b11110110, 0b11111000, 0b11111001,
                                          0b11111010, 0b11111011, 0b11111100,
                                          0b11111101, 0b11111110};

        static std::set<uint8_t> single{
            0b11110011,
            0b11000000,
            0b11010000,
        };

        static std::set<uint8_t> two{
            0b10000000, 0b10010000, 0b10100000,
            0b11110010, 0b10110000, 0b11100000,
        };
        uint8_t b = identifier;
        auto masked = b & 0b11110000;
        if (noLength.contains(b)) {
          std::cout << "MIDI skip" << std::endl;
          /* curr++; */
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }
        if (single.contains(b) || single.contains(masked)) {
          std::cout << "MIDI skip 1" << std::endl;
          curr += 1;
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }

        if (two.contains(b) || two.contains(masked)) {
          std::cout << "MIDI skip 2" << std::endl;
          curr += 2;
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }
        // no status byte encountered
        if (noLength.contains(runningStatus)) {
          curr--;
          break;
        }
        if (single.contains(runningStatus) || single.contains(maskedRunning)) {
          break;
        }
        if (two.contains(runningStatus) || two.contains(maskedRunning)) {
          std::cout << "MIDI" << std::endl;
          curr += 1;
          break;
        }
        throw std::runtime_error("SOMETHING WRONG");
        endOfTrackFound = true;
        break;
    }
  }
  std::cout << "Finished parsing track" << std::endl;

}  // Parser::parseTrackData

}  // namespace MidiParser
