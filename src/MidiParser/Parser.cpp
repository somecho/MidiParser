#include <cmath>
#include <cstdint>
#include <format>
#include <stdexcept>

#include "MidiFile.h"
#include "Parser.h"
#include "events.h"
#include "tables.h"

namespace MidiParser {

Parser::Parser(const std::string& midiFilePath)
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

MidiFile Parser::parse() {
  if (m_state != State::NEW) {
    throw std::runtime_error(
        "Not in a fresh state. Parser has already been used.");
  }
  while (m_state != State::FINISHED) {
    processEvent(m_nextEvent);
  }
  return MidiFile{m_fileFormat, m_numTracks, m_tickDivision, m_midiTracks};
}

void Parser::processEvent(Event event) {
  if (StateEvents.find({m_state, event}) == StateEvents.end()) {
    std::string msg = std::format(
        "Unable to process event given current state.\nEvent: {}\nState: {}",
        toString(event), toString(m_state));
    throw std::runtime_error(msg);
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

State Parser::getState() const {
  return m_state;
}

Event Parser::getEventRegister() const {
  return m_eventRegister;
}

Event Parser::getMessageRegister() const {
  return m_messageRegister;
}

void Parser::onIdentifier() {
  auto identifier = m_scanner.scan<4>();
  bool isHeaderID =
      std::equal(HeaderID.begin(), HeaderID.end(), identifier.begin());
  bool isTrackID =
      std::equal(TrackID.begin(), TrackID.end(), identifier.begin());

  if (!isHeaderID && !isTrackID) {
    throw std::runtime_error("Identifier is neither MThd nor MTrk.");
  }
  setState(isHeaderID ? State::HEADER_ID_FOUND : State::TRACK_ID_FOUND);
  setNextEvent(Event::FIXED_LENGTH);
}

void Parser::onFixedLength() {
  auto length = ntohl(m_scanner.scan<uint32_t>());
  if (m_state == State::HEADER_ID_FOUND && length != 6) {
    throw std::runtime_error("The length of the header chunk must be 6.");
  }
  if (m_state == State::TRACK_ID_FOUND) {
    m_midiTracks.at(m_trackCount).length = length;
  }
  auto nextEvent = m_state == State::TRACK_ID_FOUND ? Event::VARIABLE_TIME
                                                    : Event::FILE_FORMAT;
  setNextEvent(nextEvent);
  setState(State::FIXED_LENGTH_FOUND);
}

void Parser::onFileFormat() {
  m_fileFormat = ntohs(m_scanner.scan<uint16_t>());
  setState(State::FILE_FORMAT_FOUND);
  setNextEvent(Event::NUM_TRACKS);
}

void Parser::onNumTracks() {
  m_numTracks = ntohs(m_scanner.scan<uint16_t>());
  m_midiTracks.resize(m_numTracks);
  setState(State::NUM_TRACKS_FOUND);
  setNextEvent(Event::TICKS);
}

void Parser::onTicks() {
  m_tickDivision = ntohs(m_scanner.scan<int16_t>());
  setState(State::HEADER_CHUNK_READ);
  setNextEvent(Event::IDENTIFIER);
}

void Parser::onVariableTime() {
  if (m_state != State::READING_VARIABLE_TIME && !m_bytesRegister.empty()) {
    throw std::runtime_error(
        "Trying to parse variable time but bytes register is not empty.");
  }
  if (m_bytesRegister.size() > 4) {
    throw std::runtime_error("End byte of variable time not found.");
  }
  auto byte = m_scanner.scan<uint8_t>();
  bool isMeta = byte == 0xFF;
  bool isMidi = byte >= 0x80 && byte <= 0xEF;
  bool isLastByte = 0x00 <= byte && byte <= 0x7F;  // Bytes where the MSB = 0
  bool shouldReadVariableTime =
      m_state == State::EVENT_READ || m_state == State::READING_VARIABLE_TIME;

  if (isLastByte) {
    m_bytesRegister.emplace_back(byte);
    m_variableLength = variableTo32(m_bytesRegister);
    m_deltaTimeRegister = m_variableLength;
    m_bytesRegister.clear();
    setState(State::VARIABLE_TIME_READ);
    if (TextEvents.find(m_eventRegister) != TextEvents.end()) {
      setNextEvent(m_eventRegister);
    } else {
      setNextEvent(Event::VARIABLE_TIME);
    }
    return;
  }

  if (isMeta && !shouldReadVariableTime) {
    setState(State::META_FOUND);
    return setNextEvent(Event::META_TYPE);
  }

  if (isMidi && !shouldReadVariableTime) {
    static const uint8_t messageMask = 0b11110000;
    static const uint8_t channelMask = 0b00001111;
    m_messageRegister = static_cast<Event>(byte & messageMask);
    m_channelRegister = byte & channelMask;
    setState(State::MIDI_FOUND);
    setNextEvent(m_messageRegister);
    if (MidiMessages.find(m_messageRegister) == MidiMessages.end()) {
      throw std::runtime_error("Unrecognized midi event.");
    }
    return;
  }

  if (shouldReadVariableTime) {
    m_bytesRegister.emplace_back(byte);
    setState(State::READING_VARIABLE_TIME);
    return setNextEvent(Event::VARIABLE_TIME);
  }

  // unsupported System messages
  static std::set<uint8_t> skip{0b11110100, 0b11110001, 0b11110101, 0b11110110,
                                0b11111000, 0b11111001, 0b11111010, 0b11111011,
                                0b11111100, 0b11111101, 0b11111110, 0b11111111};

  if (skip.contains(byte)) {  // skip
    return setNextEvent(Event::VARIABLE_TIME);
  }

  if (byte == 0b11110011) {  // skip 1 byte
    m_scanner.scan<1>();
    return setNextEvent(Event::VARIABLE_TIME);
  }
  if (byte == 0xF2) {  // skip 2 bytes
    m_scanner.scan<2>();
    return setNextEvent(Event::VARIABLE_TIME);
  }

  if (byte == 0b11110000) {  // skip system exclusive meta messages
    auto curr = m_scanner.scan<uint8_t>();
    while (curr != 0b11110111) {
      curr = m_scanner.scan<uint8_t>();
    }
    return setNextEvent(Event::VARIABLE_TIME);
  }

  if (byte == 0xF7) {  // skip sysex messages
    auto curr = m_scanner.scan<uint8_t>();
    while (curr != 0xF7) {
      curr = m_scanner.scan<uint8_t>();
    }
    return setNextEvent(Event::VARIABLE_TIME);
  }

  throw std::runtime_error(
      "Invalid parser state. Something is wrong reading variable time.");
}

void Parser::onMetaType() {
  auto typeByte = m_scanner.scan<uint8_t>();
  auto metaType = Event(typeByte);

  if (typeByte == 0x7F) {
    // 0x7F is the same as MIDI Note Off
    setState(State::META_SEQUENCER_SPECIFIC_FOUND);
    setNextEvent(Event::SEQUENCER_SPECIFIC);
    return;
  }

  if (MetaHandlers.find(metaType) == MetaHandlers.end()) {
    throw std::runtime_error("Unrecognized meta event!");
  } else {
    setState(MetaHandlers.at(metaType));
    setNextEvent(metaType);
  }
}

void Parser::onSequenceNumber() {
  auto length = m_scanner.scan<uint8_t>();
  if (length == 0) {
    setState(State::EVENT_READ);
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(
            MetaSequenceNumberEvent{{m_deltaTimeRegister}, true, 0});
    return;
  }
  if (length == 2) {
    auto sequenceNumber = m_scanner.scan<uint16_t>();
    setState(State::EVENT_READ);
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(MetaSequenceNumberEvent{
            {m_deltaTimeRegister}, false, sequenceNumber});
    return;
  }
  throw std::runtime_error(
      "Length of sequence number meta event must be either 0 or 2.");
}

void Parser::onText() {
  if (m_eventRegister != Event::TEXT) {
    m_eventRegister = Event::TEXT;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(
            MetaTextEvent{m_deltaTimeRegister,
                          std::string(reinterpret_cast<char*>(buffer.data()),
                                      buffer.size())});
  }
}

void Parser::onCopyrightNotice() {
  if (m_eventRegister != Event::COPYRIGHT_NOTICE) {
    m_eventRegister = Event::COPYRIGHT_NOTICE;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(MetaCopyrightNoticeEvent{
            m_deltaTimeRegister,
            std::string(reinterpret_cast<char*>(buffer.data()),
                        buffer.size())});
  }
};

void Parser::onTrackName() {
  if (m_eventRegister != Event::TRACK_NAME) {
    m_eventRegister = Event::TRACK_NAME;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(MetaTrackNameEvent{
            m_deltaTimeRegister,
            std::string(reinterpret_cast<char*>(buffer.data()),
                        buffer.size())});
  }
};

void Parser::onInstrumentName() {
  if (m_eventRegister != Event::INSTRUMENT_NAME) {
    m_eventRegister = Event::INSTRUMENT_NAME;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(MetaInstrumentNameEvent{
            m_deltaTimeRegister,
            std::string(reinterpret_cast<char*>(buffer.data()),
                        buffer.size())});
  }
};

void Parser::onLyric() {
  if (m_eventRegister != Event::LYRIC) {
    m_eventRegister = Event::LYRIC;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(
            MetaLyricEvent{m_deltaTimeRegister,
                           std::string(reinterpret_cast<char*>(buffer.data()),
                                       buffer.size())});
  }
};

void Parser::onMarker() {
  if (m_eventRegister != Event::MARKER) {
    m_eventRegister = Event::MARKER;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(
            MetaMarkerEvent{m_deltaTimeRegister,
                            std::string(reinterpret_cast<char*>(buffer.data()),
                                        buffer.size())});
  }
};

void Parser::onCue() {
  if (m_eventRegister != Event::CUE) {
    m_eventRegister = Event::CUE;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
    m_midiTracks.at(m_trackCount)
        .events.emplace_back(
            MetaCueEvent{m_deltaTimeRegister,
                         std::string(reinterpret_cast<char*>(buffer.data()),
                                     buffer.size())});
  }
};

void Parser::onChannelPrefix() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 1) {
    throw std::runtime_error("Length of channel prefix meta message must be 1");
  }
  auto channel = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MetaChannelEvent{m_deltaTimeRegister, channel});
};

