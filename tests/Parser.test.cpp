#include <MidiParser/Parser.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

#include "MidiParser/enums.h"
#include "MidiParser/tables.h"

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

TEST(ProcessingEvents, MetaMessages) {
  auto parser =
      MidiParser::Parser(std::string(TEST_DATA_DIR) + "/metamsgs.mid");

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

  std::vector<std::pair<MidiParser::State, MidiParser::Event>> events{
      {MidiParser::State::META_SEQUENCE_NUMBER_FOUND,
       MidiParser::Event::SEQUENCE_NUMBER},
      {MidiParser::State::META_SEQUENCE_NUMBER_FOUND,
       MidiParser::Event::SEQUENCE_NUMBER},
      {MidiParser::State::META_TEXT_FOUND, MidiParser::Event::TEXT},
      /* {MidiParser::State::META_COPYRIGHT_NOTICE_FOUND, */
      /*  MidiParser::Event::COPYRIGHT_NOTICE}, */
      /* {MidiParser::State::META_TRACK_NAME_FOUND, MidiParser::Event::TRACK_NAME}, */
      /* {MidiParser::State::META_INSTRUMENT_NAME_FOUND, */
      /*  MidiParser::Event::INSTRUMENT_NAME}, */
      /* {MidiParser::State::META_LYRIC_FOUND, MidiParser::Event::LYRIC}, */
      /* {MidiParser::State::META_MARKER_FOUND, MidiParser::Event::MARKER}, */
      /* {MidiParser::State::META_CUE_FOUND, MidiParser::Event::CUE}, */
      /* {MidiParser::State::META_CHANNEL_PREFIX_FOUND, */
      /*  MidiParser::Event::CHANNEl_PREFIX}, */
      /* {MidiParser::State::META_SET_TEMPO_FOUND, MidiParser::Event::SET_TEMPO}, */
      /* {MidiParser::State::META_SMPTE_OFFSET_FOUND, */
      /*  MidiParser::Event::SMPTE_OFFSET}, */
      /* {MidiParser::State::META_TIME_SIGNATURE_FOUND, */
      /*  MidiParser::Event::TIME_SIGNATURE}, */
      /* {MidiParser::State::META_KEY_SIGNATURE_FOUND, */
      /*  MidiParser::Event::KEY_SIGNATURE}, */
      /* {MidiParser::State::END_OF_TRACK_FOUND, MidiParser::Event::END_OF_TRACK}, */
  };

  for (const auto& p : events) {
    parser.processEvent(MidiParser::Event::VARIABLE_TIME);
    EXPECT_EQ(parser.getState(), MidiParser::State::VARIABLE_TIME_READ);
    parser.processEvent(MidiParser::Event::VARIABLE_TIME);
    EXPECT_EQ(parser.getState(), MidiParser::State::META_FOUND);
    parser.processEvent(MidiParser::Event::META_TYPE);
    EXPECT_EQ(parser.getState(), p.first);
    parser.processEvent(p.second);

    if (MidiParser::TextEvents.contains(p.second)) {
      parser.processEvent(MidiParser::Event::VARIABLE_TIME);
      EXPECT_EQ(parser.getState(), MidiParser::State::VARIABLE_TIME_READ);
      parser.processEvent(p.second);
    }

    if (p.second == MidiParser::Event::END_OF_TRACK) {
      EXPECT_EQ(parser.getState(), MidiParser::State::TRACK_READ);

    } else {
      EXPECT_EQ(parser.getState(), MidiParser::State::EVENT_READ);
    }
  }
}
