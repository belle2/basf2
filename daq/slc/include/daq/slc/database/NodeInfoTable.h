#ifndef _Belle2_NodeInfoTable_h
#define _Belle2_NodeInfoTable_h

#include <daq/slc/database/DBInterface.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {

  class NodeInfo {

  public:
    NodeInfo() { m_id = 0; }
    NodeInfo(const std::string& name,
             const std::string& group, int id = 0)
      : m_name(name), m_group(group), m_id(id) {}
    ~NodeInfo() throw() {}

  public:
    const std::string& getName() const throw() { return m_name; }
    const std::string& getGroup() const throw() { return m_group; }
    int getId() const throw() { return m_id; }
    void setName(const std::string& name) throw() { m_name = name; }
    void setGroup(const std::string& group) throw() { m_group = group; }
    void setId(int id) throw() { m_id = id; }

  private:
    std::string m_name;
    std::string m_group;
    int m_id;

  };

  typedef std::vector<NodeInfo> NodeInfoList;
  typedef std::map<std::string, NodeInfoList> NodeInfoListMap;

  class NodeInfoTable {

  public:
    NodeInfoTable(DBInterface* db = NULL) : m_db(db) {}
    ~NodeInfoTable() throw() {}

  public:
    NodeInfoList getNodeInfos();
    void addNode(const NodeInfo& info);

  private:
    DBInterface* m_db;

  };

};

#endif
