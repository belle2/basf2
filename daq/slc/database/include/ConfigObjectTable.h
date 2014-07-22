#ifndef _Belle2_ConfigObjectTable_h
#define _Belle2_ConfigObjectTable_h

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/ConfigObject.h>

namespace Belle2 {

  class ConfigObjectTable {

  public:
    ConfigObjectTable(DBInterface* db = NULL) : m_db(db) {}
    ~ConfigObjectTable() throw() {}

  public:
    ConfigObjectList getList(const std::string& configname,
                             const std::string& nodename,
                             const std::string& tablename,
                             int revision) throw();
    ConfigObject get(const std::string& configname,
                     const std::string& nodename) throw();
    ConfigObjectList getList(int confid, bool isroot = false) throw();
    ConfigObject get(int confid) throw();
    void add(const ConfigObject& obj, bool isroot = true)
    throw(DBHandlerException);
    void add(const ConfigObjectList& obj, bool isroot = false) throw();
    void addAll(const ConfigObject& obj, bool isroot)
    throw(DBHandlerException);

  private:
    DBInterface* m_db;

  };

};

#endif
