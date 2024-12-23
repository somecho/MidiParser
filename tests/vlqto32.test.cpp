#include <gtest/gtest.h>

#include "MidiParser/Parser.hpp"

TEST(Parser_vlqto32, ConcatenatesSingleBytesCorrectly) {
  {
    std::stack<uint8_t> s;
    s.push(0x0);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x0);
  }
  {
    std::stack<uint8_t> s;
    s.push(0x40);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x40);
  }
  {
    std::stack<uint8_t> s;
    s.push(0x7F);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x7F);
  }
}

TEST(Parser_vlqto32, Concatenates2BytesCorrectly) {
  {
    std::stack<uint8_t> s;
    s.push(0x81);
    s.push(0x0);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x80);
  }
  {
    std::stack<uint8_t> s;
    s.push(0xC0);
    s.push(0x0);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x2000);
  }
}

TEST(Parser_vlqto32, Concatenates3BytesCorrectly) {
  {
    std::stack<uint8_t> s;
    s.push(0x81);
    s.push(0x80);
    s.push(0x00);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x4000);
  }
  {
    std::stack<uint8_t> s;
    s.push(0xC0);
    s.push(0x80);
    s.push(0x0);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x100000);
  }
  {
    std::stack<uint8_t> s;
    s.push(0xFF);
    s.push(0xFF);
    s.push(0x7F);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x1FFFFF);
  }
}

TEST(Parser_vlqto32, Concatenates4BytesCorrectly) {
  {
    std::stack<uint8_t> s;
    s.push(0xC0);
    s.push(0x80);
    s.push(0x80);
    s.push(0x00);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x8000000);
  }
  {
    std::stack<uint8_t> s;
    s.push(0x81);
    s.push(0x80);
    s.push(0x80);
    s.push(0x00);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x200000);
  }
  {
    std::stack<uint8_t> s;
    s.push(0xFF);
    s.push(0xFF);
    s.push(0xFF);
    s.push(0x7F);
    auto res = MidiParser::Parser::vlqto32(s);
    EXPECT_EQ(res, 0x0FFFFFFF);
  }
}
