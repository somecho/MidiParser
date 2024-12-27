enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(MidiParserTest
  ${CMAKE_CURRENT_SOURCE_DIR}/Parser.test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/vlqto32.test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/events.test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/read.test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/regression.test.cpp
)

target_compile_features(MidiParserTest PUBLIC cxx_std_23)

target_sources(MidiParserTest
  PRIVATE
    ${MIDI_PARSER_SOURCES}
)

target_include_directories(MidiParserTest
  PRIVATE
  ${MIDI_PARSER_DIR}
)

target_compile_definitions(MidiParserTest
  PRIVATE
  EXAMPLES_DIR="${MIDI_PARSER_DATA_DIR}/midi_examples"
)

target_link_libraries(MidiParserTest PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(MidiParserTest)

