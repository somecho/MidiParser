#pragma once

#include <functional>
#include <set>
#include <unordered_map>
#include <utility>

#include "enums.h"
#include "parser.h"

namespace MidiParser {

extern const std::set<std::pair<State, Event>> StateEvents;
extern const std::unordered_map<Event, State> MetaHandlers;
extern const std::set<Event> MidiMessages;

}  // namespace MidiParser
