#include "Parser.h"
#include "HeaderChunk.h"
#include "Scanner.h"
#include <cstdint>
#include <netinet/in.h>

Midi::File Midi::Parser::parse(std::string midiFilePath) {
  auto scanner = Scanner(midiFilePath);
  auto headerBytes = scanner.scan<4>();

  if (!std::equal(HeaderChunk::Identifier.begin(),
                  HeaderChunk::Identifier.end(), headerBytes.begin())) {
    throw std::runtime_error("Invalid midi file. Midi header not found.");
  }

  auto chunkLength = ntohl(scanner.scan<uint32_t>());
  if (chunkLength != 6) {
    throw std::runtime_error(
        "Invalid midi file. Header chunk length is not 6.");
  }

  auto header =
      Midi::HeaderChunk{.length = chunkLength,
                        .format = Midi::Format(ntohs(scanner.scan<uint16_t>())),
                        .numTracks = ntohs(scanner.scan<uint16_t>()),
                        .division = ntohs(scanner.scan<uint16_t>())};

  return Midi::File(header.format, header.division); // TEMPORARY
}
