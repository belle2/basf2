#include <nsm/NSMNodeDaemon.hh>
#include <nsm/NSMData.hh>
#include <nsm/NSMCommunicator.hh>

#include <db/MySQLInterface.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 2) {
    B2DAQ::debug("Usage : ./recvd <name>");
    return 1;
  }
  const char* name = argv[1];
  const std::string db_host = getenv("B2SC_DB_HOST");
  const std::string db_name = getenv("B2SC_DB_NAME");
  const std::string db_user = getenv("B2SC_DB_USER");
  const std::string db_password = getenv("B2SC_DB_PASS");
  const int db_port = atoi(getenv("B2SC_DB_PORT"));
  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(db_host, db_name, db_user, db_password, db_port);

  NSMNode* node = new NSMNode(name);
  NSMCommunicator* comm = new NSMCommunicator(node);
  comm->init();
  std::vector<NSMData*> data_v;
  data_v.push_back(new NSMData("RUN_STATUS", "run_status", 1));
  data_v.push_back(new NSMData("TTD_STATUS", "pocket_ttd", 5));
  for (size_t i = 0; i < data_v.size(); i++) {
    NSMData* data = data_v[i];
    data->open();
    try {
      db->execute(B2DAQ::form("create table %s_rev%d (%s);",
                              data->getName().c_str(), data->getRevision(),
                              data->toSQLConfig().c_str()));
    } catch (const DBHandlerException& e) {}
  }

  while (true) {
    for (size_t i = 0; i < data_v.size(); i++) {
      NSMData* data = data_v[i];
      db->execute(B2DAQ::form("insert into %s_rev%d values (%s);",
                              data->getName().c_str(), data->getRevision(),
                              data->toSQLValues().c_str()));
      std::cout << data->toSQLValues() << std::endl;
    }
    sleep(5);
  }

  return 0;
}
