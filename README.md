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

You can try it with the [example midi files](./data/midi_examples). For more information about how to use this library, see the [simple examples](./examples) provided.

## Building 

Using the CMake CLI, MidiParser can be built like so:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RELEASE # or DEBUG
cmake --build build
```

On Visual Studio, open this repository as a CMake project. Build configuration can be set in the 'Manage Configurations' menu.

### Tests, Tools, Examples

To build these parts of MidiParser, use these flags:
- `-DBUILD_TESTS=ON` to build tests
- `-DBUILD_TOOLS=ON` to build tools
- `-DBUILD_EXAMPLES=ON` to build examples

### Running tests

The test is an executable binary that gets built to `./build/tests/MidiParserTest` if you use the CLI method to build. On Visual Studio, CMake builds to `./out` by default. 

---

© 2024 Somē Cho 
