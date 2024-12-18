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

std::streampos Scanner::getPos() const { return pos; }

uint32_t Scanner::getFileLength() const { return length; }

void Scanner::advance() {
  f.ignore();
  pos = f.tellg();
}

uint8_t Scanner::peek() { return f.peek(); }

void Scanner::seek(std::streampos position) {
  if (position < 0) {
    throw std::runtime_error("Attempting to provide a negative position!");
  }
  if (position >= static_cast<std::streampos>(length)) {
    auto msg = std::format("Attempting to scan beyond the end. Pos: {}",
                           static_cast<std::streamoff>(position + pos));
    throw std::runtime_error(msg);
  }
  f.seekg(position);
  pos = f.tellg();
}

}  // namespace MidiParser
