#include <gtest/gtest.h>
#include <format>
#include "gtest/gtest.h"

#include "read.hpp"

namespace ReadMetaEventTests {

using bytes = std::vector<uint8_t>;

struct TestData {
  std::string title;
  bytes b;
};

class MetaEvent : public testing::TestWithParam<TestData> {};

TEST_P(MetaEvent, ReturnsMetaEvent) {
  auto b = GetParam().b;
  auto it = b.begin();
  auto e = MidiParser::readMetaEvent(it, 0);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(b[1], e.status);
  EXPECT_EQ(0, e.deltaTime);
  EXPECT_EQ(b[2], e.data.size());
}

const inline TestData data[] = {
    TestData{"SequenceNumber", {0xFF, 0x0, 0x02, 0x0, 0x0}},
    TestData{"Text", {0xFF, 0x01, 0x05, 'p', 'i', 'a', 'n', 'o'}},
    TestData{"MidiPort", {0xFF, 0x21, 0x01, 0x05}},
    TestData{"EndOfTrack", {0xFF, 0x2F, 0x0}},
    TestData{"Tempo", {0xFF, 0x51, 0x03, 0xFF, 0xFF, 0xFF}},
    TestData{"SequencerSpecific",
             {0xFF, 0x7F, 0x07, 'a', 'b', 'c', 'd', 'e', 'f', 'g'}},
};

INSTANTIATE_TEST_SUITE_P(Reading, MetaEvent, testing::ValuesIn(data),
                         [](const testing::TestParamInfo<TestData>& info) {
                           return info.param.title;
                         });

}  // namespace ReadMetaEventTests

namespace ReadSysExEventTests {

using bytes = std::vector<uint8_t>;

TEST(ReadingSysExEvent, ReturnsSysExEvent) {
  bytes b = {0xF7, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0xF7};
  auto it = b.begin();
  auto e = MidiParser::readSysExEvent(it, 0);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.deltaTime, 0);
  EXPECT_EQ(e.data.size(), 10);
}

}  // namespace ReadSysExEventTests

namespace ReadMidiEventTests {

using bytes = std::vector<uint8_t>;

class StatusOnly : public testing::TestWithParam<uint8_t> {};
class SingleByte : public testing::TestWithParam<uint8_t> {};
class DoubleByte : public testing::TestWithParam<uint8_t> {};
class NonValidStatus : public testing::TestWithParam<uint8_t> {};

TEST_P(StatusOnly, ReturnsMidiEvent) {
  bytes b = {GetParam()};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0);
  ASSERT_TRUE(e);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.value().deltaTime, 0);
  EXPECT_EQ(e.value().data.size(), 0);
}

TEST_P(StatusOnly, WithRunningStatusReturnsNoMidiEvent) {
  bytes b;
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, GetParam(), 0);
  ASSERT_FALSE(e);
}

TEST_P(SingleByte, ReturnsMidiEvent) {
  bytes b = {GetParam(), 0};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0);
  ASSERT_TRUE(e);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.value().deltaTime, 0);
  EXPECT_EQ(e.value().data.size(), 1);
}

TEST_P(SingleByte, WithRunningStatusReturnsMidiEvent) {
  bytes b = {0};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0, GetParam());
  ASSERT_TRUE(e);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.value().deltaTime, 0);
  EXPECT_EQ(e.value().data.size(), 1);
}

TEST_P(DoubleByte, ReturnsMidiEvent) {
  bytes b = {GetParam(), 0, 0};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0);
  ASSERT_TRUE(e);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.value().deltaTime, 0);
  EXPECT_EQ(e.value().data.size(), 2);
}

TEST_P(DoubleByte, WithRunningStatusReturnsMidiEvent) {
  bytes b = {0, 0};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0, GetParam());
  ASSERT_TRUE(e);
  EXPECT_EQ(it, b.end());
  EXPECT_EQ(e.value().deltaTime, 0);
  EXPECT_EQ(e.value().data.size(), 2);
}

TEST_P(NonValidStatus, ReturnsNullOpt) {
  char status = ~GetParam();
  bytes b = {uint8_t(status)};
  auto it = b.begin();
  auto e = MidiParser::readMidiEvent(it, 0);
  ASSERT_FALSE(e);
}

auto name = [](const testing::TestParamInfo<uint8_t>& info) {
  return std::format("{:08B}", info.param);
};

INSTANTIATE_TEST_SUITE_P(Reading, StatusOnly,
                         testing::Values(0b11110001, 0b11110100, 0b11110101,
                                         0b11110110, 0b11111000, 0b11111001,
                                         0b11111010, 0b11111011, 0b11111100,
                                         0b11111101, 0b11111110),
                         name);

INSTANTIATE_TEST_SUITE_P(Reading, SingleByte,
                         testing::Values(0b11110011, 0b11000000, 0b11010000),
                         name);

INSTANTIATE_TEST_SUITE_P(Reading, DoubleByte,
                         testing::Values(0b10000000, 0b10010000, 0b10100000,
                                         0b10010001, 0b10010011, 0b11110010,
                                         0b10010101, 0b10011101, 0b10011111,
                                         0b10110000, 0b11100000),
                         name);

INSTANTIATE_TEST_SUITE_P(
    Reading, NonValidStatus,
    // These values are taken from the above 3 tests
    testing::Values(0b11110011, 0b11000000, 0b11010000, 0b10000000, 0b10010000,
                    0b10100000, 0b10010001, 0b10010011, 0b11110010, 0b10010101,
                    0b10011101, 0b10011111, 0b10110000, 0b11100000, 0b11110001,
                    0b11110100, 0b11110101, 0b11110110, 0b11111000, 0b11111001,
                    0b11111010, 0b11111011, 0b11111100, 0b11111101, 0b11111110),

    name);

}  // namespace ReadMidiEventTests
