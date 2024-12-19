#include <MidiParser/Parser.h>
#include <gtest/gtest.h>
#include <stdexcept>

TEST(ParserNegativeTests, OpeningInvalidFile) {
  EXPECT_THROW(MidiParser::Parser("/path/to/nowhere"), std::runtime_error);
}

TEST(ParsingCompleteMidiFile, Multitrack) {
  auto parser = MidiParser::Parser(std::string(EXAMPLES_DIR) + "/queen.mid");
  parser.parse();
  EXPECT_EQ(parser.getState(), MidiParser::State::FINISHED);
}

TEST(ProcessingEvents, Header) {
  auto parser =
      MidiParser::Parser(std::string(TEST_DATA_DIR) + "/midimsgs.mid");

  parser.processEvent(MidiParser::Event::IDENTIFIER);
  EXPECT_EQ(parser.getState(), MidiParser::State::HEADER_ID_FOUND);

  parser.processEvent(MidiParser::Event::FIXED_LENGTH);
  EXPECT_EQ(parser.getState(), MidiParser::State::FIXED_LENGTH_FOUND);

  parser.processEvent(MidiParser::Event::FILE_FORMAT);
  EXPECT_EQ(parser.getState(), MidiParser::State::FILE_FORMAT_FOUND);

  parser.processEvent(MidiParser::Event::NUM_TRACKS);
  EXPECT_EQ(parser.getState(), MidiParser::State::NUM_TRACKS_FOUND);

  parser.processEvent(MidiParser::Event::TICKS);
  EXPECT_EQ(parser.getState(), MidiParser::State::HEADER_CHUNK_READ);
}

TEST(ProcessingEvents, MidiMessages) {
  auto parser =
      MidiParser::Parser(std::string(TEST_DATA_DIR) + "/midimsgs.mid");

  // Send events to parse header
  parser.processEvent(MidiParser::Event::IDENTIFIER);
  parser.processEvent(MidiParser::Event::FIXED_LENGTH);
  parser.processEvent(MidiParser::Event::FILE_FORMAT);
  parser.processEvent(MidiParser::Event::NUM_TRACKS);
  parser.processEvent(MidiParser::Event::TICKS);

  // Send events to start parsing track
  parser.processEvent(MidiParser::Event::IDENTIFIER);
  EXPECT_EQ(parser.getState(), MidiParser::State::TRACK_ID_FOUND);
  parser.processEvent(MidiParser::Event::FIXED_LENGTH);
  EXPECT_EQ(parser.getState(), MidiParser::State::FIXED_LENGTH_FOUND);

  // Parse every midi message
  // 7 voice messages
  // 16 channels
  for (int i = 0; i < (7 * 16); i++) {
    parser.processEvent(MidiParser::Event::VARIABLE_TIME);
    EXPECT_EQ(parser.getState(), MidiParser::State::VARIABLE_TIME_READ);
    parser.processEvent(MidiParser::Event::VARIABLE_TIME);
    EXPECT_EQ(parser.getState(), MidiParser::State::MIDI_FOUND);
    parser.processEvent(parser.getMessageRegister());
    EXPECT_EQ(parser.getState(), MidiParser::State::EVENT_READ);
  }
}
