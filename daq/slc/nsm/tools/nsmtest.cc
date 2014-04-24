#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

namespace Belle2 {

  class NSMTestCallback : public RCCallback {

  public:
    NSMTestCallback(const NSMNode& node, DBInterface* db)
      : RCCallback(node), m_db(db) {}
    virtual ~NSMTestCallback() throw() {}

  public:
    virtual void init() throw() {
      NSMData data("HVTEST", "hv_status", 1);
      data.allocate(getCommunicator());
      data.parse();
      m_db->connect();
      LoggerObjectTable(m_db).add(data);
      m_db->close();
    }

    virtual bool boot() throw() {
      NSMMessage& msg(getCommunicator()->getMessage());
      ConfigObject& obj(getConfig().getObject());
      msg.getData(obj);
      obj.print();
      return true;
    }

  private:
    DBInterface* m_db;

  };

}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : ./nsmtest <name>");
    return 1;
  }
  const char* name = argv[1];
  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  NSMNode node(name);
  NSMTestCallback* callback = new NSMTestCallback(node, db);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
