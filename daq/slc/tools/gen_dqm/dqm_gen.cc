#include "HistoManagerGenerator.hh"

#include <iostream>

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "Usage : ./gui "
              << "<manager name> "
              << "<directory path to outputs> "
              << std::endl;
    return 1;
  }

  const std::string name = argv[1];
  const std::string output_path = argv[2];

  B2DQM::HistoManagerGenerator generator(name, output_path);
  generator.create();

  return 0;
}
