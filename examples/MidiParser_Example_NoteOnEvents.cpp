#include <format>
#include <iostream>
#include <variant>

#include "MidiParser/Parser.hpp"
#include "MidiParser/events.hpp"

int main() {
  auto p = MidiParser::Parser();
  MidiParser::MidiFile f = p.parse("data/midi_examples/queen.mid");

  // Iterate over every track in the midi file
  for (const MidiParser::MidiTrack& t : f.tracks) {

    // Iterate over every event in the midi track
    for (const MidiParser::TrackEvent& e : t.events) {

      // Check if the event is a MIDI Event
      if (std::holds_alternative<MidiParser::MIDIEvent>(e)) {

        // If it is, get its value
        const auto event = std::get<MidiParser::MIDIEvent>(e);

        // Check if the top 4 bits is matches those of a note on event and
        // print if it is
        if ((event.status & 0b11110000) == 0b10010000) {
          std::cout << std::format("Midi Note On - Key: {} | ",
                                   event.data.at(0));
        }
      }
    }
  }
  std::cout << std::endl;
  return 0;
}
