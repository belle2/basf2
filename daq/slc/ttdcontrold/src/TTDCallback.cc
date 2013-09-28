#include "TTDCallback.hh"

#include <system/Fork.hh>

#include <nsm/RunStatus.hh>
#include <nsm/TTDData.hh>

#include <node/TTDNode.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

using namespace B2DAQ;

TTDCallback::TTDCallback(TTDNode* node, TTDData* data)
  : RCCallback(node), _node(node), _data(data)
{
  _status = new RunStatus("RUN_STATUS");
}

TTDCallback::~TTDCallback() throw()
{

}

bool TTDCallback::boot() throw()
{
  while (!_status->isAvailable()) {
    try {
      _status->open();
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("TTD daemon : Failed to open run status. Waiting for 5 seconds..");
      sleep(5);
    }
  }
  if (_data != NULL) {
    try {
      _data->read(_node);
    } catch (const NSMHandlerException& e) {
      B2DAQ::debug("Failed to access to NSM data, %s", e.what());
      return false;
    }
  }
  std::vector<FTSW*>& ftsw_v(_node->getFTSWs());
  if (_ftswcon_v.size() != ftsw_v.size()) {
    _ftswcon_v.resize(0);
    for (size_t i = 0; i < ftsw_v.size(); i++) {
      _ftswcon_v.push_back(FTSWController(ftsw_v[i]));
    }
  }
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (_ftswcon_v[slot].boot()) {
      B2DAQ::debug("Failed to boot FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::load() throw()
{
  try {
    if (_data != NULL) {
      _data->read(_node);
    }
  } catch (const NSMHandlerException& e) {
    B2DAQ::debug("Failed to access NSM data.");
    return false;
  }
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].load()) {
      B2DAQ::debug("Failed to load FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::start() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].start()) {
      B2DAQ::debug("Failed to start FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::stop() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].stop()) {
      B2DAQ::debug("Failed to stop FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::resume() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].resume()) {
      B2DAQ::debug("Failed to resume FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::pause() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].pause()) {
      B2DAQ::debug("Failed to pause FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::abort() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].abort()) {
      B2DAQ::debug("Failed to abort FTSW:%d", slot);
      return false;
    }
  }
  return true;
}
