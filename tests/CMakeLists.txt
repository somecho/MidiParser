include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    doctest
    PREFIX ${CMAKE_BINARY_DIR}/doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(doctest source_dir)
set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")


enable_testing()
add_executable(MidiParserTest main.test.cpp)
target_sources(MidiParserTest
  PRIVATE
    Scanner.test.cpp
)
target_compile_features(MidiParserTest PUBLIC cxx_std_23)
target_include_directories(MidiParserTest PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(MidiParserTest PRIVATE MidiParser)
target_compile_definitions(MidiParserTest PRIVATE 
  MIDI_FILES_DIR="${CMAKE_SOURCE_DIR}/midi_examples")

add_test(NAME MidiParserTest 
         COMMAND MidiParserTest --success true)
