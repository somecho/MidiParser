#include <functional>

#include "Parser.h"
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
    {State::META_SEQUENCE_NUMBER_FOUND, Event::SEQUENCE_NUMBER},
    {State::META_TEXT_FOUND, Event::TEXT},
    {State::META_TEXT_FOUND, Event::VARIABLE_TIME},
    {State::META_COPYRIGHT_NOTICE_FOUND, Event::COPYRIGHT_NOTICE},
    {State::META_COPYRIGHT_NOTICE_FOUND, Event::VARIABLE_TIME},
    {State::META_TRACK_NAME_FOUND, Event::TRACK_NAME},
    {State::META_TRACK_NAME_FOUND, Event::VARIABLE_TIME},
    {State::META_INSTRUMENT_NAME_FOUND, Event::INSTRUMENT_NAME},
    {State::META_INSTRUMENT_NAME_FOUND, Event::VARIABLE_TIME},
    {State::META_LYRIC_FOUND, Event::LYRIC},
    {State::META_LYRIC_FOUND, Event::VARIABLE_TIME},
    {State::META_MARKER_FOUND, Event::MARKER},
    {State::META_MARKER_FOUND, Event::VARIABLE_TIME},
    {State::META_CUE_FOUND, Event::CUE},
    {State::META_CUE_FOUND, Event::VARIABLE_TIME},
    {State::META_CHANNEL_PREFIX_FOUND, Event::CHANNEl_PREFIX},
    {State::META_SET_TEMPO_FOUND, Event::SET_TEMPO},
    {State::META_SMPTE_OFFSET_FOUND, Event::SMPTE_OFFSET},
    {State::META_TIME_SIGNATURE_FOUND, Event::TIME_SIGNATURE},
    {State::META_KEY_SIGNATURE_FOUND, Event::KEY_SIGNATURE},
    {State::VARIABLE_TIME_READ, Event::VARIABLE_TIME},
    {State::VARIABLE_TIME_READ, Event::TEXT},
    {State::VARIABLE_TIME_READ, Event::MIDI},
    {State::READING_VARIABLE_TIME, Event::VARIABLE_TIME},
    {State::MIDI_FOUND, Event::MIDI_NOTE_OFF},
    {State::MIDI_FOUND, Event::MIDI_NOTE_ON},
    {State::MIDI_FOUND, Event::MIDI_POLY_AFTERTOUCH},
    {State::MIDI_FOUND, Event::MIDI_CONTROL_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_PROGRAM_CHANGE},
    {State::MIDI_FOUND, Event::MIDI_AFTERTOUCH},
    {State::MIDI_FOUND, Event::MIDI_PITCH_BEND},
    {State::MIDI_FOUND, Event::VARIABLE_TIME},
    {State::TRACK_READ, Event::IDENTIFIER},
    {State::EVENT_READ, Event::VARIABLE_TIME},
    {State::END_OF_TRACK_FOUND, Event::END_OF_TRACK},
};

const std::unordered_map<Event, State> MetaHandlers{
    {Event::SEQUENCE_NUMBER, State::META_SEQUENCE_NUMBER_FOUND},
    {Event::TEXT, State::META_TEXT_FOUND},
    {Event::COPYRIGHT_NOTICE, State::META_COPYRIGHT_NOTICE_FOUND},
    {Event::TRACK_NAME, State::META_TRACK_NAME_FOUND},
    {Event::INSTRUMENT_NAME, State::META_INSTRUMENT_NAME_FOUND},
    {Event::LYRIC, State::META_LYRIC_FOUND},
    {Event::MARKER, State::META_MARKER_FOUND},
    {Event::CUE, State::META_CUE_FOUND},
    {Event::CHANNEl_PREFIX, State::META_CHANNEL_PREFIX_FOUND},
    {Event::END_OF_TRACK, State::END_OF_TRACK_FOUND},
    {Event::SET_TEMPO, State::META_SET_TEMPO_FOUND},
    {Event::SMPTE_OFFSET, State::META_SMPTE_OFFSET_FOUND},
    {Event::TIME_SIGNATURE, State::META_TIME_SIGNATURE_FOUND},
    {Event::KEY_SIGNATURE, State::META_KEY_SIGNATURE_FOUND},
};

