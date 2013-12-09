#ifndef _Belle2_RCDatabaseManager_hh
#define _Belle2_RCDatabaseManager_hh

#include <daq/slc/database/DBObjectLoader.h>
#include <daq/slc/database/DBInterface.h>

namespace Belle2 {

  class RCMaster;

  class RCDatabaseManager {

    typedef std::vector<DataObject*> DataObjectList;
    typedef std::map<std::string, DataObjectList > DataObjectSet;

  public:
    RCDatabaseManager(DBInterface* db, RCMaster* master);
    virtual ~RCDatabaseManager() throw() {}

  public:
    void createTables();
    void dropTables();
    int readConfigs(int confno = -1);
    int writeConfigs();
    int readConfig(const std::string classname, int confno = -1);
    int writeConfig(const std::string classname);
    int readStatus(int confno = -1);
    int writeStatus();

  private:
    void search(DataObject* data);

  private:
    DBInterface* _db;
    RCMaster* _master;
    DataObjectSet _data_v_m;

  };

}

#endif
