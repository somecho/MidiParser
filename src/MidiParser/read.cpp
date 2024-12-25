#include "read.hpp"
#include "enums.hpp"

namespace MidiParser {

uint32_t vlqto32(std::stack<uint8_t>& s) {
  uint32_t out = 0;
  size_t size = s.size();
  for (size_t i = 0; i < size; ++i) {
    uint8_t in = s.top();
    out |= (in & 0b01111111) << (7 * i);
    s.pop();
  }
  return out;
}

uint32_t readvlq(std::vector<uint8_t>::iterator& it) {
  std::stack<uint8_t> s;
  uint8_t currByte = *it;
  s.push(currByte);
  while ((currByte & 0b10000000) != 0x0) {
    currByte = *++it;
    s.push(currByte);
  }
  return vlqto32(s);
}

MetaEvent readMetaEvent(std::vector<uint8_t>::iterator& it,
                        uint32_t deltaTime) {
  uint8_t metaType = *++it;
  uint32_t length = readvlq(++it);
  std::vector<uint8_t> data;
  for (uint32_t i = 0; i < length; i++) {
    data.emplace_back(*++it);
  }
  std::advance(it, 1);
  return MetaEvent{.deltaTime = deltaTime, .status = metaType, .data = data};
}

SysExEvent readSysExEvent(std::vector<uint8_t>::iterator& it,
                          uint32_t deltaTime) {
  std::vector<uint8_t> data;
  uint8_t next = *++it;
  while (next != 0xF7) {
    data.emplace_back(next);
    next = *++it;
  }
  std::advance(it, 1);
  return SysExEvent{.deltaTime = deltaTime, .data = data};
}

std::optional<MIDIEvent> readMidiEvent(std::vector<uint8_t>::iterator& it,
                                       uint32_t deltaTime) {
  if (StatusOnlyMIDI.contains(*it)) {
    auto e = MIDIEvent{.deltaTime = deltaTime, .status = *it};
    std::advance(it, 1);
    return e;
  }
  if (SingleByteMIDI.contains(*it & 0b11110000) ||
      SingleByteMIDI.contains(*it)) {
    auto e = MIDIEvent{.deltaTime = deltaTime, .status = *it, .data = {*++it}};
    std::advance(it, 1);
    return e;
  }
  if (DoubleByteMIDI.contains(*it & 0b11110000) ||
      DoubleByteMIDI.contains(*it)) {
    auto e = MIDIEvent{
        .deltaTime = deltaTime, .status = *it, .data = {*++it, *++it}};
    std::advance(it, 1);
    return e;
  }
  return std::nullopt;
}

std::optional<MIDIEvent> readMidiEvent(std::vector<uint8_t>::iterator& it,
                                       uint32_t deltaTime,
                                       uint8_t runningStatus) {
  if (SingleByteMIDI.contains(runningStatus & 0b11110000) ||
      SingleByteMIDI.contains(runningStatus)) {
    auto e = MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it}};
    std::advance(it, 1);
    return e;
  }
  if (DoubleByteMIDI.contains(runningStatus & 0b11110000) ||
      DoubleByteMIDI.contains(runningStatus)) {
    auto e = MIDIEvent{
        .deltaTime = deltaTime, .status = runningStatus, .data = {*it, *++it}};
    std::advance(it, 1);
    return e;
  }
  return std::nullopt;
}

}  // namespace MidiParser
