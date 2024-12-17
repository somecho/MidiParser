#include "Parser.h"
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>

namespace MidiParser {

Parser::Parser(const std::string &midiFilePath)
    : m_scanner(midiFilePath), m_state(State::NEW), m_prevState(State::NEW) {
  m_stateEvents = {
      {State::NEW, Event::IDENTIFIER},
      {State::HEADER_ID_FOUND, Event::FIXED_LENGTH},
      {State::FIXED_LENGTH_FOUND, Event::FILE_FORMAT},
      {State::FILE_FORMAT_FOUND, Event::NUM_TRACKS},
      {State::NUM_TRACKS_FOUND, Event::TICKS},
      {State::HEADER_CHUNK_READ, Event::IDENTIFIER},
      {State::TRACK_ID_FOUND, Event::FIXED_LENGTH},
      {State::FIXED_LENGTH_FOUND, Event::VARIABLE_TIME},
      {State::READING_VARIABLE_TIME, Event::VARIABLE_TIME},
      {State::META_FOUND, Event::META_TYPE},
      {State::META_SET_TEMPO_FOUND, Event::SET_TEMPO},
      {State::META_TIME_SIGNATURE_FOUND, Event::TIME_SIGNATURE},
      {State::EVENT_READ, Event::VARIABLE_TIME},
      {State::VARIABLE_TIME_READ, Event::VARIABLE_TIME},
      {State::END_OF_TRACK_FOUND, Event::END_OF_TRACK},
      {State::TRACK_READ, Event::IDENTIFIER},
  };
  m_actions = {
      {Event::IDENTIFIER, [this]() { onIdentifier(); }},
      {Event::FIXED_LENGTH, [this]() { onFixedLength(); }},
      {Event::FILE_FORMAT, [this]() { onFileFormat(); }},
      {Event::NUM_TRACKS, [this]() { onNumTracks(); }},
      {Event::TICKS, [this]() { onTicks(); }},
      {Event::VARIABLE_TIME, [this]() { onVariableTime(); }},
      {Event::META_TYPE, [this]() { onMetaType(); }},
      {Event::SET_TEMPO, [this]() { onSetTempo(); }},
      {Event::TIME_SIGNATURE, [this]() { onTimeSignature(); }},
      {Event::END_OF_TRACK, [this]() { onEndOfTrack(); }},
  };
  m_metaHandlers = {
      {Event::SET_TEMPO, State::META_SET_TEMPO_FOUND},
      {Event::TIME_SIGNATURE, State::META_TIME_SIGNATURE_FOUND},
      {Event::END_OF_TRACK, State::END_OF_TRACK_FOUND},
  };
}

void Parser::parse() {
  if (m_state != State::NEW) {
    throw std::runtime_error(
        "Not in a fresh state. Parser has already been used.");
  }
  processEvent(Event::IDENTIFIER);
}

void Parser::processEvent(Event event) {
  if (m_stateEvents.find({m_state, event}) == m_stateEvents.end()) {
    throw std::runtime_error("Unable to process event given current state.");
  }

  if (m_actions.find(event) == m_actions.end()) {
    throw std::runtime_error("No action for this event.");
  }
  m_actions[event]();
}

void Parser::setState(State state) {
  m_prevState = m_state;
  m_state = state;
}

void Parser::onIdentifier() {
  auto identifier = m_scanner.scan<4>();

  if (std::equal(HeaderID.begin(), HeaderID.end(), identifier.begin())) {
    std::cout << "Midi Header Identifier Found" << std::endl;
    setState(State::HEADER_ID_FOUND);
  } else if (std::equal(TrackID.begin(), TrackID.end(), identifier.begin())) {
    std::cout << "Midi Track Identifier Found" << std::endl;
    setState(State::TRACK_ID_FOUND);
  } else {
    throw std::runtime_error("Identifier is not MThd or MTrk");
  }
  processEvent(Event::FIXED_LENGTH);
}

void Parser::onFixedLength() {
  auto length = ntohl(m_scanner.scan<uint32_t>());
  setState(State::FIXED_LENGTH_FOUND);
  if (m_prevState == State::HEADER_ID_FOUND) {
    std::cout << "Header chunk length: " << length << std::endl;
    if (length != 6) {
      throw std::runtime_error("The length of the header chunk must be 6.");
    }
    processEvent(Event::FILE_FORMAT);
  } else if (m_prevState == State::TRACK_ID_FOUND) {
    std::cout << "Track chunk length: " << length << std::endl;
    processEvent(Event::VARIABLE_TIME);
  }
}

void Parser::onFileFormat() {
  auto fileFormat = ntohs(m_scanner.scan<uint16_t>());
  std::cout << "Midi file format number: " << fileFormat << std::endl;
  setState(State::FILE_FORMAT_FOUND);
  processEvent(Event::NUM_TRACKS);
}

void Parser::onNumTracks() {
  auto n = ntohs(m_scanner.scan<uint16_t>());
  std::cout << "Number of tracks: " << n << std::endl;
  setState(State::NUM_TRACKS_FOUND);
  processEvent(Event::TICKS);
}

void Parser::onTicks() {
  auto ticks = ntohs(m_scanner.scan<int16_t>());
  std::cout << "Number of ticks: " << ticks << std::endl;
  setState(State::HEADER_CHUNK_READ);
  processEvent(Event::IDENTIFIER);
}

void Parser::onVariableTime() {
  if (m_state != State::READING_VARIABLE_TIME && !m_bytesRegister.empty()) {
    throw std::runtime_error(
        "Trying to parse variable time but bytes register is not empty.");
  }
  if (m_bytesRegister.size() > 4) {
    throw std::runtime_error(
        "Error parsing variable time. Reading more than 4 bytes.");
  }
  auto byte = m_scanner.scan<uint8_t>();
  bool isMeta = byte == 0xFF;
  bool isSysex = byte == 0xF0 || byte == 0xF7;
  bool isLastByte = 0x00 <= byte && byte <= 0x7F; // Bytes where the MSB = 0
  std::cout << std::format("Found byte: {:02X}", byte) << std::endl;

  if (isLastByte) {
    m_bytesRegister.emplace_back(byte);
    // do something with the full register then clear it
    m_bytesRegister.clear();
    setState(State::VARIABLE_TIME_READ);
    processEvent(Event::VARIABLE_TIME);
  } else if (m_state != State::VARIABLE_TIME_READ) {
    m_bytesRegister.emplace_back(byte);
    setState(State::READING_VARIABLE_TIME);
    processEvent(Event::VARIABLE_TIME);
  }
  if (isMeta) {
    std::cout << "Found meta event" << std::endl;
    setState(State::META_FOUND);
    processEvent(Event::META_TYPE);
  }
}

void Parser::onMetaType() {
  auto typeByte = m_scanner.scan<uint8_t>();
  std::cout << std::format("Found byte: {:02X}", typeByte) << std::endl;
  auto metaType = Event(typeByte);
  if (m_metaHandlers.find(metaType) == m_metaHandlers.end()) {
    std::cout << "Not yet implemented" << std::endl;
  } else {
    setState(m_metaHandlers[metaType]);
    processEvent(metaType);
  }
}

void Parser::onSetTempo() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 3) {
    throw std::runtime_error("Length of the set tempo meta event must be 3.");
  }
  auto tempo = variableTo32<3>(m_scanner.scan<3>());
  std::cout << "Tempo: " << tempo << std::endl;
  setState(State::EVENT_READ);
  processEvent(Event::VARIABLE_TIME);
}

void Parser::onTimeSignature() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 4) {
    throw std::runtime_error(
        "Length of the time signature meta event must be 4.");
  }
  auto numerator = m_scanner.scan<uint8_t>();
  auto denominator = m_scanner.scan<uint8_t>();
  auto clocksPerClick = m_scanner.scan<uint8_t>();
  auto quarterDivision = m_scanner.scan<uint8_t>();
  std::cout << "Time Signature: " << std::endl;
  std::cout << std::format("Numerator: {}", numerator) << std::endl;
  std::cout << "Denominator: " << pow(2, denominator) << std::endl;
  std::cout << std::format("Midi clocks per click: {}", clocksPerClick)
            << std::endl;
  std::cout << std::format("32nds in a quarter: {}", quarterDivision)
            << std::endl;
  setState(State::EVENT_READ);
  processEvent(Event::VARIABLE_TIME);
}

void Parser::onEndOfTrack() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 0) {
    throw std::runtime_error("Length of the end of track event must be 0.");
  }
  setState(State::TRACK_READ);
  processEvent(Event::IDENTIFIER);
}

} // namespace MidiParser
