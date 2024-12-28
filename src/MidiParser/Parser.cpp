#include <format>
#include <iostream>
#include <stdexcept>

#include "Parser.hpp"
#include "read.hpp"

namespace MidiParser {

MidiFile Parser::parse(const std::string& path) {
  m_file = std::ifstream(path, std::ios::binary);
  if(!m_file){
    throw std::ios_base::failure("Unable to open file.");
  }
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
}

void Parser::readHeaderData() {
  m_file.read(reinterpret_cast<char*>(m_headerData.data()),
              m_headerData.size());
  m_fileFormat = 0 | (m_headerData[8] << 8) | m_headerData[9];
  m_numTracks = 0 | (m_headerData[10] << 8) | m_headerData[11];
  m_tickDivision = 0 | (m_headerData[12] << 8) | m_headerData[13];
  m_trackData.resize(m_numTracks);
  m_midiTracks.resize(m_numTracks);
}

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
}

void Parser::parseAllTrackData() {
  for (size_t i = 0; i < m_trackData.size(); ++i) {
    m_threadPool.emplace_back(std::thread(&Parser::parseTrackData, this, i));
  }
}

std::vector<TrackEvent> Parser::parseTrackData(size_t trackIndex) {
  auto& data = m_trackData.at(trackIndex);
  std::vector<byte>::iterator it = data.begin();
  auto& trackEvents = m_midiTracks.at(trackIndex).events;
  bool endOfTrackFound = false;
  uint8_t runningStatus = 0;
  while (!endOfTrackFound) {
    uint32_t deltaTime = readvlq(it);
    uint8_t identifier = *++it;
    switch (identifier) {
      case 0xFF: {  // Meta Event
        auto e = readMetaEvent(it, deltaTime);
        if (e.status == 0x2F) {
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
        auto e = readMidiEvent(it, deltaTime);
        if (e) {
          runningStatus = identifier;
          trackEvents.emplace_back(e.value());
          break;
        }
        e = readMidiEvent(it, deltaTime, runningStatus);
        if (e) {
          trackEvents.emplace_back(e.value());
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
}

}  // namespace MidiParser
