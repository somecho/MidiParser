#include <netinet/in.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <span>
#include <stack>
#include <string>
#include <vector>

uint32_t variableTo32(std::span<uint8_t> buffer) {
  uint32_t u32 = 0;
  for (size_t i = 0; i < buffer.size(); ++i) {
    auto offset = (buffer.size() - 1 - i) * 8;
    u32 |= static_cast<uint32_t>(buffer[i]) << offset;
  }
  return u32;
}

void parse(std::string path) {
  std::ifstream f(path, std::ios::binary);

  f.seekg(0, std::ios::end);
  uint32_t length = f.tellg();
  std::cout << "File size: " << length << std::endl;
  f.seekg(0, std::ios::beg);
  assert(f.tellg() == 0);

  // Midi Header is 14 bytes
  // 4 Bytes MThd
  // 4 Bytes length
  // 2 bytes Format
  // 2 bytes num tracks
  // 2 bytes num division
  std::array<char, 14> headerArr;
  f.read(reinterpret_cast<char*>(headerArr.data()), headerArr.size());

  uint16_t numTracks = 0 | (headerArr[10] << 8) | headerArr[11];
  std::cout << "Num tracks: " << numTracks << std::endl;

  std::vector<std::vector<uint8_t>> tracks;
  for (auto i = 0u; i < numTracks; i++) {
    // Read MTrk
    std::array<uint8_t, 4> id;
    f.read(reinterpret_cast<char*>(id.data()), id.size());
    std::cout << std::string(reinterpret_cast<char*>(id.data()), 4)
              << std::endl;

    // Get track length
    uint32_t trackLength = 0;
    f.read(reinterpret_cast<char*>(&trackLength), sizeof(trackLength));
    trackLength = ntohl(trackLength);

    // Get track bytes
    std::vector<uint8_t> trackBytes(trackLength);
    f.read(reinterpret_cast<char*>(trackBytes.data()), trackBytes.size());
    tracks.emplace_back(trackBytes);
  }

  f.ignore(1);  // Read EOF
  assert(f.eof());

  // Parse first track
  for (size_t i = 0; i < tracks.size(); i++) {
    size_t curr = 0;
    bool endOfTrackFound = false;
    uint8_t runningStatus = 0;
    uint8_t maskedRunning = runningStatus & 0b11110000;
    while (!endOfTrackFound) {

      //FIND DELTA TIME

      bool deltaFound = false;
      while (!deltaFound) {
        uint8_t currByte = tracks[i][curr];
        uint8_t masked = currByte & 0b10000000;
        bool isMSBSet = masked != 0x0;
        if (!isMSBSet) {
          deltaFound = true;
        }
        curr++;
      }

      // CHECK WHAT EVENT
      uint8_t identifier = tracks[i][curr];
      curr++;
      switch (identifier) {
        case 0xFF: {
          uint8_t metaType = tracks[i][curr];
          if (metaType == 0x2F) {
            std::cout << "END OF TRACK" << std::endl;
            endOfTrackFound = true;
          }
          std::cout << std::format("META: {:02X}", metaType) << std::endl;
          curr++;
          uint32_t length = 0;
          bool lenFound = false;
          /* size_t j = 0; */
          std::stack<uint8_t> s;
          while (!lenFound) {
            uint8_t b = tracks[i][curr];
            s.push(b);
            /* length |= b << (j * 7); */
            if ((b & 0b10000000) == 0x0) {
              lenFound = true;
              curr++;
              break;
            }
            curr++;
          }
          const auto size = s.size();
          for (int i = 0; i < size; i++) {
            length |= (s.top() & 0b01111111) << (7 * i);
            s.pop();
          }
          std::cout << length << std::endl;
          curr += length;
          break;
        }  // case 0xFF
        case 0xF7: {
          std::cout << "SYSEX" << std::endl;
          uint8_t next = tracks[i][curr];
          while (next != 0xF7) {
            curr++;
            next = tracks[i][curr];
          }
          curr++;
          break;
        }
        case 0xF0: {
          std::cout << "SYSEX" << std::endl;
          uint8_t next = tracks[i][curr];
          while (next != 0xF7) {
            curr++;
            next = tracks[i][curr];
          }
          curr++;
          break;
        }
        default:  // find midi
          static std::set<uint8_t> noLength{0b11110001, 0b11110100, 0b11110101,
                                            0b11110110, 0b11111000, 0b11111001,
                                            0b11111010, 0b11111011, 0b11111100,
                                            0b11111101, 0b11111110};

          static std::set<uint8_t> single{
              0b11110011,
              0b11000000,
              0b11010000,
          };

          static std::set<uint8_t> two{
              0b10000000, 0b10010000, 0b10100000,
              0b11110010, 0b10110000, 0b11100000,
          };
          uint8_t b = identifier;
          auto masked = b & 0b11110000;
          if (noLength.contains(b)) {
            std::cout << "MIDI skip" << std::endl;
            /* curr++; */
            runningStatus = identifier;
            maskedRunning = runningStatus & 0b11110000;
            break;
          }
          if (single.contains(b) || single.contains(masked)) {
            std::cout << "MIDI skip 1" << std::endl;
            curr += 1;
            runningStatus = identifier;
            maskedRunning = runningStatus & 0b11110000;
            break;
          }

          if (two.contains(b) || two.contains(masked)) {
            std::cout << "MIDI skip 2" << std::endl;
            curr += 2;
            runningStatus = identifier;
            maskedRunning = runningStatus & 0b11110000;
            break;
          }
          // no status byte encountered
          if (noLength.contains(runningStatus)) {
            curr--;
            break;
          }
          if (single.contains(runningStatus) ||
              single.contains(maskedRunning)) {
            break;
          }
          if (two.contains(runningStatus) || two.contains(maskedRunning)) {
            std::cout << "MIDI" << std::endl;
            curr += 1;
            break;
          }
          throw std::runtime_error("SOMETHING WRONG");
          break;
      }
    }
    std::cout << "Finished parsing track: " << i << std::endl;
  }
}

int main(int argc, char* argv[]) {
  parse(argv[1]);
}
