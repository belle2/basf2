#ifndef _Belle2_DBObjectLoader_hh
#define _Belle2_DBObjectLoader_hh

#include "database/DBInterface.h"

#include "base/DataObject.h"

namespace Belle2 {

  class DBObjectLoader {

  public:
    DBObjectLoader(DBInterface* db) : _db(db) {}
    ~DBObjectLoader() {}

  public:
    void createTable(DataObject* obj);
    void dropTable(DataObject* obj);
    int readTable(std::vector<DataObject*>& obj_v, int version);
    int writeTable(std::vector<DataObject*>& obj_v, int version);

  private:
    DBInterface* _db;

  };

}

#endif
