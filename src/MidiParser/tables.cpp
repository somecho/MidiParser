#include "tables.h"

namespace MidiParser {

const std::set<std::pair<State, Event>> StateEvents{
    {State::NEW, Event::IDENTIFIER},
    {State::HEADER_ID_FOUND, Event::FIXED_LENGTH},
    {State::FILE_FORMAT_FOUND, Event::NUM_TRACKS},
    {State::NUM_TRACKS_FOUND, Event::TICKS},
    {State::HEADER_CHUNK_READ, Event::IDENTIFIER},
    {State::TRACK_ID_FOUND, Event::FIXED_LENGTH},
    {State::FIXED_LENGTH_FOUND, Event::FILE_FORMAT},
    {State::FIXED_LENGTH_FOUND, Event::VARIABLE_TIME},
    {State::META_FOUND, Event::META_TYPE},
    {State::META_SET_TEMPO_FOUND, Event::SET_TEMPO},
    {State::META_TIME_SIGNATURE_FOUND, Event::TIME_SIGNATURE},
    {State::META_TEXT_EVENT_FOUND, Event::TEXT},
    {State::META_TEXT_EVENT_FOUND, Event::VARIABLE_TIME},
    {State::VARIABLE_TIME_READ, Event::VARIABLE_TIME},
    {State::VARIABLE_TIME_READ, Event::TEXT},
    {State::VARIABLE_TIME_READ, Event::MIDI},
    {State::READING_VARIABLE_TIME, Event::VARIABLE_TIME},
    {State::MIDI_FOUND, Event::MIDI_NOTE_ON},
    {State::MIDI_FOUND, Event::MIDI_NOTE_OFF},
    {State::MIDI_FOUND, Event::MIDI_PROGRAM_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_CONTROL_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_PITCH_BEND},
    {State::MIDI_FOUND, Event::VARIABLE_TIME},
    {State::TRACK_READ, Event::IDENTIFIER},
    {State::EVENT_READ, Event::VARIABLE_TIME},
    {State::END_OF_TRACK_FOUND, Event::END_OF_TRACK},
};

const std::unordered_map<Event, State> MetaHandlers{
    {Event::SET_TEMPO, State::META_SET_TEMPO_FOUND},
    {Event::TIME_SIGNATURE, State::META_TIME_SIGNATURE_FOUND},
    {Event::END_OF_TRACK, State::END_OF_TRACK_FOUND},
    {Event::TEXT, State::META_TEXT_EVENT_FOUND},
};

const std::set<Event> MidiMessages{
    Event::MIDI_CONTROL_CHANGE, Event::MIDI_NOTE_ON, Event::MIDI_PROGRAM_CHANGE,
    Event::MIDI_NOTE_OFF, Event::MIDI_PITCH_BEND};

std::unordered_map<Event, std::function<void()>> bindActions(Parser& parser) {
  return {
      {Event::IDENTIFIER, [&parser]() { parser.onIdentifier(); }},
      {Event::FIXED_LENGTH, [&parser]() { parser.onFixedLength(); }},
      {Event::FILE_FORMAT, [&parser]() { parser.onFileFormat(); }},
      {Event::NUM_TRACKS, [&parser]() { parser.onNumTracks(); }},
      {Event::TICKS, [&parser]() { parser.onTicks(); }},
      {Event::VARIABLE_TIME, [&parser]() { parser.onVariableTime(); }},
      {Event::META_TYPE, [&parser]() { parser.onMetaType(); }},
      {Event::SET_TEMPO, [&parser]() { parser.onSetTempo(); }},
      {Event::TIME_SIGNATURE, [&parser]() { parser.onTimeSignature(); }},
      {Event::END_OF_TRACK, [&parser]() { parser.onEndOfTrack(); }},
      {Event::TEXT, [&parser]() { parser.onText(); }},
      {Event::MIDI_NOTE_ON, [&parser]() { parser.onMIDINoteOn(); }},
      {Event::MIDI_NOTE_OFF, [&parser]() { parser.onMIDINoteOff(); }},
      {Event::MIDI_PROGRAM_CHANGE,
       [&parser]() { parser.onMIDIProgramChange(); }},
      {Event::MIDI_CONTROL_CHANGE,
       [&parser]() { parser.onMIDIControlChange(); }},
      {Event::MIDI_PITCH_BEND, [&parser]() { parser.onMIDIPitchBend(); }},
  };
}

}  // namespace MidiParser
