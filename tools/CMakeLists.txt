add_executable(writemidimsgs ${CMAKE_SOURCE_DIR}/tools/writeMidiMsgs.cpp)

target_link_libraries(writemidimsgs MidiParser)

add_executable(writemetamsgs ${CMAKE_SOURCE_DIR}/tools/writeMetaMsgs.cpp)

target_link_libraries(writemetamsgs MidiParser)