void Parser::onMIDIPort() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 1) {
    throw std::runtime_error("Length of midi port meta message must be 1");
  }
  auto port = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  // TODO
  /* m_midiTracks.at(m_trackCount) */
  /*     .events.emplace_back(MetaChannelEvent{m_deltaTimeRegister, channel}); */
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
    setState(State::FINISHED);
    setNextEvent(Event::NO_OP);
  }
}

void Parser::onSetTempo() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 3) {
    throw std::runtime_error("Length of the set tempo meta event must be 3.");
  }
  auto buffer = m_scanner.scan<3>();
  auto tempo = variableTo32(buffer);
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MetaSetTempoEvent{m_deltaTimeRegister, tempo});
}

void Parser::onSMPTEOffset() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 5) {
    throw std::runtime_error("Length of SMPTE offset meta event must be 5");
  }
  auto data = m_scanner.scan<5>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MetaSMPTEOffsetEvent{
          m_deltaTimeRegister, data[0], data[1], data[2], data[3], data[4]});
};

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
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MetaSMPTEOffsetEvent{m_deltaTimeRegister, numerator,
                                                denominator, clocksPerClick,
                                                quarterDivision});
}

void Parser::onKeySignature() {
  auto length = m_scanner.scan<uint8_t>();
  if (length != 2) {
    throw std::runtime_error(
        "Length of the key signature meta event must be 2.");
  }
  auto sign = m_scanner.scan<int8_t>();
  auto mode = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MetaKeySignatureEvent{m_deltaTimeRegister, sign, mode});
};

