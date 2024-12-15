#include "Parser.h"
#include "doctest.h"
#include <format>

static const std::string QUEEN_MIDI =
    std::format("{}/queen.mid", MIDI_FILES_DIR);

TEST_CASE("Parsing a valid file") {
  SUBCASE("Multi Track Midi File") {
    auto midiFile = Midi::Parser::parse(QUEEN_MIDI);
    CHECK_EQ(midiFile.getType(), Midi::Format::MULTI_TRACK);
    CHECK_EQ(midiFile.getDivision(), uint16_t(192));
  }
}
