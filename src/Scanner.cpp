#include "Scanner.h"
#include <cstdint>
#include <ios>
#include <stdexcept>

std::streampos Midi::Scanner::getPos() { return pos; }

uint32_t Midi::Scanner::getFileLength() { return length; }

void Midi::Scanner::seek(std::streampos position) {
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
