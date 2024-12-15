cmake_minimum_required(VERSION 3.15)

project(midiparser)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)


add_executable(midiparser src/main.cpp)

target_sources(midiparser
  PRIVATE
    src/events.h
    src/Scanner.cpp
)

target_compile_features(midiparser PUBLIC cxx_std_23)


add_subdirectory(src)
add_subdirectory(tests)



