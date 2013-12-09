#include "daq/slc/apps/runcontrold/RunStatus.h"
#include "daq/slc/apps/runcontrold/RunConfig.h"

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RunStatus::RunStatus(const std::string& data_name, int revision) throw()
  : DataObject(data_name, "RunStatus")
{
  setRevision(revision);
  addInt("exp_number", 0);
  addInt("cold_number", 0);
  addInt("hot_number", 0);
  addInt("start_time", 0);
  addInt("end_time", -1);
  addInt("run_config", 0);
  addText("operators", "", 64);
}

int RunStatus::incrementExpNumber() throw()
{
  int exp_number = getExpNumber();
  exp_number++;
  setExpNumber(exp_number);
  return exp_number;
}

int RunStatus::incrementColdNumber() throw()
{
  int run_number = getColdNumber();
  run_number++;
  setColdNumber(run_number);
  return run_number;
}

int RunStatus::incrementHotNumber() throw()
{
  int run_number = getHotNumber();
  run_number++;
  setHotNumber(run_number);
  return run_number;
}

void RunStatus::add(NSMNode* node_in)
{
  NSMNode* node = NULL;
  for (std::vector<NSMNode*>::iterator it = _node_v.begin();
       it != _node_v.end(); it++) {
    if (node_in == *it) {
      node = node_in;
    }
  }
  if (node == node_in) {
    setInt(node->getName() + "_state", node->getState().getId());
    setInt(node->getName() + "_connection", node->getConnection().getId());
  } else {
    node = node_in;
    addInt(node->getName() + "_state", node->getState().getId());
    addInt(node->getName() + "_connection", node->getConnection().getId());
    _node_v.push_back(node);
  }
}

void RunStatus::update()
{
  for (size_t i = 0; i < _node_v.size(); i++) {
    NSMNode* node = _node_v[i];
    setInt(node->getName() + "_state", node->getState().getId());
    setInt(node->getName() + "_connection", node->getConnection().getId());
  }
  setInt("run_config", _config->getConfigNumber());
}
