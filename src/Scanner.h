#pragma once

#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>

namespace Midi {

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
  };

  template <size_t N> std::array<uint8_t, N> scan() {
    std::array<uint8_t, N> buffer;
    if (!f.read(reinterpret_cast<char *>(buffer.data()), buffer.size())) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return buffer;
  };

  template <typename T> T scan() {
    T data;
    if (!f.read(reinterpret_cast<char *>(&data), sizeof(T))) {
      throw std::runtime_error(std::format("Failed to read from position {}",
                                           static_cast<uint16_t>(pos)));
    };
    pos = f.tellg();
    return data;
  };

  std::streampos getPos();
  uint32_t getFileLength();
  void seek(std::streampos position);

private:
  std::ifstream f;
  std::streampos pos;
  uint32_t length;
};

}; // namespace Midi
