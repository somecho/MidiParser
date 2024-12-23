#include <gtest/gtest.h>
#include <string>

#include "MidiParser/Parser.hpp"

TEST(Parser_ParsesFile, Queen) {
  auto p = MidiParser::Parser();
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/queen.mid");
  ASSERT_EQ(f.numTracks, f.tracks.size());
}

TEST(Parser_ParsesFile, Mozart) {
  auto p = MidiParser::Parser();
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/mozart.mid");
  ASSERT_EQ(f.numTracks, f.tracks.size());
}

TEST(Parser_ParsesFile, Mahler) {
  auto p = MidiParser::Parser();
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/mahler.mid");
  ASSERT_EQ(f.numTracks, f.tracks.size());
}

TEST(Parser_ParsesFile, Debussy) {
  auto p = MidiParser::Parser();
  auto f = p.parse(std::string(EXAMPLES_DIR) + "/debussy.mid");
  ASSERT_EQ(f.numTracks, f.tracks.size());
}
