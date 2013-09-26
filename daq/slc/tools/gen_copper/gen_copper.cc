#include "COPPERCodeGenerator.hh"

#include <xml/NodeLoader.hh>

#include <iostream>
#include <cstdlib>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  /*
  if (argc < 4) {
    std::cerr << "Usage : ./gui ["
              << "<directory path to XML files> "
              << "<entry XML file name> "
              << "<directory path to outputs> ]"
              << std::endl;
    return 1;
  }
  */
  const std::string dir = (argc > 1) ? argv[1] : getenv("B2SC_XML_PATH");
  const std::string entry = (argc > 2) ? argv[2] : getenv("B2SC_XML_ENTRY");
  const std::string output_path = (argc > 3) ? argv[3] : getenv("B2SC_CPRLIB_PATH");

  NodeLoader loader(dir);
  loader.load(entry);
  std::map<std::string, COPPERNode*>& copper_m(loader.getCOPPERNodeList());
  COPPERCodeGenerator generator(output_path);
  generator.setCOPPERs(copper_m);
  generator.create();

  return 0;
}
