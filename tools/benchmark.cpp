#include <MidiParser/Parser.h>

int main(int argc, char* argv[]) {
  auto parser = MidiParser::Parser(argv[1]);
  parser.parse();
}
