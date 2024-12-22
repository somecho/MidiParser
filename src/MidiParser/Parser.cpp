#include <netinet/in.h>
#include <cassert>
#include <format>
#include <iostream>
#include <iterator>
#include <set>
#include <stack>
#include <stdexcept>
#include <variant>

#include "Meta.hpp"
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

void Parser::parseTrackData(std::vector<byte>& data) {
  std::vector<byte>::iterator it = data.begin();
  std::vector<TrackEvent> trackEvents;
  bool endOfTrackFound = false;
  bool firstByteRead = false;
  uint8_t runningStatus = 0;
  uint8_t maskedRunning = runningStatus & 0b11110000;
  while (!endOfTrackFound) {
    uint8_t deltaTime = firstByteRead ? readvlq(++it) : readvlq(it);
    if (!firstByteRead) {
      firstByteRead = true;
    }

    uint8_t identifier = *++it;
    switch (identifier) {
      case 0xFF: {  // Meta Event
        auto e = readMetaEvent(it, deltaTime);
        if (std::holds_alternative<MetaEndOfTrackEvent>(e)) {
          endOfTrackFound = true;
        }
        trackEvents.emplace_back(e);
        break;
      }
      case 0xF0:  // SysEx Event
        readSysExEvent(it);
        break;
      case 0xF7:  // SysEx Event
        readSysExEvent(it);
        break;
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
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }
        if (single.contains(b) || single.contains(masked)) {
          std::cout << "MIDI skip 1" << std::endl;
          std::advance(it, 1);
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }

        if (two.contains(b) || two.contains(masked)) {
          std::cout << "MIDI skip 2" << std::endl;
          std::advance(it, 2);
          runningStatus = identifier;
          maskedRunning = runningStatus & 0b11110000;
          break;
        }
        // no status byte encountered
        if (noLength.contains(runningStatus)) {
          std::advance(it, -1);
          break;
        }
        if (single.contains(runningStatus) || single.contains(maskedRunning)) {
          break;
        }
        if (two.contains(runningStatus) || two.contains(maskedRunning)) {
          std::cout << "MIDI" << std::endl;
          std::advance(it, 1);
          break;
        }
        throw std::runtime_error("SOMETHING WRONG");
        endOfTrackFound = true;
        break;
    }
  }
  std::cout << "Finished parsing track" << std::endl;
  assert(++it == data.end());
}  // Parser::parseTrackData

uint32_t Parser::readvlq(std::vector<byte>::iterator& it) const {
  std::stack<byte> s;
  uint8_t currByte = *it;
  s.push(currByte);
  while ((currByte & 0b10000000) != 0x0) {
    currByte = *++it;
    s.push(currByte);
  }
  return vlqto32(s);
}  // Parser::readDeltaTime

TrackEvent Parser::readMetaEvent(std::vector<byte>::iterator& it,
                                 uint32_t deltaTime) {
  uint8_t metaType = *++it;
  uint32_t length = 1;
  switch (static_cast<Meta>(metaType)) {
    case Meta::SEQUENCE_NUMBER: {
      uint32_t length = readvlq(++it);
      std::advance(it, length);
      if (length == 0) {
        return MetaSequenceNumberEvent{.deltaTime = deltaTime,
                                       .numberOmmited = true};
      } else {
        uint16_t num;
        m_file.read(reinterpret_cast<char*>(&num), sizeof(num));
        return MetaSequenceNumberEvent{.deltaTime = deltaTime,
                                       .number = ntohs(num),
                                       .numberOmmited = false};
      }
    }
    case Meta::TEXT: {
      uint32_t length = readvlq(++it);
      std::vector<byte> data(length);
      m_file.read(reinterpret_cast<char*>(data.data()), data.size());
      std::advance(it, length);
      return MetaTextEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::END_OF_TRACK:
      std::advance(it, 1);  // skip the length byte
      return MetaEndOfTrackEvent{deltaTime};
    default:
      uint32_t length = readvlq(++it);
      std::advance(it, length);
  }
  return MetaTextEvent{};  // TODO
}  // Parser::readMetaEvent

void Parser::readSysExEvent(std::vector<byte>::iterator& it) const {
  uint8_t next = *++it;
  while (next != 0xF7) {
    next = *++it;
  }
};  // Parser::readSysexEvent

}  // namespace MidiParser
