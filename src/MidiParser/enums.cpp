#include "enums.h"

namespace MidiParser {

std::string toString(MidiParser::State s) {
  switch (s) {
  case MidiParser::State::NEW:
    return "State::NEW";
  case MidiParser::State::HEADER_ID_FOUND:
    return "State::HEADER_ID_FOUND";
  case MidiParser::State::FIXED_LENGTH_FOUND:
    return "State::FIXED_LENGTH_FOUND";
  case MidiParser::State::FILE_FORMAT_FOUND:
    return "State::FILE_FORMAT_FOUND";
  case MidiParser::State::NUM_TRACKS_FOUND:
    return "State::NUM_TRACKS_FOUND";
  case MidiParser::State::HEADER_CHUNK_READ:
    return "State::HEADER_CHUNK_READ";
  case MidiParser::State::TRACK_ID_FOUND:
    return "State::TRACK_ID_FOUND";
  case MidiParser::State::READING_VARIABLE_TIME:
    return "State::READING_VARIABLE_TIME";
  case MidiParser::State::VARIABLE_TIME_READ:
    return "State::VARIABLE_TIME_READ";
  case MidiParser::State::META_FOUND:
    return "State::META_FOUND";
  case MidiParser::State::META_SET_TEMPO_FOUND:
    return "State::META_SET_TEMPO_FOUND";
  case MidiParser::State::META_TIME_SIGNATURE_FOUND:
    return "State::META_TIME_SIGNATURE_FOUND";
  case MidiParser::State::META_TEXT_EVENT_FOUND:
    return "State::META_TEXT_EVENT_FOUND";
  case MidiParser::State::END_OF_TRACK_FOUND:
    return "State::END_OF_TRACK_FOUND";
  case MidiParser::State::EVENT_READ:
    return "State::EVENT_READ";
  case MidiParser::State::TRACK_READ:
    return "State::TRACK_READ";
  case MidiParser::State::MIDI_FOUND:
    return "State::MIDI_FOUND";
  case MidiParser::State::FINISHED:
    return "State::FINISHED";
  default:
    return "Unrecognized state.";
  }
}

std::string toString(Event e) {
  switch (e) {
  case MidiParser::Event::TEXT:
    return "Event::TEXT";
  case MidiParser::Event::SET_TEMPO:
    return "Event::SET_TEMPO";
  case MidiParser::Event::TIME_SIGNATURE:
    return "Event::TIME_SIGNATURE";
  case MidiParser::Event::END_OF_TRACK:
    return "Event::END_OF_TRACK";
  case MidiParser::Event::IDENTIFIER:
    return "Event::IDENTIFIER";
  case MidiParser::Event::FIXED_LENGTH:
    return "Event::FIXED_LENGTH";
  case MidiParser::Event::FILE_FORMAT:
    return "Event::FILE_FORMAT";
  case MidiParser::Event::NUM_TRACKS:
    return "Event::NUM_TRACKS";
  case MidiParser::Event::TICKS:
    return "Event::TICKS";
  case MidiParser::Event::VARIABLE_TIME:
    return "Event::VARIABLE_TIME";
  case MidiParser::Event::META_TYPE:
    return "Event::META_TYPE";
  case MidiParser::Event::NO_OP:
    return "Event::NO_OP";
  case MidiParser::Event::MIDI:
    return "Event::MIDI";
  case MidiParser::Event::MIDI_NOTE_OFF:
    return "Event::MIDI_NOTE_OFF";
  case MidiParser::Event::MIDI_NOTE_ON:
    return "Event::MIDI_NOTE_ON";
  case MidiParser::Event::MIDI_POLY_AFTERTOUCH:
    return "Event::MIDI_POLY_AFTERTOUCH";
  case MidiParser::Event::MIDI_CONTROL_CHANGE:
    return "Event::MIDI_CONTROL_CHANGE";
  case MidiParser::Event::MIDI_PROGRAM_CHANGE:
    return "Event::MIDI_PROGRAM_CHANGE";
  case MidiParser::Event::MIDI_AFTERTOUCH:
    return "Event::MIDI_AFTERTOUCH";
  case MidiParser::Event::MIDI_PITCH_BEND:
    return "Event::MIDI_PITCH_BEND";
  default:
    return "Unrecognized event.";
  }
}

} // namespace MidiParser
