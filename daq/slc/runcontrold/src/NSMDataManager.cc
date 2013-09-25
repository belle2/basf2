#include "NSMDataManager.hh"

#include <nsm/TTDData.hh>
#include <nsm/DataReceiverData.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <iostream>
extern "C" {
#include <dlfcn.h>
}

using namespace B2DAQ;

NSMDataManager::NSMDataManager(NodeLoader* node_loader) throw()
  : _loader(node_loader), _status(NULL), _config(NULL)
{

}

NSMDataManager::~NSMDataManager() throw()
{
  for (size_t i = 0; i < _copper_data_v.size(); i++) {
    delete _copper_data_v[i];
  }
  for (size_t i = 0; i < _ttd_data_v.size(); i++) {
    delete _ttd_data_v[i];
  }
  for (size_t i = 0; i < _recv_data_v.size(); i++) {
    delete _recv_data_v[i];
  }
  for (size_t i = 0; i < _handler_v.size(); i++) {
    dlclose(_handler_v[i]);
  }
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
    _status->write(NULL);
  } catch (const NSMHandlerException& e) {}
  return false;
}

RunConfig* NSMDataManager::allocateRunConfig() throw()
{
  if (_config == NULL) {
    _config = new RunConfig("NODE_CONFIG");
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
    _config->write(NULL);
  } catch (const NSMHandlerException& e) {}
  return false;
}

bool NSMDataManager::allocateData(const std::string& path) throw()
{
  std::map<std::string, COPPERNode*>& copper_m(_loader->getCOPPERNodeList());
  for (std::map<std::string, COPPERNode*>::iterator it = copper_m.begin();
       it != copper_m.end(); it++) {
    std::string module_class = it->first;
    void* handler = dlopen(B2DAQ::form("%s/lib/libB2SLC_%s.so",
                                       path.c_str(), module_class.c_str()).c_str(),
                           RTLD_NOW | RTLD_GLOBAL);
    if (!handler) {
      B2DAQ::debug("dlopen error: %s", dlerror());
      return false;
    }
    _handler_v.push_back(handler);
    char* error = NULL;
    createCOPPERData_t* func =
      (createCOPPERData_t*)dlsym(handler, B2DAQ::form("create%sData", module_class.c_str()).c_str());
    if ((error = dlerror()) != NULL) {
      B2DAQ::debug("dlsym error: %s", error);
      return false;
    }
    _func_m.insert(std::map<std::string, createCOPPERData_t*>::value_type(module_class, func));
  }
  NodeSystem& node_system(_loader->getSystem());
  try {
    std::vector<COPPERNode*>& node_v(node_system.getCOPPERNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      std::string module_class = node_v[i]->getModuleClass();
      NSMData* data = (NSMData*)_func_m[module_class]((void*)node_v[i],
                                                      (node_v[i]->getName() + "_DATA").c_str());
      data->allocate();
      _copper_data_v.push_back(data);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to allocate NSM data for copperd");
    return false;
  }
  try {
    std::vector<TTDNode*>& node_v(node_system.getTTDNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      NSMData* data = new TTDData(node_v[i]->getName() + "_DATA");
      data->allocate();
      _ttd_data_v.push_back(data);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to allocate NSM data for ttdd");
    return false;
  }
  try {
    std::vector<DataReceiverNode*>& node_v(node_system.getReceiverNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      NSMData* data = new DataReceiverData(node_v[i]->getName() + "_DATA");
      data->allocate();
      _recv_data_v.push_back(data);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to allocate NSM data for recvd");
    return false;
  }

  return true;
}

bool NSMDataManager::writeData() throw()
{
  NodeSystem& node_system(_loader->getSystem());
  try {
    std::vector<COPPERNode*>& node_v(node_system.getCOPPERNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      if (node_v[i] != NULL)
        _copper_data_v[i]->write(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to write NSM data for copperd");
    return false;
  }
  try {
    std::vector<TTDNode*>& node_v(node_system.getTTDNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      if (node_v[i] != NULL)
        _ttd_data_v[i]->write(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to write NSM data for ttdd");
    return false;
  }
  try {
    std::vector<DataReceiverNode*>& node_v(node_system.getReceiverNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      if (node_v[i] != NULL)
        _recv_data_v[i]->write(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to write NSM data for recvd");
    return false;
  }
  return true;
}

bool NSMDataManager::readData() throw()
{
  NodeSystem& node_system(_loader->getSystem());
  try {
    std::vector<COPPERNode*>& node_v(node_system.getCOPPERNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      _copper_data_v[i]->read(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to read NSM data for copperd");
    return false;
  }
  try {
    std::vector<TTDNode*>& node_v(node_system.getTTDNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      _ttd_data_v[i]->read(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to read NSM data for ttdd");
    return false;
  }
  try {
    std::vector<DataReceiverNode*>& node_v(node_system.getReceiverNodes());
    for (size_t i = 0; i < node_v.size(); i++) {
      _recv_data_v[i]->read(node_v[i]);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to read NSM data for recvd");
    return false;
  }
  return true;
}
