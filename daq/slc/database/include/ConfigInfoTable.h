#ifndef _Belle2_ConfigInfoTable_h
#define _Belle2_ConfigInfoTable_h

#include <daq/slc/database/DBInterface.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {

  class ConfigInfo {

  public:
    ConfigInfo() { m_id = 0; }
    ConfigInfo(const std::string& name,
               const std::string& nodename,
               const std::string& tablename,
               int revision,
               int id = 0)
      : m_name(name), m_nodename(nodename),
        m_tablename(tablename), m_id(id),
        m_revision(revision) {}
    ~ConfigInfo() throw() {}

  public:
    const std::string& getName() const throw() { return m_name; }
    const std::string& getNode() const throw() { return m_nodename; }
    const std::string& getTable() const throw() { return m_tablename; }
    int getId() const throw() { return m_id; }
    int getRevision() const throw() { return m_revision; }
    void setName(const std::string& name) throw() { m_name = name; }
    void setTable(const std::string& tablename) throw() {
      m_tablename = tablename;
    }
    void setNode(const std::string& nodename) throw() {
      m_nodename = nodename;
    }
    void setId(int id) throw() { m_id = id; }
    void setRevision(int revision) throw() { m_revision = revision; }
    const std::string getSQL() const throw();

  private:
    std::string m_name;
    std::string m_nodename;
    std::string m_tablename;
    int m_id;
    int m_revision;

  };

  typedef std::vector<ConfigInfo> ConfigInfoList;
  typedef std::map<std::string, ConfigInfoList> ConfigInfoListMap;

  class ConfigInfoTable {

  public:
    ConfigInfoTable(DBInterface* db = NULL) : m_db(db) {}
    ~ConfigInfoTable() throw() {}

  public:
    ConfigInfo get(int configid);
    ConfigInfoList getList();
    ConfigInfoList getList(const std::string& nodename);
    void add(const ConfigInfo& info);

  public:
    void create() throw();

  private:
    DBInterface* m_db;

  };

};

#endif
