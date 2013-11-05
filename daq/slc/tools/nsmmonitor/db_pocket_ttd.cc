#include <nsm/NSMNodeDaemon.h>
#include <nsm/NSMData.h>
#include <nsm/NSMCommunicator.h>

#include "pocket_ttd.h"

#include <database/MySQLInterface.h>

#include <base/StringUtil.h>

#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    std::cout << "Usage : ./db_pocket_ttd <dataname> <revision>"
              << std::endl;
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
  NSMData* data = new NSMData(argv[1], "pocket_ttd", atoi(argv[2]));
  pocket_ttd* status = (pocket_ttd*)data->parse();
  try {
    db->execute(Belle2::form("select * from %s_rev%d;",
                             data->getName().c_str(), data->getRevision()));
    DBRecordList& ret(db->loadRecords());
    for (size_t i = 0; i < ret.size(); i++) {
      data->setSQLValues(ret[i].getFieldNames(), ret[i].getFieldValues());
      std::cout << data->getName() << " : "
                << status->staa[0] << " " << status->evt_number << std::endl;
    }
  } catch (const DBHandlerException& e) {}

  return 0;
}
