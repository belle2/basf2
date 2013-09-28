#include "NSMDataManager.hh"

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

