#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include <database/MySQLInterface.h>

#include <base/StringUtil.h>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage : ./nsmmond <name> "
              << "[<dataname:formatname:rev>...]" << std::endl;
    return 1;
  }
  const char* name = argv[1];
  NSMNode* node = new NSMNode(name);
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init();

  const std::string db_host = getenv("B2SC_DB_HOST");
  const std::string db_name = getenv("B2SC_DB_NAME");
  const std::string db_user = getenv("B2SC_DB_USER");
  const std::string db_password = getenv("B2SC_DB_PASS");
  const int db_port = atoi(getenv("B2SC_DB_PORT"));
  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(db_host, db_name, db_user, db_password, db_port);

  std::vector<NSMData*> data_v;
  for (int i = 2; i < argc; i++) {
    std::vector<std::string> str_v = Belle2::split(argv[i], ':');
    data_v.push_back(new NSMData(str_v[0], str_v[1], atoi(str_v[2].c_str())));
  }
  for (size_t i = 0; i < data_v.size(); i++) {
    NSMData* data = data_v[i];
    data->open();
    try {
      db->execute(Belle2::form("create table %s_rev%d (%s);",
                               data->getName().c_str(), data->getRevision(),
                               data->toSQLConfig().c_str()));
    } catch (const DBHandlerException& e) {}
  }

  while (true) {
    for (size_t i = 0; i < data_v.size(); i++) {
      NSMData* data = data_v[i];
      db->execute(Belle2::form("insert into %s_rev%d values (%s);",
                               data->getName().c_str(), data->getRevision(),
                               data->toSQLValues().c_str()));
      std::cout << data->getName() << " : "
                << data->toSQLValues() << std::endl;
    }
    sleep(5);
  }

  return 0;
}
