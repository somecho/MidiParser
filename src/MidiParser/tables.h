#pragma once

#include <set>
#include <utility>

#include "enums.h"

namespace MidiParser {

extern const std::set<std::pair<State, Event>> StateEvents;

}
