#pragma once

#include <optional>
#include <stack>
#include "events.hpp"

namespace MidiParser {

uint32_t vlqto32(std::stack<uint8_t>& s);

uint32_t readvlq(std::vector<uint8_t>::iterator& it);

MetaEvent readMetaEvent(std::vector<uint8_t>::iterator& it, uint32_t deltaTime);

SysExEvent readSysExEvent(std::vector<uint8_t>::iterator& it,
                          uint32_t deltaTime);

std::optional<MIDIEvent> readMidiEvent(std::vector<uint8_t>::iterator& it,
                                       uint32_t deltaTime);

std::optional<MIDIEvent> readMidiEvent(std::vector<uint8_t>::iterator& it,
                                       uint32_t deltaTime,
                                       uint8_t runningStatus);

}  // namespace MidiParser
