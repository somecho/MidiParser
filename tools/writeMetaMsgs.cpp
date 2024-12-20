#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

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
  bytes.insert(bytes.end(), {0x0, 0x0, 0x01, 0x2C});  // length does not matter

  // SEQUENCE NUMBER
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x0, 0x0});

  // SEQUENCE NUMBER EXPLICIT
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x0, 0x02, 0x0, 0x01});

  // TEXT
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x01, 0x0A /*10*/, 'T', 'e', 'x', 't',
                             ' ', 'e', 'v', 'e', 'n', 't'});

  // COPYRIGHT
  bytes.insert(bytes.end(),
               {0x0, 0xFF, 0x02, 0x08, '(', 'C', ')', ' ', '2', '0', '2', '4'});

  // TRACK
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x03, 0x05, 'T', 'r', 'a', 'c', 'k'});

  // Instrument
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x04, 0x05, 'P', 'i', 'a', 'n', 'o'});

  // Lyric
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x05, 0x05, 'H', 'e', 'l', 'l', 'o'});

  // Marker
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x06, 0x05, 'B', 'e', 'g', 'i', 'n'});

  // Cue
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x07, 0x0A, 'E', 'x', 'p', 'l', 'o',
                             's', 'i', 'o', 'n', 's'});

  // Channel Prefix
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x20, 0x01, 0x01});

  // Set Tempo
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x51, 0x03, 0x07, 0xA1, 0x20});

  // SMPTE Offset
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x54, 0x05, 0x0, 0x0, 0x0, 0x0, 0x0});

  // Time Signature
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x58, 0x04, 0x06, 0x03, 0x24, 0x08});

  // Key Signature
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x59, 0x02, 0x00, 0x00});

  // End Of Track
  bytes.insert(bytes.end(), {0x0, 0xFF, 0x20, 0x01, 0x01});

  std::ofstream file("data/test_examples/metamsgs.mid",
                     std::ios::out | std::ios::binary | std::ios::trunc);
  file.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
  file.close();
}
