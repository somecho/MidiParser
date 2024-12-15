#include "Scanner.h"
#include <cstdint>
#include <doctest.h>
#include <ios>
#include <stdexcept>

static const std::string QUEEN_MIDI =
    std::format("{}/queen.mid", MIDI_FILES_DIR);

TEST_SUITE("Scanner") {
  TEST_CASE("Throws when file does not exist") {
    CHECK_THROWS_AS(Midi::Scanner("no/file/here"), std::runtime_error);
  }

  TEST_CASE("Stream position is at 0 on successful file load") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    CHECK_EQ(scanner.getPos(), 0);
  }

  TEST_CASE("Scanning") {
    SUBCASE("Primitive Types") {
      auto scanner = Midi::Scanner(QUEEN_MIDI);
      scanner.scan<uint8_t>();
      CHECK_EQ(scanner.getPos(), 1);
      scanner.scan<uint16_t>();
      CHECK_EQ(scanner.getPos(), 3);
      scanner.scan<uint32_t>();
      CHECK_EQ(scanner.getPos(), 7);
      scanner.scan<char>();
      CHECK_EQ(scanner.getPos(), 8);
    }

    SUBCASE("Arrays") {
      auto scanner = Midi::Scanner(QUEEN_MIDI);
      auto data = scanner.scan<8>();
      CHECK_EQ(scanner.getPos(), 8);
      CHECK_EQ(data.size(), 8);
      scanner.scan<100>();
      CHECK_EQ(scanner.getPos(), 108);
    }
  }

  TEST_CASE("Getting file length") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    auto length = scanner.getFileLength();
    SUBCASE("Filesize is correct") { CHECK_EQ(length, 51221); }
    SUBCASE("Position remains untouched") { CHECK_EQ(scanner.getPos(), 0); }
    SUBCASE("Filesize and position is correct after scanning") {
      scanner.scan<10>();
      CHECK_EQ(length, 51221);
      CHECK_EQ(scanner.getPos(), 10);
    }
  }

  TEST_CASE("Scanning at the end throws") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    scanner.seek(scanner.getFileLength() - 1);
    scanner.scan<char>(); // reading byte at last position
    CHECK_THROWS_AS(scanner.scan<char>(), std::runtime_error);
  }

  TEST_CASE("Going beyond the end throws") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    CHECK_THROWS_AS(scanner.seek(scanner.getFileLength() + 1),
                    std::runtime_error);
  }

  TEST_CASE("Seeking at valid position") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    scanner.seek(scanner.getFileLength() - 2);
    auto pos = scanner.getPos();
    pos += 1;
    scanner.seek(pos);
    scanner.scan<char>();
  }

  TEST_CASE("Seeking to a negative position") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    CHECK_THROWS_AS(scanner.seek(-1), std::runtime_error);
  }

  TEST_CASE("Seeking back and forth") {
    auto scanner = Midi::Scanner(QUEEN_MIDI);
    scanner.seek(10);
    CHECK_EQ(scanner.getPos(), 10);
    scanner.seek(0);
    CHECK_EQ(scanner.getPos(), 0);
    scanner.seek(100);
    CHECK_EQ(scanner.getPos(), 100);
    scanner.seek(50);
    CHECK_EQ(scanner.getPos(), 50);
    scanner.seek(5000);
    CHECK_EQ(scanner.getPos(), 5000);
    scanner.seek(1000);
    CHECK_EQ(scanner.getPos(), 1000);
  }
}
