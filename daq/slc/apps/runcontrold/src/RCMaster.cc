#include "daq/slc/apps/runcontrold/RCMaster.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RCMaster::RCMaster(NSMNode* node,
                   RunConfig* config, RunStatus* status)
{
  _master_node = node;
  _config = config;
  if (_config != NULL) _config->setMaster(this);
  _status = status;
  _is_sending = false;
}

RCMaster::~RCMaster() throw()
{

}

void RCMaster::lock() throw()
{
  _mutex.lock();
}

void RCMaster::unlock() throw()
{
  _mutex.unlock();
}

void RCMaster::wait() throw()
{
  _cond.wait(_mutex);
}

void RCMaster::signal() throw()
{
  _cond.signal();
}

NSMNode* RCMaster::findNode(int id, const NSMMessage& msg) throw()
{
  NSMNode* node = getNodeByID(id);
  if (node == NULL) {
    const char* nodename = msg.getNodeName();
    if (nodename != NULL) {
      node = getNodeByName(nodename);
      if (node == NULL) {
        Belle2::debug("[DEBUG] Unexcepted node id: %d", id);
        return NULL;
      }
    }
  }
  return node;
}

NSMNode* RCMaster::getNodeByID(int id) throw()
{
  return (_node_id_m.find(id) != _node_id_m.end()) ? _node_id_m[id] : NULL;
}

NSMNode* RCMaster::getNodeByName(const std::string& name) throw()
{
  return (_node_name_m.find(name) != _node_name_m.end()) ? _node_name_m[name] : NULL;
}

void RCMaster::addNode(int id, NSMNode* node) throw()
{
  if (id >= 0 && node != NULL) {
    node->setNodeID(id);
    _node_id_m.insert(std::map<int, NSMNode*>::value_type(id, node));
    if (_node_name_m.find(node->getName()) == _node_name_m.end())
      _node_name_m.insert(std::map<std::string, NSMNode*>::value_type(node->getName(), node));
  }
}

void RCMaster::addNode(NSMNode* node) throw()
{
  if (node != NULL) {
    _node_v.push_back(node);
    if (_node_name_m.find(node->getName()) == _node_name_m.end())
      _node_name_m.insert(std::map<std::string, NSMNode*>::value_type(node->getName(), node));
  }
}

bool RCMaster::isSynchronized(NSMNode* node_in)
{
  if (node_in == NULL) return false;
  for (RCMaster::NSMNodeList::iterator it = getNSMNodes().begin();
       it != getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    if (node == node_in) break;
    if (!node->isUsed()) continue;
    if (node->getState() != node_in->getState()) return false;
  }
  return true;
}

void RCMaster::setData(DataObject* data)
{
  _data = data;
  for (DataObject::ParamNameList::iterator it = data->getParamNames().begin();
       it != data->getParamNames().end(); it++) {
    DataObject::ParamInfoMap& param_m(data->getParams());
    std::string name = *it;
    if (param_m[name].type == DataObject::OBJECT) {
      NSMNode* node = new NSMNode(name, data->getObject(name));
      addNode(node);
      _status->add(node);
    } else if (param_m[name].type == DataObject::INT) {
      NSMNode* node = new NSMNode(name, new DataObject());
      addNode(node);
      _status->add(node);
      _config->addInt(name, data->getInt(name));
    }
  }
}

void RCMaster::setNodeControl(XMLElement* el)
{
  std::vector<XMLElement*> elc_v = el->getElements();
  for (size_t i = 0; i < elc_v.size(); i++) {
    XMLElement* elc = elc_v[i];
    for (RCMaster::NSMNodeList::iterator it = getNSMNodes().begin();
         it != getNSMNodes().end(); it++) {
      NSMNode* node = *it;
      if (elc->getAttribute("name") == node->getName()) {
        node->setUsed(!(Belle2::tolower(elc->getAttribute("used")) == "false"));
        node->setSynchronized(!(Belle2::tolower(elc->getAttribute("synchronized")) == "false"));
        break;
      }
    }
  }
}
