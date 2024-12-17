#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <vector>

namespace MidiParser {

class Scanner {
public:
  explicit Scanner(const std::string &filePath) : f(filePath) {
    if (!f) {
      throw std::runtime_error(std::format("Failed to open file {}", filePath));
    }
    f.seekg(0, std::ios::end);
    length = f.tellg();
    f.seekg(0, std::ios::beg);
    pos = f.tellg();
  }

  template <size_t N> std::array<uint8_t, N> scan() {
    std::array<uint8_t, N> buffer;
    if (!f.read(reinterpret_cast<char *>(buffer.data()), buffer.size())) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return buffer;
  }

  template <typename T> T scan() {
    T data;
    if (!f.read(reinterpret_cast<char *>(&data), sizeof(T))) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return data;
  }

  std::vector<uint8_t> scan(size_t size) {
    std::vector<uint8_t> buffer(size);
    if (!f.read(reinterpret_cast<char *>(buffer.data()), size)) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return buffer;
  }

  std::streampos getPos();

  uint32_t getFileLength();

  /// Advances the stream position by 1 byte.
  void advance();

  /// Views the byte at the current position without moving the stream position.
  uint8_t peek();

  void seek(std::streampos position);

private:
  std::ifstream f;
  std::streampos pos;
  uint32_t length;
};

} // namespace MidiParser
