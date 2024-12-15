#pragma once

#include <cstdint>
#include <ostream>

namespace Midi {

enum class Format : uint16_t {
  SINGLE_TRACK = 0,
  MULTI_TRACK = 1,
  MULTI_SONG = 2
};

class File {
public:
  File(Format type, uint16_t division) : m_type(type), m_division(division) {};

  Format getType();
  uint16_t getDivision();

private:
  Format m_type;
  uint16_t m_division;
};

} // namespace Midi

std::ostream &operator<<(std::ostream &os, Midi::Format f);
