#include "base/NodeSystem.h"

#include "base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const int NodeSystem::MODE_GLOBAL = 1;
const int NodeSystem::MODE_LOCAL = 0;

NodeSystem::~NodeSystem() throw()
{
  for (std::vector<NSMNode*>::iterator it = _node_v.begin();
       it != _node_v.end(); it++) {
    delete *it;
  }
  for (std::vector<NodeGroup*>::iterator it = _group_v.begin();
       it != _group_v.end(); it++) {
    delete *it;
  }
}

void NodeSystem::addNodeGroup(NodeGroup* group)
{
  _group_v.push_back(group);
  std::vector<NSMNode*>& node_v(group->getNodes());
  for (std::vector<NSMNode*>::iterator it = node_v.begin();
       it != node_v.end(); it++) {
    _node_v.push_back(*it);
  }
}

