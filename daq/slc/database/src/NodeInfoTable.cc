#include "daq/slc/database/NodeInfoTable.h"

#include <daq/slc/system/LogFile.h>

using namespace Belle2;

NodeInfoList NodeInfoTable::getNodeInfos()
{
  NodeInfoList info_v;
  if (m_db != NULL) {
    try {
      m_db->execute("select * from nodenames()");
      DBRecordList record_v(m_db->loadRecords());
      for (DBRecordList::iterator it = record_v.begin();
           it != record_v.end(); it++) {
        DBRecord& record(*it);
        int id = record.getInt("id");
        if (id > 0) {
          info_v.push_back(NodeInfo(record.get("name"),
                                    record.get("group"), id));
        }
      }
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
  return info_v;
}

void NodeInfoTable::addNode(const NodeInfo& info)
{
  if (m_db != NULL) {
    try {
      m_db->execute("select addgroup as id from addgroup('%s');",
                    info.getGroup().c_str());
      m_db->execute("select addnode as id from addnode('%s', '%s');",
                    info.getName().c_str(), info.getGroup().c_str());
    } catch (const DBHandlerException& e) {
      LogFile::error("error on DB acess: %s", e.what());
    }
  }
}

