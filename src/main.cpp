#include "events.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

static constexpr std::array<uint8_t, 4> HEADER_BYTES{/*M*/ 0x4D, /*T*/ 0x54,
                                                     /*h*/ 0x68, /*d*/ 0x64};

static constexpr std::array<uint8_t, 4> TRACK_BYTES{/*M*/ 0x4D, /*T*/ 0x54,
                                                    /*r*/ 0x72, /*k*/ 0x6B};

void read(std::ifstream &f, std::span<uint8_t> buf) {
  uint16_t pos = f.tellg();
  if (!f.read(reinterpret_cast<char *>(buf.data()), buf.size())) {
    throw std::runtime_error(
        std::format("Failed to read from position {}", pos));
  };
}

template <typename T> T read(std::ifstream &f) {
  T buf;
  uint16_t pos = f.tellg();
  if (!f.read(reinterpret_cast<char *>(&buf), sizeof(T))) {
    throw std::runtime_error(
        std::format("Failed to read from position {}", pos));
  };
  return buf;
}

/// To be used once after parsing and validating the MIDI header chunk. Consumes
/// the next 4 bytes of the stream and validates it.
void validateHeaderEnd(std::ifstream &f) {
  std::array<uint8_t, 4> chunkHeader;
  read(f, std::span(chunkHeader));
  if (!std::equal(TRACK_BYTES.begin(), TRACK_BYTES.end(),
                  chunkHeader.begin())) {
    throw std::runtime_error("Malformed MIDI file. Could not find MIDI track.");
  }
}

uint32_t findDeltaTime(std::ifstream &f) {
  bool found = false;
  uint8_t bytesRead = 0;
  uint32_t deltaTime = 0;
  std::vector<uint8_t> buffer;
  while (!found) {
    uint8_t currByte = f.peek();
    if (currByte != 0xFF && currByte != 0xF0 && currByte != 0xF7 &&
        currByte < 0x80 && currByte > 0xEF) {
      buffer.push_back(currByte);
      f.ignore();
      bytesRead += 1;
    } else {
      f.ignore();
      for (size_t i = 0; i < buffer.size(); i++) {
        deltaTime = (deltaTime << 0) | buffer[i];
      }
      return deltaTime;
    }
    if (bytesRead > 4) {
      throw std::runtime_error(
          "Malformed MIDI file. Could not find delta time of event");
    }
  }
  std::unreachable();
}

struct Chunk {
  uint32_t chunkLength;
};

struct MidiHeader : public Chunk {
  uint16_t format;
  uint16_t numTracks;
  uint16_t division;
};

struct Event {
  /// The amount of time from the previous event. According to the MIDI
  /// specification, this field has variable length, but since the largest
  /// number allowed is 32 bits, it is represented as a `uint32_t`.
  uint32_t deltaTime;
};

// Midi Track
struct MidiTrack : public Chunk {
  std::vector<std::shared_ptr<Event>> events;
};

void getEvents(std::ifstream &f) {
  bool endOfTrack = false;
  std::vector<Midi::Event::Track> events;
  while (!endOfTrack) {
    std::cout << "Getting events" << std::endl;
    auto pos = f.tellg();
    std::array<uint8_t, 4> buf;
    read(f, std::span(buf));
    f.seekg(pos);
    if (std::equal(TRACK_BYTES.begin(), TRACK_BYTES.end(), buf.begin())) {
      endOfTrack = true;
      break;
    }
    uint32_t deltaTime = findDeltaTime(f);
    auto eventType = Midi::Event::Type(read<uint8_t>(f));
    if (eventType == Midi::Event::Type::META) {
      auto metatype = read<uint8_t>(f);
      switch (Midi::Event::Meta::Type(metatype)) {
      case Midi::Event::Meta::Type::SET_TEMPO: {
        uint32_t length = findDeltaTime(f); // FIXME: should be 3
        std::array<uint8_t, 3> microsecs;
        read(f, std::span(microsecs));
        uint32_t result = (static_cast<uint32_t>(microsecs[0]) << 16) |
                          (static_cast<uint32_t>(microsecs[1]) << 8) |
                          static_cast<uint32_t>(microsecs[2]);
        Midi::Event::SetTempo e{deltaTime, length, result};
        std::cout << e << std::endl;
        events.push_back(e);
        break;
      }
      case Midi::Event::Meta::Type::TEXT: {
        std::cout << "Found text" << std::endl;
        f.ignore(1);
        break;
      }
      default:
        f.ignore(1);
        break;
      }
    }
  }
}

int main() {
  std::ifstream f("queen.mid", std::ios::binary);
  if (!f) {
    throw std::runtime_error("Unable to open queen.mid");
  }

  // Parse Header

  std::array<uint8_t, 4> header;
  read(f, std::span(header));
  if (!std::equal(HEADER_BYTES.begin(), HEADER_BYTES.end(), header.begin())) {
    throw std::runtime_error("Invalid midi file. Midi header not found");
  }
  MidiHeader headerContent;
  headerContent.chunkLength = ntohl(read<uint32_t>(f));
  headerContent.format = ntohs(read<uint16_t>(f));
  headerContent.numTracks = ntohs(read<uint16_t>(f));
  headerContent.division = ntohs(read<uint16_t>(f));
  std::cout << headerContent.chunkLength << std::endl;
  std::cout << headerContent.format << std::endl;
  std::cout << headerContent.numTracks << std::endl;
  std::cout << headerContent.division << std::endl;
  validateHeaderEnd(f);

  std::vector<MidiTrack> midiTracks(headerContent.numTracks);
  size_t currentTrack = 0;
  while (currentTrack < headerContent.numTracks) {
    // Find Track Length
    uint32_t trackLength = ntohl(read<uint32_t>(f));
    std::cout << std::format("Number of bytes in track {}: {}", currentTrack,
                             trackLength)
              << std::endl;

    getEvents(f);
    // parse track events

    /* // Find delta time */
    /* uint32_t deltaTime = findDeltaTime(f); */
    /* // determine the type of event */
    /* EventType eventType = findEventType(f); */
    /* std::cout << "Event Type: " << eventType << std::endl; */
    /* getEvent(f, eventType); */
    /* // based on event type, get the data out */
    std::exit(0);
  }
}
