#include "COPPERCodeGenerator.hh"

#include <xml/NodeLoader.hh>

#include <iostream>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 4) {
    std::cerr << "Usage : ./gui "
              << "<directory path to XML files> "
              << "<entry XML file name> "
              << "<directory path to outputs> "
              << std::endl;
    return 1;
  }

  const std::string dir = argv[1];
  const std::string entry = argv[2];
  const std::string output_path = argv[3];

  NodeLoader loader(dir);
  loader.load(entry);
  std::map<std::string, COPPERNode*>& copper_m(loader.getCOPPERNodeList());
  COPPERCodeGenerator generator(output_path);
  generator.setCOPPERs(copper_m);
  generator.create();

  return 0;
}
