#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

#include "MidiParser/MidiFile.hpp"
#include "MidiParser/Parser.hpp"

class Parser : public testing::TestWithParam<std::string> {
 public:
  Parser() {};

  static void SetUpTestSuite() {
    map = new std::unordered_map<std::string, MidiParser::MidiFile>();
  };

  static void TearDownTestSuite() {
    delete map;
    map = nullptr;
  };

  std::string data = std::string(EXAMPLES_DIR) + "/" + GetParam() + ".mid";
  static std::unordered_map<std::string, MidiParser::MidiFile>* map;
};

std::unordered_map<std::string, MidiParser::MidiFile>* Parser::map = nullptr;

TEST_P(Parser, ParsesFileWithoutThrowing) {
  auto p = MidiParser::Parser();
  auto m = p.parse(data);
  map->insert({GetParam(), m});
}

TEST_P(Parser, HeaderNumMatchesNumParsedTracks) {
  const auto& m = map->at(GetParam());
  EXPECT_EQ(m.numTracks, m.tracks.size());
}

TEST_P(Parser, ParsedTracksAreNotEmpty) {
  const auto& m = map->at(GetParam());
  for (const auto& t : m.tracks) {
    EXPECT_FALSE(t.events.empty());
  }
}

TEST_P(Parser, SizesReadMatchesRealSize) {
  const auto& m = map->at(GetParam());
  size_t s = 14;
  for (const auto& t : m.tracks) {
    s += t.length + 8;
  }
  std::ifstream file(data, std::ios::binary);
  file.seekg(0, std::ios::end);
  EXPECT_EQ(s, file.tellg());
}

INSTANTIATE_TEST_SUITE_P(
    Basic, Parser, testing::Values("queen", "mozart", "debussy", "mahler"),
    [](const testing::TestParamInfo<std::string>& info) { return info.param; });
