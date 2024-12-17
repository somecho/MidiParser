#include <MidiParser/Parser.h>
#include <gtest/gtest.h>
#include <stdexcept>

TEST(Parser, OpeningValidFile) {
  auto parser = MidiParser::Parser(std::string(EXAMPLES_DIR) + "/queen.mid");
}

TEST(Parser, OpeningInvalidFile) {
  EXPECT_THROW(MidiParser::Parser("/path/to/nowhere"), std::runtime_error);
}

TEST(Parser, Parse) {
  auto parser = MidiParser::Parser(std::string(EXAMPLES_DIR) + "/queen.mid");
  parser.parse();
}
