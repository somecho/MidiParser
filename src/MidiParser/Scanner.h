#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <vector>

namespace MidiParser {

class Scanner {
 public:
  explicit Scanner(const std::string &filePath);

  template <size_t N>
  std::array<uint8_t, N> scan() {
    std::array<uint8_t, N> buffer;
    if (!f.read(reinterpret_cast<char *>(buffer.data()), buffer.size())) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return buffer;
  }

  template <typename T>
  T scan() {
    T data{};
    if (!f.read(reinterpret_cast<char *>(&data), sizeof(T))) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return data;
  }

  std::vector<uint8_t> scan(size_t size);

  std::streampos getPos() const;

  uint32_t getFileLength() const;

 private:
  std::ifstream f;
  std::streampos pos;
  uint32_t length;
};

}  // namespace MidiParser
