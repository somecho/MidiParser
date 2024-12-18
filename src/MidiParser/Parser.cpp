#include "Parser.h"

#include <cmath>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace MidiParser {

Parser::Parser(const std::string &midiFilePath)
    : m_eventRegister(Event::NO_OP),
      m_messageRegister(Event::NO_OP),
      m_channelRegister(UINT8_MAX),
      m_variableLength(UINT32_MAX),
      m_trackCount(0),
      m_numTracks(0),
      m_state(State::NEW),
      m_prevState(State::NEW),
      m_prevEvent(Event::IDENTIFIER),
      m_nextEvent(Event::IDENTIFIER),
      m_scanner(midiFilePath),
      m_actions(bindActions(*this)) {}

void Parser::parse() {
  if (m_state != State::NEW) {
    throw std::runtime_error(
        "Not in a fresh state. Parser has already been used.");
  }
  while (m_state != State::FINISHED) {
    processEvent(m_nextEvent);
  }
}

void Parser::processEvent(Event event) {
  if (StateEvents.find({m_state, event}) == StateEvents.end()) {
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

void Parser::setNextEvent(Event event) {
  m_prevEvent = m_nextEvent;
  m_nextEvent = event;
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
  setNextEvent(Event::FIXED_LENGTH);
}

void Parser::onFixedLength() {
  auto length = ntohl(m_scanner.scan<uint32_t>());
  setState(State::FIXED_LENGTH_FOUND);
  if (m_prevState == State::HEADER_ID_FOUND) {
    std::cout << "Header chunk length: " << length << std::endl;
    if (length != 6) {
      throw std::runtime_error("The length of the header chunk must be 6.");
    }
    setNextEvent(Event::FILE_FORMAT);
  } else if (m_prevState == State::TRACK_ID_FOUND) {
    std::cout << "Track chunk length: " << length << std::endl;
    setNextEvent(Event::VARIABLE_TIME);
  }
}

void Parser::onFileFormat() {
  auto fileFormat = ntohs(m_scanner.scan<uint16_t>());
  std::cout << "Midi file format number: " << fileFormat << std::endl;
  setState(State::FILE_FORMAT_FOUND);
  setNextEvent(Event::NUM_TRACKS);
}

void Parser::onNumTracks() {
  m_numTracks = ntohs(m_scanner.scan<uint16_t>());
  std::cout << "Number of tracks: " << m_numTracks << std::endl;
  setState(State::NUM_TRACKS_FOUND);
  setNextEvent(Event::TICKS);
}

void Parser::onTicks() {
  auto ticks = ntohs(m_scanner.scan<int16_t>());
  std::cout << "Number of ticks: " << ticks << std::endl;
  setState(State::HEADER_CHUNK_READ);
  setNextEvent(Event::IDENTIFIER);
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
  bool isLastByte = 0x00 <= byte && byte <= 0x7F;  // Bytes where the MSB = 0
  std::cout << std::format("Found byte: {:02X}", byte) << std::endl;

  if (isLastByte) {
    std::cout << "last byte" << std::endl;
    m_bytesRegister.emplace_back(byte);
    // do something with the full register then clear it
    m_variableLength = variableTo32(m_bytesRegister);
    m_bytesRegister.clear();
    setState(State::VARIABLE_TIME_READ);
    if (m_eventRegister == Event::TEXT) {
      setNextEvent(m_eventRegister);
    } else {
      setNextEvent(Event::VARIABLE_TIME);
    }
    return;
    /* switch (m_eventRegister) { */
    /* case Event::TEXT: */
    /*   setNextEvent(m_eventRegister); */
    /*   break; */
    /* default: */
    /*   setNextEvent(Event::VARIABLE_TIME); */
    /*   break; */
    /* } */
  } else if (m_state != State::VARIABLE_TIME_READ) {
    m_bytesRegister.emplace_back(byte);
    setState(State::READING_VARIABLE_TIME);
    setNextEvent(Event::VARIABLE_TIME);
    return;
  }
  if (isMeta) {
    std::cout << "Found meta event" << std::endl;
    setState(State::META_FOUND);
    setNextEvent(Event::META_TYPE);
    return;
  } else {
    // determine which midi event
    std::cout << "Found midi event" << std::endl;
    uint8_t messageMask = 0b11110000;
    uint8_t channelMask = 0b00001111;
    m_messageRegister = static_cast<Event>(byte & messageMask);
    m_channelRegister = byte & channelMask;
    std::cout << std::format("Midi message: {:08B}", byte) << std::endl;
    setState(State::MIDI_FOUND);
    if (MidiMessages.find(m_messageRegister) != MidiMessages.end()) {
      setNextEvent(m_messageRegister);
      return;
      /* setNextEvent(Event::META_TYPE); */
    } else {
      std::cout << "not implemented" << std::endl;
      setState(State::FINISHED);
      return;
    }
  }
}

void Parser::onMetaType() {
  auto typeByte = m_scanner.scan<uint8_t>();
  std::cout << std::format("Found byte: {:02X}", typeByte) << std::endl;
  auto metaType = Event(typeByte);
  if (MetaHandlers.find(metaType) == MetaHandlers.end()) {
    std::cout << "Not yet implemented" << std::endl;
    setState(State::FINISHED);
  } else {
    setState(MetaHandlers.at(metaType));
    setNextEvent(metaType);
  }
}

void Parser::onSetTempo() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 3) {
    throw std::runtime_error("Length of the set tempo meta event must be 3.");
  }
  auto buffer = m_scanner.scan<3>();
  auto tempo = variableTo32(buffer);
  std::cout << "Tempo: " << tempo << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
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
  setNextEvent(Event::VARIABLE_TIME);
}

