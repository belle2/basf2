#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <base/StringUtil.h>

#include <xml/XMLParser.h>
#include <xml/NodeLoader.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cerr << "Usage : ./gui "
              << "<directory path to XML files> "
              << "<entry XML file name> <module_type>"
              << std::endl;
    return 1;
  }

  const std::string dir = argv[1];
  const std::string entry = argv[2];
  const char* module_type = argv[3];

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);
  NodeSystem& node_system(loader.getSystem());

  std::vector<COPPERNode*>& copper_v(node_system.getCOPPERNodes());
  for (std::vector<COPPERNode*>::iterator it = copper_v.begin();
       it != copper_v.end(); it++) {
    COPPERNode* copper = *it;
    for (size_t slot = 0; slot < 4; slot++) {
      HSLB* hslb = copper->getHSLB(slot);
      if (hslb != NULL) {
        FEEModule* module = hslb->getFEEModule();
        if (module != NULL && module->getType() == module_type) {
          std::cout << Belle2::form("%s:%c:", copper->getName().c_str(), (char)(slot + 'a'));
          FEEModule::RegisterList& reg_v(module->getRegisters());
          for (size_t i = 0; true;) {
            FEEModule::Register& reg(reg_v[i]);
            std::cout << reg.getName() << "=";
            for (size_t ch = 0; true;) {
              std::cout << reg.getValue(ch);
              ch++;
              if (ch == reg.length()) break;
              std::cout << ",";
            }
            i++;
            if (i == reg_v.size()) break;
            std::cout << ":";
          }
          std::cout << std::endl;
        }
      }
    }
  }
  return 0;
}
