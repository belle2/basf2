#include "daq/slc/apps/runcontrold/RunConfig.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"

using namespace Belle2;

RunConfig::RunConfig(const std::string& data_name, int revision) throw()
  : DataObject(data_name, "RunConfig"), _master(NULL)
{
  setRevision(revision);
  addText("run_type", "TESTT:DEFAULT", 64);
  addText("description", "Defualt settings", 128);
}

void RunConfig::add(std::map<std::string, DataObject*>& data_m,
                    std::vector<NSMNode*>& node_v)
{
  for (std::map<std::string, DataObject*>::iterator it = data_m.begin();
       it != data_m.end(); it++) {
    if (it->first.size() > 0) {
      DataObject* data = it->second;
      std::string label = it->first;;
      addInt(label, data->getConfigNumber());
      _data_m.insert(std::map<std::string, DataObject*>::value_type(label, data));
    }
  }
  for (std::vector<NSMNode*>::iterator it = node_v.begin();
       it != node_v.end(); it++) {
    NSMNode* node = *it;
    addBool(node->getName() + "_used", node->isUsed());
  }
}

void RunConfig::update()
{
  for (std::map<std::string, DataObject*>::iterator it = _data_m.begin();
       it != _data_m.end(); it++) {
    if (it->first.size() > 0) {
      DataObject* data = it->second;
      std::string label = it->first;
      setInt(label, data->getConfigNumber());
    }
  }
  if (_master != NULL) {
    RCMaster::NSMNodeList& node_v(_master->getNSMNodes());
    for (RCMaster::NSMNodeList::iterator it = node_v.begin();
         it != node_v.end(); it++) {
      NSMNode* node = *it;
      setBool(node->getName() + "_used", node->isUsed());
      if (hasValue(node->getName()) && node->getData() != NULL) {
        setInt(node->getName(), node->getData()->getConfigNumber());
      }
    }
  }
}
