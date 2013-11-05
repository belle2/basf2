#ifndef _Belle2_DBNodeSystemConfigurator_hh
#define _Belle2_DBNodeSystemConfigurator_hh

#include "database/DBObjectLoader.h"
#include "database/DBInterface.h"

#include "xml/NodeLoader.h"

namespace Belle2 {

  class DBNodeSystemConfigurator {

  public:
    DBNodeSystemConfigurator(DBInterface* db = NULL,
                             NodeLoader* loader = NULL) throw();
    virtual ~DBNodeSystemConfigurator() throw() {}

  public:
    DBInterface* getDB() { return _db; }
    NodeLoader* getNodeLoader() { return _loader; }
    std::map<std::string, int>& getVersions() { return _version_m; }
    void setVersion(const std::string& name, int version) {
      _version_m[name] = version;
    }

  public:
    void createTable();
    void dropTable();
    int rereadTable();
    int readTable(int revision);
    int writeTable(int revision);

  private:
    DBInterface* _db;
    NodeLoader* _loader;
    DBObjectLoader _dbloader;
    std::map<std::string, int> _version_m;

  };

}

#endif

