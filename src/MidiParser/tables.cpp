#include "tables.h"

namespace MidiParser {

const std::set<std::pair<State, Event>> StateEvents{
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
    {State::META_TEXT_EVENT_FOUND, Event::TEXT},
    {State::META_TEXT_EVENT_FOUND, Event::VARIABLE_TIME},
    {State::VARIABLE_TIME_READ, Event::TEXT},
    {State::VARIABLE_TIME_READ, Event::MIDI},
    {State::MIDI_FOUND, Event::MIDI_CONTROL_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_NOTE_ON},
    {State::MIDI_FOUND, Event::MIDI_PROGRAM_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_NOTE_OFF},
    {State::MIDI_FOUND, Event::VARIABLE_TIME},
    {State::MIDI_FOUND, Event::MIDI_PITCH_BEND},
};

}
