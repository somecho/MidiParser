#include <netinet/in.h>
#include <cassert>
#include <cstdint>
#include <format>
#include <iostream>
#include <iterator>
#include <set>
#include <stack>
#include <stdexcept>
#include <string>
#include <variant>

#include "Parser.hpp"
#include "enums.hpp"
#include "events.hpp"

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
  parseTrackData();
  return MidiFile{.fileFormat = m_fileFormat,
                  .numTracks = m_numTracks,
                  .tickDivision = m_tickDivision,
                  .tracks = std::move(m_midiTracks)};
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
  m_fileFormat = 0 | (m_headerData[8] << 8) | m_headerData[9];
  m_numTracks = 0 | (m_headerData[10] << 8) | m_headerData[11];
  m_tickDivision = 0 | (m_headerData[12] << 8) | m_headerData[13];
  m_trackData.resize(m_numTracks);
  m_midiTracks.resize(m_numTracks);
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
    m_midiTracks.at(i).length = trackDataLength;
    m_file.read(reinterpret_cast<char*>(track.data()), track.size());
  }
}  // Parser::readTrackData

void Parser::parseTrackData() {
  for (size_t i = 0; i < m_trackData.size(); i++) {
    m_midiTracks.at(i).events = parseTrackData(m_trackData.at(i));
  }

}  // Parser::parseTrackData

std::vector<TrackEvent> Parser::parseTrackData(std::vector<byte>& data) {
  std::vector<byte>::iterator it = data.begin();
  std::vector<TrackEvent> trackEvents;
  bool endOfTrackFound = false;
  bool firstByteRead = false;
  uint8_t runningStatus = 0;
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
      case 0xF0:
      case 0xF7:  // SysEx Event
        readSysExEvent(it, deltaTime);
        break;
      default:  // Midi Event
        if (readMidiEvent(it, deltaTime)) {
          runningStatus = identifier;
          break;
        }
        if (readMidiEvent(it, deltaTime, runningStatus)) {
          break;
        }
        throw std::runtime_error("SOMETHING WRONG");
    }
  }
  assert(++it == data.end());
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
  switch (static_cast<Meta>(metaType)) {
    case Meta::SEQUENCE_NUMBER: {
      if (length == 0) {
        return MetaSequenceNumberEvent{.deltaTime = deltaTime,
                                       .numberOmmited = true};
      } else {
        uint16_t num = 0 | (*++it) << 8 | *++it;
        return MetaSequenceNumberEvent{.deltaTime = deltaTime,
                                       .number = ntohs(num),
                                       .numberOmmited = false};
      }
    }
    case Meta::TEXT: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaTextEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::COPYRIGHT_NOTICE: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaCopyrightNoticeEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::TRACK_NAME: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaTrackNameEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::INSTRUMENT_NAME: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaInstrumentNameEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::LYRIC: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaLyricEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::MARKER: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaMarkerEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::CUE: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaCueEvent{.deltaTime = deltaTime, .data = data};
    }
    case Meta::CHANNEl_PREFIX:
      return MetaChannelPrefixEvent{.deltaTime = deltaTime, .channel = *++it};
    case Meta::MIDI_PORT:
      return MetaMIDIPortEvent{.deltaTime = deltaTime, .port = *++it};
    case Meta::END_OF_TRACK:
      return MetaEndOfTrackEvent{deltaTime};
    case Meta::SET_TEMPO:
      return MetaSetTempoEvent{
          .deltaTime = deltaTime,
          .tempo = uint32_t(0 | (*++it << 16) | (*++it << 8) | *++it)};
    case Meta::SMPTE_OFFSET:
      return MetaSMPTEOffsetEvent{.deltaTime = deltaTime,
                                  .hours = *++it,
                                  .minutes = *++it,
                                  .seconds = *++it,
                                  .frames = *++it,
                                  .subframes = *++it};
    case Meta::TIME_SIGNATURE:
      return MetaTimeSignatureEvent{.deltaTime = deltaTime,
                                    .numerator = *++it,
                                    .denominator = *++it,
                                    .clocksPerClick = *++it,
                                    .quarterDivision = *++it};
    case Meta::KEY_SIGNATURE:
      return MetaKeySignatureEvent{.deltaTime = deltaTime,
                                   .signature = static_cast<int8_t>(*++it),
                                   .mode = *++it};

    case Meta::SEQUENCER_SPECIFIC: {
      std::vector<byte> data(++it, (it + length + 1));
      std::advance(it, length - 1);
      return MetaSequencerSpecificEvent{.deltaTime = deltaTime, .data = data};
    }
    default:
      throw std::runtime_error(
          std::format("Unrecognized meta event: {:02x}", metaType));
  }
}  // Parser::readMetaEvent

SysExEvent Parser::readSysExEvent(std::vector<byte>::iterator& it,
                                  uint32_t deltaTime) const {
  std::vector<uint8_t> data;
  uint8_t next = *++it;
  while (next != 0xF7) {
    data.emplace_back(next);
    next = *++it;
  }
  return SysExEvent{.deltaTime = deltaTime, .data = data};
}  // Parser::readSysexEvent

std::optional<MIDIEvent> Parser::readMidiEvent(std::vector<byte>::iterator& it,
                                               uint32_t deltaTime) const {
  if (StatusOnlyMIDI.contains(*it)) {
    return MIDIEvent{.deltaTime = deltaTime, .status = *it};
  }
  if (SingleByteMIDI.contains(*it & 0b11110000)) {
    return MIDIEvent{.deltaTime = deltaTime, .status = *it, .data = {*++it}};
  }

  if (DoubleByteMIDI.contains(*it & 0b11110000)) {
    return MIDIEvent{
        .deltaTime = deltaTime, .status = *it, .data = {*++it, *++it}};
  }
  return std::nullopt;
}  // Parser::readMidiEvent

std::optional<MIDIEvent> Parser::readMidiEvent(std::vector<byte>::iterator& it,
                                               uint32_t deltaTime,
                                               uint8_t runningStatus) const {

  if (SingleByteMIDI.contains(runningStatus & 0b11110000)) {
    return MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it}};
  }
  if (DoubleByteMIDI.contains(runningStatus & 0b11110000)) {
    return MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it, *++it}};
  }
  return std::nullopt;
}  // Parser::readMidiEvent

}  // namespace MidiParser
