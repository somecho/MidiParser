#include <MidiParser/Parser.hpp>

int main(int argc, char* argv[]) {
  auto parser = MidiParser::Parser();
  parser.parse(argv[1]);
}
