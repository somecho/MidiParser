cmake_minimum_required(VERSION 3.15)

project(midiparser)

add_library(MidiParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(MIDI_PARSER_SOURCES
  ${CMAKE_SOURCE_DIR}/src/Scanner.cpp
  ${CMAKE_SOURCE_DIR}/src/Parser.cpp
  ${CMAKE_SOURCE_DIR}/src/File.cpp
)

set(MIDI_PARSER_HEADERS
  ${CMAKE_SOURCE_DIR}/src/Scanner.h
  ${CMAKE_SOURCE_DIR}/src/Parser.h
  ${CMAKE_SOURCE_DIR}/src/File.h
)


target_sources(MidiParser
  PRIVATE
    ${MIDI_PARSER_SOURCES}
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/src
    FILES ${MIDI_PARSER_HEADERS}
)

target_include_directories(MidiParser
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(MidiParser PUBLIC cxx_std_23)

add_subdirectory(tests)



