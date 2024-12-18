cmake_minimum_required(VERSION 3.15)

project(midiparser)

add_library(MidiParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(MIDI_PARSER_SOURCES
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Parser.cpp
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Scanner.cpp
  ${CMAKE_SOURCE_DIR}/src/MidiParser/tables.cpp
)

set(MIDI_PARSER_HEADERS
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Parser.h
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Scanner.h
  ${CMAKE_SOURCE_DIR}/src/MidiParser/enums.h
  ${CMAKE_SOURCE_DIR}/src/MidiParser/tables.h
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

if(WIN32)
target_link_libraries(MidiParser PRIVATE ws2_32)
endif(WIN32)


add_subdirectory(tests)
