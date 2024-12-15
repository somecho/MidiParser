add_library(MidiParser)

target_sources(MidiParser
  PRIVATE
    Scanner.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES Scanner.h
)

target_include_directories(MidiParser 
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(MidiParser PUBLIC cxx_std_23)
