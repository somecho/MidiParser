cmake_minimum_required(VERSION 3.15)

project(midiparser VERSION 0.2.0)

add_library(MidiParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(MIDI_PARSER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/MidiParser)
set(MIDI_PARSER_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

set(MIDI_PARSER_SOURCES
  ${MIDI_PARSER_DIR}/Parser.cpp
)

set(MIDI_PARSER_HEADERS
  ${MIDI_PARSER_DIR}/Parser.hpp
  ${MIDI_PARSER_DIR}/enums.hpp
  ${MIDI_PARSER_DIR}/events.hpp
)

target_sources(MidiParser
  PRIVATE
    ${MIDI_PARSER_SOURCES}
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES ${MIDI_PARSER_HEADERS}
)

target_include_directories(MidiParser
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(MidiParser PUBLIC cxx_std_23)

if(WIN32)
  target_link_libraries(MidiParser PRIVATE ws2_32)
endif(WIN32)

if(UNIX)
  target_compile_options(MidiParser PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wpedantic>
    $<$<CONFIG:Release>:-O3 -DNDEBUG -s -Wall -Wpedantic>
)
endif(UNIX)

if(MSVC)
  target_compile_options(MidiParser PRIVATE
    $<$<CONFIG:Debug>:/Zi /Od /Wall /W3 /RTC1>
    $<$<CONFIG:Release>: /O2 /DNDEBUG /W3 /WX /GS- /RTC1>
)
endif(MSVC)


add_subdirectory(tests)
add_subdirectory(tools)

add_executable(example_noteon 
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/MidiParser_Example_NoteOnEvents.cpp)
target_link_libraries(example_noteon PRIVATE MidiParser)
  target_compile_options(example_noteon PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wpedantic>
    $<$<CONFIG:Release>:-O3 -DNDEBUG -s -Wall -Wpedantic>
)
