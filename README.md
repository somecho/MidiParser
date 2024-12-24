![GitHub Tag](https://img.shields.io/github/v/tag/somecho/MidiParser)
# MidiParser 

A simple and straightforward library for parsing General MIDI files and follows [this specification](http://www.somascape.org/midi/tech/mfile.html#delta).

## Usage

The easiest way to use this library is via CMake.

### CMake 

In your `CMakeLists.txt`, you can use FetchContent like so: 

```cmake
include(FetchContent)

FetchContent_Declare(
  midiparser
  GIT_REPOSITORY https://github.com/somecho/MidiParser.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(midiparser)

target_link_libraries(TARGET_APP PRIVATE MidiParser)
```

### Library Usage

Here's a simple application that uses MidiParser: 

```cpp
#include <MidiParser/Parser.hpp>

int main() {
  MidiParser::Parser parser;
  MidiParser::MidiFile midifile = parser.parse("path/to/file.mid");
}
```

You can try it with the [example midi files](./data/midi_examples). 

---

© 2024 Somē Cho 