void Parser::onSequencerSpecific() {
  if (m_eventRegister != Event::SEQUENCER_SPECIFIC) {
    m_eventRegister = Event::SEQUENCER_SPECIFIC;
    setNextEvent(Event::VARIABLE_TIME);
  } else {
    auto buffer = m_scanner.scan(m_variableLength);
    setState(State::EVENT_READ);
    m_eventRegister = Event::NO_OP;
    setNextEvent(Event::VARIABLE_TIME);
  }
}

void Parser::onMIDIControlChange() {
  uint8_t controllerNumber = m_scanner.scan<uint8_t>();
  uint8_t value = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MIDIControlChangeEvent{
          m_variableLength, m_channelRegister, controllerNumber, value});
}

void Parser::onMIDINoteOn() {
  uint8_t key = m_scanner.scan<uint8_t>();
  uint8_t velocity = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MIDINoteOnEvent{m_variableLength, m_channelRegister, key, velocity});
}

void Parser::onMIDIPolyAftertouch() {
  uint8_t key = m_scanner.scan<uint8_t>();
  uint8_t value = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(MIDIPolyAftertouchEvent{
          m_variableLength, m_channelRegister, key, value});
}

void Parser::onMIDIProgramChange() {
  uint8_t program = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MIDIProgramChangeEvent{m_variableLength, m_channelRegister, program});
}

void Parser::onMIDIAftertouch() {
  uint8_t value = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MIDIAftertouchEvent{m_variableLength, m_channelRegister, value});
}

void Parser::onMIDINoteOff() {
  uint8_t key = m_scanner.scan<uint8_t>();
  uint8_t velocity = m_scanner.scan<uint8_t>();
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MIDINoteOffEvent{m_variableLength, m_channelRegister, key, velocity});
}

void Parser::onMIDIPitchBend() {
  auto data = m_scanner.scan<2>();
  uint8_t value = (data[0] << 7) + data[1];
  setState(State::EVENT_READ);
  setNextEvent(Event::VARIABLE_TIME);
  m_midiTracks.at(m_trackCount)
      .events.emplace_back(
          MIDIPitchBendEvent{m_variableLength, m_channelRegister, value});
}

}  // namespace MidiParser