const std::set<Event> MidiMessages{
    Event::MIDI_NOTE_OFF,        Event::MIDI_NOTE_ON,
    Event::MIDI_POLY_AFTERTOUCH, Event::MIDI_CONTROL_CHANGE,
    Event::MIDI_PROGRAM_CHANGE,  Event::MIDI_AFTERTOUCH,
    Event::MIDI_PITCH_BEND,
};

std::unordered_map<Event, std::function<void()>> bindActions(Parser& parser) {
  return {
      {Event::SEQUENCE_NUMBER,
       [&parser]() {
         parser.onSequenceNumber();
       }},
      {Event::TEXT,
       [&parser]() {
         parser.onText();
       }},
      {Event::COPYRIGHT_NOTICE,
       [&parser]() {
         parser.onCopyrightNotice();
       }},
      {Event::TRACK_NAME,
       [&parser]() {
         parser.onTrackName();
       }},
      {Event::INSTRUMENT_NAME,
       [&parser]() {
         parser.onInstrumentName();
       }},
      {Event::LYRIC,
       [&parser]() {
         parser.onLyric();
       }},
      {Event::MARKER,
       [&parser]() {
         parser.onMarker();
       }},
      {Event::CUE,
       [&parser]() {
         parser.onCue();
       }},
      {Event::CHANNEl_PREFIX,
       [&parser]() {
         parser.onChannelPrefix();
       }},
      {Event::END_OF_TRACK,
       [&parser]() {
         parser.onEndOfTrack();
       }},
      {Event::SET_TEMPO,
       [&parser]() {
         parser.onSetTempo();
       }},
      {Event::SMPTE_OFFSET,
       [&parser]() {
         parser.onSMPTEOffset();
       }},
      {Event::TIME_SIGNATURE,
       [&parser]() {
         parser.onTimeSignature();
       }},
      {Event::KEY_SIGNATURE,
       [&parser]() {
         parser.onKeySignature();
       }},
      {Event::IDENTIFIER,
       [&parser]() {
         parser.onIdentifier();
       }},
      {Event::FIXED_LENGTH,
       [&parser]() {
         parser.onFixedLength();
       }},
      {Event::FILE_FORMAT,
       [&parser]() {
         parser.onFileFormat();
       }},
      {Event::NUM_TRACKS,
       [&parser]() {
         parser.onNumTracks();
       }},
      {Event::TICKS,
       [&parser]() {
         parser.onTicks();
       }},
      {Event::VARIABLE_TIME,
       [&parser]() {
         parser.onVariableTime();
       }},
      {Event::META_TYPE,
       [&parser]() {
         parser.onMetaType();
       }},
      {Event::MIDI_NOTE_OFF,
       [&parser]() {
         parser.onMIDINoteOff();
       }},
      {Event::MIDI_NOTE_ON,
       [&parser]() {
         parser.onMIDINoteOn();
       }},
      {Event::MIDI_POLY_AFTERTOUCH,
       [&parser]() {
         parser.onMIDIPolyAftertouch();
       }},
      {Event::MIDI_CONTROL_CHANGE,
       [&parser]() {
         parser.onMIDIControlChange();
       }},
      {Event::MIDI_PROGRAM_CHANGE,
       [&parser]() {
         parser.onMIDIProgramChange();
       }},
      {Event::MIDI_AFTERTOUCH,
       [&parser]() {
         parser.onMIDIAftertouch();
       }},
      {Event::MIDI_PITCH_BEND,
       [&parser]() {
         parser.onMIDIPitchBend();
       }},
  };
}

const std::set<Event> TextEvents{Event::TEXT,       Event::COPYRIGHT_NOTICE,
                                 Event::TRACK_NAME, Event::INSTRUMENT_NAME,
                                 Event::LYRIC,      Event::MARKER,
                                 Event::CUE};

}  // namespace MidiParser
