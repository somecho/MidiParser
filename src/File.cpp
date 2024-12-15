#include "File.h"

Midi::Format Midi::File::getType() { return m_type; }

uint16_t Midi::File::getDivision() { return m_division; }

std::ostream &operator<<(std::ostream &os, Midi::Format f) {
  switch (f) {
  case Midi::Format::SINGLE_TRACK:
    return os << "Single Track";
  case Midi::Format::MULTI_TRACK:
    return os << "Multi Track";
  case Midi::Format::MULTI_SONG:
    return os << "Multi Song";
  }
}
