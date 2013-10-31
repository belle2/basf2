#include "NSMDataManager.h"

#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <nsm/TTDStatus.h>

#include <iostream>
extern "C" {
#include <dlfcn.h>
}

using namespace Belle2;

NSMDataManager::NSMDataManager(NodeSystem* node_system) throw()
  : _system(node_system), _status(NULL), _config(NULL)
{
  std::vector<NSMNode*>& node_v(_system->getNodes());
  for (size_t i = 0; i < node_v.size(); i++) {
    NSMNode* node = node_v[i];
    if (node->getType() == "ttd_node") {
      NSMData* data = new TTDStatus(node_v[i]->getName());
      _node_status_v.push_back(data);
    } else {
      _node_status_v.push_back(NULL);
    }
  }
}

NSMDataManager::~NSMDataManager() throw()
{
}

RunStatus* NSMDataManager::allocateRunStatus() throw()
{
  if (_status == NULL) {
    _status = new RunStatus("RUN_STATUS");
    try {
      _status->allocate();
    } catch (const NSMHandlerException& e) {
      delete _status;
      _status = NULL;
    }
  }
  return _status;
}

bool NSMDataManager::writeRunStatus() throw()
{
  if (_status == NULL) return false;
  try {
    _status->write();
  } catch (const NSMHandlerException& e) {}
  return false;
}

RunConfig* NSMDataManager::allocateRunConfig() throw()
{
  if (_config == NULL) {
    _config = new RunConfig("RUN_CONFIG");
    try {
      _config->allocate();
    } catch (const NSMHandlerException& e) {
      delete _config;
      _config = NULL;
    }
  }
  return _config;
}

bool NSMDataManager::writeRunConfig() throw()
{
  if (_config == NULL) return false;
  try {
    _config->write();
  } catch (const NSMHandlerException& e) {}
  return false;
}

void NSMDataManager::readNodeStatus() throw()
{
  std::vector<NSMNode*>& node_v(_system->getNodes());
  for (size_t i = 0; i < node_v.size(); i++) {
    NSMNode* node = node_v[i];
    NSMData* data = _node_status_v[i];
    if (data != NULL) {
      try {
        if (!data->isAvailable()) data->open();
        /*  */
      } catch (const NSMHandlerException& e) {
        Belle2::debug("Failed to read NSM data in %s", node->getName().c_str());
      }
    }
  }
}
