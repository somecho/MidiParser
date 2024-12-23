#include <netinet/in.h>
#include <format>
#include <iostream>
#include <stdexcept>

#include "Parser.hpp"
#include "enums.hpp"

namespace MidiParser {

MidiFile Parser::parse(const std::string& path) {
  m_file = std::ifstream(path, std::ios::binary);
  readHeaderData();
  readTrackData();
  m_file.ignore(1);
  if (!m_file.eof()) {
    throw std::runtime_error(
        "Error reading midi file. There seems to be a length mismatch.");
  }
  m_file.close();
  parseAllTrackData();
  for (auto& t : m_threadPool) {
    t.join();
  }
  return MidiFile{.fileFormat = m_fileFormat,
                  .numTracks = m_numTracks,
                  .tickDivision = m_tickDivision,
                  .tracks = std::move(m_midiTracks)};
}  // Parser::parse

uint32_t Parser::vlqto32(std::stack<byte>& s) {
  uint32_t out = 0;
  size_t size = s.size();
  for (size_t i = 0; i < size; ++i) {
    byte in = s.top();
    out |= (in & 0b01111111) << (7 * i);
    s.pop();
  }
  return out;
}  // Parser::vlqto32

void Parser::readHeaderData() {
  m_file.read(reinterpret_cast<char*>(m_headerData.data()),
              m_headerData.size());
  m_fileFormat = 0 | (m_headerData[8] << 8) | m_headerData[9];
  m_numTracks = 0 | (m_headerData[10] << 8) | m_headerData[11];
  m_tickDivision = 0 | (m_headerData[12] << 8) | m_headerData[13];
  m_trackData.resize(m_numTracks);
  m_midiTracks.resize(m_numTracks);
}  // Parser::readHeaderData

void Parser::readTrackData() {
  for (size_t i = 0; i < m_trackData.size(); ++i) {
    std::array<byte, 8> metadata;
    m_file.read(reinterpret_cast<char*>(metadata.data()), metadata.size());
    uint32_t trackDataLength = 0 | metadata[4] << 24 | metadata[5] << 16 |
                               metadata[6] << 8 | metadata[7];
    auto& track = m_trackData.at(i);
    track.resize(trackDataLength);
    m_midiTracks.at(i).length = trackDataLength;
    m_file.read(reinterpret_cast<char*>(track.data()), track.size());
  }
}  // Parser::readTrackData

void Parser::parseAllTrackData() {
  for (size_t i = 0; i < m_trackData.size(); ++i) {
    m_threadPool.emplace_back(std::thread(&Parser::parseTrackData, this, i));
  }
}  // Parser::parseTrackData

std::vector<TrackEvent> Parser::parseTrackData(size_t trackIndex) {
  auto& data = m_trackData.at(trackIndex);
  std::vector<byte>::iterator it = data.begin();
  auto& trackEvents = m_midiTracks.at(trackIndex).events;
  bool endOfTrackFound = false;
  uint8_t runningStatus = 0;
  while (!endOfTrackFound) {
    uint8_t deltaTime = readvlq(it);
    uint8_t identifier = *++it;
    switch (identifier) {
      case 0xFF: {  // Meta Event
        auto e = readMetaEvent(it, deltaTime);
        if (std::get<MetaEvent>(e).status == 0x2F) {
          endOfTrackFound = true;
        }
        trackEvents.emplace_back(e);
        break;
      }
      case 0xF0:
      case 0xF7:  // SysEx Event
        trackEvents.emplace_back(readSysExEvent(it, deltaTime));
        break;
      default:  // Midi Event
        if (readMidiEvent(it, deltaTime)) {
          runningStatus = identifier;
          break;
        }
        if (readMidiEvent(it, deltaTime, runningStatus)) {
          break;
        }
        throw std::runtime_error(
            std::format("Unable to read or process byte: {:02X}", *it));
    }
  }
  if (it != data.end()) {
    throw std::runtime_error(
        "Track was marked as finished before reaching the end of the "
        "iterator.");
  }
  return trackEvents;
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
                                 uint32_t deltaTime) const {
  uint8_t metaType = *++it;
  uint32_t length = readvlq(++it);
  std::vector<byte> data;
  for (uint32_t i = 0; i < length; i++) {
    data.emplace_back(*++it);
  }
  std::advance(it, 1);
  return MetaEvent{.deltaTime = deltaTime, .status = metaType, .data = data};
}  // Parser::readMetaEvent

SysExEvent Parser::readSysExEvent(std::vector<byte>::iterator& it,
                                  uint32_t deltaTime) const {
  std::vector<uint8_t> data;
  uint8_t next = *++it;
  while (next != 0xF7) {
    data.emplace_back(next);
    next = *++it;
  }
  std::advance(it, 1);
  return SysExEvent{.deltaTime = deltaTime, .data = data};
}  // Parser::readSysexEvent

std::optional<MIDIEvent> Parser::readMidiEvent(std::vector<byte>::iterator& it,
                                               uint32_t deltaTime) const {
  if (StatusOnlyMIDI.contains(*it)) {
    std::advance(it, 1);
    return MIDIEvent{.deltaTime = deltaTime, .status = *it};
  }
  if (SingleByteMIDI.contains(*it & 0b11110000)) {
    std::advance(it, 1);
    return MIDIEvent{.deltaTime = deltaTime, .status = *it, .data = {*++it}};
  }
  if (DoubleByteMIDI.contains(*it & 0b11110000)) {
    std::advance(it, 1);
    return MIDIEvent{
        .deltaTime = deltaTime, .status = *it, .data = {*++it, *++it}};
  }
  return std::nullopt;
}  // Parser::readMidiEvent

std::optional<MIDIEvent> Parser::readMidiEvent(std::vector<byte>::iterator& it,
                                               uint32_t deltaTime,
                                               uint8_t runningStatus) const {
  if (SingleByteMIDI.contains(runningStatus & 0b11110000)) {
    std::advance(it, 1);
    return MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it}};
  }
  if (DoubleByteMIDI.contains(runningStatus & 0b11110000)) {
    std::advance(it, 1);
    return MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it, *++it}};
  }
  return std::nullopt;
}  // Parser::readMidiEvent

}  // namespace MidiParser
