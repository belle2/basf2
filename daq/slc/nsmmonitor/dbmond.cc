#include <nsm/NSMNodeDaemon.hh>
#include <nsm/NSMData.hh>
#include <nsm/NSMCommunicator.hh>

#include <db/MySQLInterface.hh>

#include <util/StringUtil.hh>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace B2DAQ;

int main(int argc, char** argv)
{
  if (argc < 1) {
    std::cout << "Usage : ./dbmond "
              << "[<dataname:formatname:rev>...]" << std::endl;
    return 1;
  }
  const std::string db_host = getenv("B2SC_DB_HOST");
  const std::string db_name = getenv("B2SC_DB_NAME");
  const std::string db_user = getenv("B2SC_DB_USER");
  const std::string db_password = getenv("B2SC_DB_PASS");
  const int db_port = atoi(getenv("B2SC_DB_PORT"));
  MySQLInterface* db = new MySQLInterface();
  db->init();
  db->connect(db_host, db_name, db_user, db_password, db_port);

  std::vector<NSMData*> data_v;
  for (int i = 1; i < argc; i++) {
    std::vector<std::string> str_v = B2DAQ::split(argv[i], ':');
    data_v.push_back(new NSMData(str_v[0], str_v[1], atoi(str_v[2].c_str())));
  }
  for (size_t i = 0; i < data_v.size(); i++) {
    NSMData* data = data_v[i];
    //data->parse("/home/usr/tkonno/b2slc/bin");
    data->parse();
    try {
      std::cout << B2DAQ::form("select * from %s_rev%d;",
                               data->getName().c_str(), data->getRevision()) << std::endl;
      db->execute(B2DAQ::form("select * from %s_rev%d;",
                              data->getName().c_str(), data->getRevision()));
      DBRecordList& ret(db->loadRecords());
      for (size_t i = 0; i < ret.size(); i++) {
        data->setSQLValues(ret[i].getFieldNames(), ret[i].getFieldValues());
        std::cout << data->getName() << " : "
                  << data->toSQLValues() << std::endl;
      }
    } catch (const DBHandlerException& e) {}
  }

  return 0;
}
