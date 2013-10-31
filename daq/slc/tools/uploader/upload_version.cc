#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>

#include <base/StringUtil.h>

#include <xml/XMLParser.h>
#include <xml/NodeLoader.h>

#include <database/DBNodeSystemConfigurator.h>
#include <database/MySQLInterface.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 5) {
    std::cerr << "Usage : " << argv[0] << " "
              << "<directory path to XML files> "
              << "<old version> <operation_mode> <run_type> "
              << "<tag1>=<verion1>:<tag2>=<version2>... "
              << "<description>"
              << std::endl;
    return 1;
  }

  const std::string dir = argv[1];
  const std::string entry = argv[2];
  int old_version = atoi(argv[3]);
  int mode = (strcmp(argv[4], "global") == 0) ? NodeSystem::MODE_GLOBAL : NodeSystem::MODE_LOCAL;
  const char* run_type = argv[5];
  std::vector<std::string> str_v = Belle2::split(argv[6], ':');
  std::stringstream ss;
  for (int i = 7; i < argc; i++) {
    ss << argv[i] << " ";
  }

  NodeLoader loader(dir);
  loader.setVersion(0);
  loader.load(entry);
  NodeSystem& node_system(loader.getSystem());

  MySQLInterface db;
  DBNodeSystemConfigurator config(&db, &node_system);
  db.connect("localhost", "sample", "tkonno", "homerun0308", 0);
  int latest_version = -1;
  int temp_version = -1;
  db.execute("select version from version_control;");
  std::vector<DBRecord>& record_v(db.loadRecords());
  for (size_t i = 0; i < record_v.size(); i++) {
    int version = record_v[i].getFieldValueInt("version");
    if (version > latest_version) latest_version = version;
    if (version == old_version) temp_version = old_version;
  }
  int new_version = latest_version + 1;
  if (old_version == temp_version) {
    try {
      config.readTables(old_version);
      std::cerr << "Entry with version = " << old_version
                << " was set as template." << std::endl;
    } catch (const DBHandlerException& e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  } else {
    std::cerr << "No entry with version = " << old_version << " was founf."
              << "Default configuration was used." << std::endl;
  }
  for (size_t i = 0; i < str_v.size(); i++) {
    std::vector<std::string> label_v = Belle2::split(str_v[i], '=');
    int version = atoi(label_v[1].c_str());
    node_system.setVersion(label_v[0], version);
  }
  node_system.setVersion(new_version);
  node_system.setRunType(run_type);
  node_system.setDescription(ss.str());
  node_system.setOperationMode(mode);

  config.writeVersionControlTable();

  std::cerr << "new version : " << new_version << " has been recorded." << std::endl;
  db.close();
  return 0;
}
