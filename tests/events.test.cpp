#include <gtest/gtest.h>
#include <string>
#include <variant>

#include "Parser.hpp"

TEST(Events, CMajContains8NoteOnEvents) {
  MidiParser::Parser p;
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/cmaj.mid");

  std::vector<MidiParser::MIDIEvent> noteOnEvents;

  for (const auto& t : f.tracks) {
    for (const auto& e : t.events) {
      if (std::holds_alternative<MidiParser::MIDIEvent>(e)) {
        auto event = std::get<MidiParser::MIDIEvent>(e);

        bool hasNoteOnStatus = (event.status & 0b11110000) == 0b10010000;

        // Sometimes files use Note On events with 0 velocity
        // as a note off event
        bool hasNonZeroVelocity = event.data[1] != 0;

        if (hasNoteOnStatus && hasNonZeroVelocity) {
          noteOnEvents.emplace_back(event);
        }
      }
    }
  }

  ASSERT_EQ(noteOnEvents.size(), 8);
}
