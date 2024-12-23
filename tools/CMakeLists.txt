add_executable(benchmark ${CMAKE_SOURCE_DIR}/tools/benchmark.cpp)
target_link_libraries(benchmark MidiParser)
