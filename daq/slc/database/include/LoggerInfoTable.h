#ifndef _Belle2_LoggerInfoTable_h
#define _Belle2_LoggerInfoTable_h

#include <daq/slc/database/DBInterface.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {

  class LoggerInfo {

  public:
    LoggerInfo() { m_id = 0; }
    LoggerInfo(const std::string& nodename,
               const std::string& tablename,
               int revision, int id = 0)
      : m_nodename(nodename),
        m_tablename(tablename), m_id(id),
        m_revision(revision) {}
    ~LoggerInfo() throw() {}

  public:
    const std::string& getNode() const throw() { return m_nodename; }
    const std::string& getTable() const throw() { return m_tablename; }
    int getId() const throw() { return m_id; }
    int getRevision() const throw() { return m_revision; }
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
    std::string m_nodename;
    std::string m_tablename;
    int m_id;
    int m_revision;

  };

  typedef std::vector<LoggerInfo> LoggerInfoList;
  typedef std::map<std::string, LoggerInfoList> LoggerInfoListMap;

  class LoggerInfoTable {

  public:
    LoggerInfoTable(DBInterface* db = NULL) : m_db(db) {}
    ~LoggerInfoTable() throw() {}

  public:
    LoggerInfo get(int id);
    LoggerInfoList getList();
    LoggerInfoList getList(const std::string& nodename);
    int add(const LoggerInfo& info);

  public:
    void create() throw();

  private:
    DBInterface* m_db;

  };

};

#endif
