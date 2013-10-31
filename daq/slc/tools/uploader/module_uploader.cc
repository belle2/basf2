#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <base/StringUtil.h>

#include <xml/XMLParser.h>
#include <xml/NodeLoader.h>

#include <database/DBNodeSystemConfigurator.h>
#include <database/MySQLInterface.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 5) {
    std::cerr << "Usage : ./gui "
              << "<directory path to XML files> "
              << "<entry XML file name> <module_type> <filename> "
              << "[<old version>] [<new version>]"
              << std::endl;
    std::cerr << "file format:<copper node name>:<hslb slot>:"
              << "<data_name1>=<value1>,<value2>,...:<data_name2>=..."
              << std::endl;
    return 1;
  }

  const std::string dir = argv[1];
  const std::string entry = argv[2];
  const char* module_type = argv[3];
  const std::string file_name = argv[4];
  int old_version = (argc > 5) ? atoi(argv[5]) : -1;
  int new_version = (argc > 6) ? atoi(argv[6]) : -1;

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);
  NodeSystem& node_system(loader.getSystem());

  std::vector<COPPERNode*>& copper_v(node_system.getCOPPERNodes());
  MySQLInterface db;
  db.connect("localhost", "sample", "tkonno", "homerun0308", 0);
  int latest_version = -1;
  int temp_version = -1;
  db.execute(Belle2::form("select version from %s_conf;", Belle2::tolower(module_type).c_str()));
  std::vector<DBRecord>& record_v(db.loadRecords());
  for (size_t i = 0; i < record_v.size(); i++) {
    int version = record_v[i].getFieldValueInt("version");
    if (version > latest_version) latest_version = version;
    if (new_version == version && temp_version != version) {
      temp_version = version;
      std::cerr << "version : " << new_version << " is already exits." << std::endl;
    }
  }
  if (old_version < 0) old_version = latest_version;
  if (new_version < 0) new_version = latest_version + 1;

  DBNodeSystemConfigurator config(&db, &node_system);
  std::cerr << "old version : " << new_version << " is used as template." << std::endl;
  config.readFEEModuleTable(module_type, node_system.getModules(module_type), old_version);

  std::fstream fin(file_name.c_str());
  std::string str;
  while (fin >> str) {
    std::cout << str << std::endl;
    std::vector<std::string> str_v(Belle2::split(str, ':'));
    for (std::vector<COPPERNode*>::iterator it = copper_v.begin();
         it != copper_v.end(); it++) {
      COPPERNode* copper = *it;
      if (copper->getName() == str_v[0]) {
        int slot = (int)(str_v[1].at(0) - 'a');
        if (slot >= 0 && slot < 4) {
          HSLB* hslb = copper->getHSLB(slot);
          if (hslb != NULL) {
            FEEModule* module = hslb->getFEEModule();
            if (module != NULL) {
              for (size_t i = 2; i < str_v.size(); i++) {
                std::vector<std::string> label_v(Belle2::split(str_v[i], '='));
                if (label_v.size() != 2) continue;
                FEEModule::Register* reg = module->getRegister(label_v[0]);
                if (reg != NULL) {
                  std::vector<std::string> value_v(Belle2::split(label_v[1], ','));
                  for (size_t ch = 0; ch < value_v.size(); ch++) {
                    reg->setValue(ch, atoi(value_v[ch].c_str()));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  config.writeFEEModuleTable(module_type, node_system.getModules(module_type), new_version);

  std::cerr << "new version : " << new_version << " has been recorded." << std::endl;
  db.close();
  return 0;
}
