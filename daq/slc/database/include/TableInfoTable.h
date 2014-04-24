#ifndef _Belle2_TableInfoTable_h
#define _Belle2_TableInfoTable_h

#include <daq/slc/database/DBInterface.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {

  class TableInfo {
  public:
    enum Type {
      CONFIG = 1, LOGGER
    };

  public:
    TableInfo() {}
    TableInfo(const std::string name, bool isconfig,
              int revision, bool isroot, int id = 0)
      : m_name(name), m_isconfig(isconfig),
        m_revision(revision), m_isroot(isroot),
        m_id(id) {}
    ~TableInfo() throw() {}

  public:
    const std::string& getName() const throw() { return m_name; }
    int getRevision() const throw() { return m_revision; }
    bool isConfig() const throw() { return m_isconfig; }
    bool isRoot() const throw() { return m_isroot; }
    int getId() const throw() { return m_id; }
    void setName(const std::string& name) throw() { m_name = name; }
    void setRevision(int revision) throw() { m_revision = revision; }
    void setRoot(bool isroot) throw() { m_isroot = isroot; }
    void setConfig(bool isconfig) throw() { m_isconfig = isconfig; }
    void setId(int id) throw() { m_id = id; }

  private:
    std::string m_name;
    bool m_isconfig;
    int m_revision;
    bool m_isroot;
    int m_id;

  };

  typedef std::vector<TableInfo> TableInfoList;
  typedef std::map<std::string, TableInfoList> TableInfoListMap;

  class TableInfoTable {

  public:
    TableInfoTable(DBInterface* db = NULL) : m_db(db) {}
    ~TableInfoTable() throw() {}

  public:
    TableInfoList getList() throw();
    TableInfo get(const std::string& tablename, int revision) throw();
    int add(const TableInfo& info) throw();

  private:
    DBInterface* m_db;

  };

};

#endif
