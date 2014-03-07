#include "daq/slc/apps/hvcontrold/HVControlMaster.h"

#include "daq/slc/apps/hvcontrold/hv_status.h"

using namespace Belle2;

void HVControlMaster::addNode(NSMNode* node, const std::string& filename)
{
  const std::string nodename = node->getName();
  _name_v.push_back(nodename);
  NSMData* data = new NSMData(nodename + "_STATUS", "hv_status", hv_status_revision);
  _nsmdata_m.insert(HVNSMDataList::value_type(nodename, data));
  HVNodeInfo info(node);
  info.readFile(filename);
  _node_m.insert(HVNodeInfoList::value_type(nodename, info));
}

void HVControlMaster::createTables()
{
  _db->connect();
  for (HVNodeInfoList::iterator it = _node_m.begin();
       it != _node_m.end(); it++) {
    HVNodeInfo& info(it->second);
    int id = info.create(_db);
    if (id == 0 || info.getConfigId() > id) {
      info.save(_db);
    }
  }
  _db->close();
}
