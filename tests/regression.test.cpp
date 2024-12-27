#include <gtest/gtest.h>
#include <ranges>
#include <tuple>

#include "Parser.hpp"

TEST(RegressionTest, DeltaTimeIsNotTruncated) {
  MidiParser::Parser p;
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/twinkle.mid");
  auto t = f.tracks.at(0);
  std::vector<uint32_t> deltaTimes;
  std::vector<uint32_t> expectedDeltaTimes{
      479, 479, 479, 479, 479, 479, 959, 479, 479, 479, 479, 479, 479, 959,
  };

  for (auto e : t.events) {
    if (std::holds_alternative<MidiParser::MIDIEvent>(e)) {
      const auto event = std::get<MidiParser::MIDIEvent>(e);
      if (((event.status & 0b11110000) == 0b10010000) &&
          event.data.at(1) == 0) {
        deltaTimes.emplace_back(event.deltaTime);
      }
    }
  }

  for (auto pair : std::views::zip(deltaTimes, expectedDeltaTimes)) {
    EXPECT_EQ(pair.first, pair.second);
  }
}
