#include <fstream>
#include <string>

#include "parser.hh"

auto main() -> int {
  std::ifstream file{"input.txt"};
  if (!file) {
    std::cerr << "Failed to open the file" << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(file, line)) {
    auto result = Parser::Parse(line);
    Parser::PrintInformation(std::cout, result);
  }
  return 0;
}