cmake_minimum_required(VERSION 3.15)

project(midiparser VERSION 0.1.0)

add_library(MidiParser)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(MIDI_PARSER_SOURCES
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Parser.cpp
)

set(MIDI_PARSER_HEADERS
  ${CMAKE_SOURCE_DIR}/src/MidiParser/Parser.hpp
  ${CMAKE_SOURCE_DIR}/src/MidiParser/enums.hpp
  ${CMAKE_SOURCE_DIR}/src/MidiParser/events.hpp
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
#add_subdirectory(tools)
