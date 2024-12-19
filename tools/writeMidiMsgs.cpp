#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "MidiParser/tables.h"

int main() {
  std::vector<uint8_t> bytes;

  // MIDI HEADER
  bytes.insert(bytes.end(), {'M', 'T', 'h', 'd'});
  bytes.insert(bytes.end(), {0x0, 0x0, 0x0, 0x06});
  bytes.insert(bytes.end(), {0x0, 0x0});
  bytes.insert(bytes.end(), {0x0, 0x01});
  bytes.insert(bytes.end(), {0x0, 0xC0});

  // MIDI TRACK
  bytes.insert(bytes.end(), {'M', 'T', 'r', 'k'});
  bytes.insert(bytes.end(), {0x0, 0x0, 0x01, 0x2C}); // length does not matter

  uint i = 0;
  for (const auto &msg : MidiParser::MidiMessages) {
    for (uint8_t ch = 0; ch < 16; ch++) {
      auto statusByte = static_cast<uint8_t>(msg) | ch;
      uint8_t byte1 = 127;
      uint8_t byte2 = 127;
      bytes.emplace_back(0x0);
      bytes.emplace_back(statusByte);
      bytes.emplace_back(byte1);
      if (msg != MidiParser::Event::MIDI_PROGRAM_CHANGE &&
          msg != MidiParser::Event::MIDI_AFTERTOUCH) {
        bytes.emplace_back(byte2);
      }
      i++;
    }
  }

  std::ofstream file("data/test_examples/midimsgs.mid",
                     std::ios::out | std::ios::binary | std::ios::trunc);
  file.write(reinterpret_cast<char *>(bytes.data()), bytes.size());
  file.close();

  /* std::cout << std::format("{:08X}", uint32_t(300)) << std::endl; */
}
