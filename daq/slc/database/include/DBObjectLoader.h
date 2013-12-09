#ifndef _Belle2_DBObjectLoader_hh
#define _Belle2_DBObjectLoader_hh

#include "daq/slc/database/DBInterface.h"

#include "daq/slc/base/DataObject.h"

namespace Belle2 {

  class DBObjectLoader {

  public:
    DBObjectLoader(DBInterface* db) : _db(db) {}
    ~DBObjectLoader() {}

  public:
    void create(DataObject* obj);
    void drop(DataObject* obj);
    DBRecordList& readAll(DataObject* obj);
    void read(DataObject* obj);
    void write(DataObject* obj);
    int getLatestConfig(DataObject* obj);
    std::vector<int> getConfigList(DataObject* obj);

  private:
    DBInterface* _db;

  };

}

#endif
