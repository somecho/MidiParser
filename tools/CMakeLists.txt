add_executable(writemidimsgs ${CMAKE_SOURCE_DIR}/tools/writeMidiMsgs.cpp)

target_link_libraries(writemidimsgs MidiParser)

