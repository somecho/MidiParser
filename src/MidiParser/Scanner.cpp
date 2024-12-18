#include "Scanner.h"

#include <ios>

namespace MidiParser {

 Scanner::Scanner(const std::string& filePath) : f(filePath, std::ios::binary) {
  if (!f) {
    throw std::runtime_error(std::format("Failed to open file {}", filePath));
  }
  f.seekg(0, std::ios::end);
  length = f.tellg();
  f.seekg(0, std::ios::beg);
  pos = f.tellg();
}

std::vector<uint8_t> Scanner::scan(size_t size) {
  std::vector<uint8_t> buffer(size);
  if (!f.read(reinterpret_cast<char*>(buffer.data()), size)) {
    throw std::runtime_error(std::format("Failed to read from position {}",
                                         static_cast<uint16_t>(pos)));
  };
  pos = f.tellg();
  return buffer;
}

std::streampos Scanner::getPos() const { return pos; }

uint32_t Scanner::getFileLength() const { return length; }

}  // namespace MidiParser
