#pragma once

#include <set>
#include <utility>
#include <unordered_map>

#include "enums.h"

namespace MidiParser {

extern const std::set<std::pair<State, Event>> StateEvents;
extern const std::unordered_map<Event, State> MetaHandlers;
extern const std::set<Event> MidiMessages;

}