void Parser::onEndOfTrack() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 0) {
    throw std::runtime_error("Length of the end of track event must be 0.");
  }
  m_trackCount += 1;
  setState(State::TRACK_READ);
  setNextEvent(Event::IDENTIFIER);
  if (m_trackCount >= m_numTracks) {
    std::cout << "END OF MIDI FILE" << std::endl;
    setState(State::FINISHED);
    setNextEvent(Event::NO_OP);
  }
}

void Parser::onText() {
  if (m_eventRegister != Event::TEXT) {
    m_eventRegister = Event::TEXT;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    std::cout << "TEXT EVENT" << std::endl;
    std::cout << std::string_view(reinterpret_cast<char *>(buffer.data()),
                                  m_variableLength)
              << std::endl;
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
  }
}

void Parser::onMIDIControlChange() {
  uint8_t controllerNumber = m_scanner.scan<uint8_t>();
  uint8_t value = m_scanner.scan<uint8_t>();
  std::cout << std::format("Control Change: Controller - {}, Value - {}",
                           controllerNumber, value)
            << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
}

void Parser::onMIDINoteOn() {
  uint8_t key = m_scanner.scan<uint8_t>();
  uint8_t velocity = m_scanner.scan<uint8_t>();
  std::cout << std::format("Note on: Note - {}, Velocity - {}", key, velocity)
            << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
}

void Parser::onMIDIProgramChange() {
  uint8_t program = m_scanner.scan<uint8_t>();
  std::cout << std::format("Program change: {}", program) << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
}

void Parser::onMIDINoteOff() {
  uint8_t key = m_scanner.scan<uint8_t>();
  uint8_t velocity = m_scanner.scan<uint8_t>();
  std::cout << std::format("Note off: Note - {}, Velocity - {}", key, velocity)
            << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
}

void Parser::onMIDIPitchBend() {
  auto data = m_scanner.scan<2>();
  uint8_t value = (data[0] << 7) + data[1];

  std::cout << std::format("Pitchbend: {}", value) << std::endl;
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
}

}  // namespace MidiParser
