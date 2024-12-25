#include <gtest/gtest.h>
#include <format>

#include "read.hpp"

using Input = std::stack<uint8_t>;
using Expected = uint32_t;
using TestData = std::pair<Input, Expected>;

class vlqto32 : public testing::TestWithParam<TestData> {
 protected:
  vlqto32() {};
};

TEST_P(vlqto32, ConcatenatesBytesCorrectly) {
  auto p = GetParam();
  auto i = p.first;
  auto e = p.second;
  EXPECT_EQ(MidiParser::vlqto32(i), e);
}

const inline TestData data[] = {
    {std::pair{Input({0x0}), 0x0}},
    {std::pair{Input({0x40}), 0x40}},
    {std::pair{Input({0x7F}), 0x7F}},
    {std::pair{Input({0x81, 0x0}), 0x80}},
    {std::pair{Input({0xC0, 0x0}), 0x2000}},
    {std::pair{Input({0x81, 0x80, 0x0}), 0x4000}},
    {std::pair{Input({0xC0, 0x80, 0x0}), 0x100000}},
    {std::pair{Input({0xFF, 0xFF, 0x7F}), 0x1FFFFF}},
    {std::pair{Input({0x81, 0x80, 0x80, 0x0}), 0x200000}},
    {std::pair{Input({0xC0, 0x80, 0x80, 0x0}), 0x8000000}},
    {std::pair{Input({0xFF, 0xFF, 0xFF, 0x7F}), 0x0FFFFFFF}},
};

auto name = [](const testing::TestParamInfo<TestData>& info) {
  std::string n = "in0x";
  auto input = info.param.first;
  auto expected = info.param.second;
  while (!input.empty()) {
    n += std::format("{:02X}", input.top());
    input.pop();
  }
  n += std::format("_0x{:08X}", expected);
  return n;
};

INSTANTIATE_TEST_SUITE_P(Parser_Method, vlqto32, testing::ValuesIn(data), name);
