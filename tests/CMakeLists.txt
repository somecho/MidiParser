enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(MidiParserTest ${CMAKE_SOURCE_DIR}/tests/Parser.test.cpp)

target_compile_features(MidiParserTest PUBLIC cxx_std_23)

target_sources(MidiParserTest
  PRIVATE
    ${MIDI_PARSER_SOURCES}
)

target_include_directories(MidiParserTest
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
)

target_compile_definitions(MidiParserTest
  PRIVATE
    EXAMPLES_DIR="${CMAKE_SOURCE_DIR}/data/midi_examples"
)

if(UNIX)
  target_compile_options(MidiParserTest PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wpedantic>
    $<$<CONFIG:Release>:-O3 -DNDEBUG -s -Wall -Wpedantic>
)
endif(UNIX)

if(MSVC)
  target_compile_options(MidiParserTest PRIVATE
    $<$<CONFIG:Debug>:/Zi /Od /Wall /W3 /RTC1>
    $<$<CONFIG:Release>: /O2 /DNDEBUG /W3 /WX /GS- /RTC1>
)
endif(MSVC)

if(WIN32)
  target_link_libraries(MidiParserTest PRIVATE ws2_32)
endif(WIN32)

target_link_libraries(MidiParserTest PRIVATE gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(MidiParserTest)

