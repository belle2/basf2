#include "TTDCallback.h"

#include <system/Fork.h>

#include <nsm/RunStatus.h>
#include <nsm/TTDData.h>

#include <base/TTDNode.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>

using namespace Belle2;

TTDCallback::TTDCallback(TTDNode* node)
  : RCCallback(node), _node(node)
{
}

TTDCallback::~TTDCallback() throw()
{

}

bool TTDCallback::boot() throw()
{
  std::vector<FTSW*>& ftsw_v(_node->getFTSWs());
  if (_ftswcon_v.size() != ftsw_v.size()) {
    _ftswcon_v.resize(0);
    for (size_t i = 0; i < ftsw_v.size(); i++) {
      _ftswcon_v.push_back(FTSWController(ftsw_v[i]));
    }
  }
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (_ftswcon_v[slot].boot()) {
      Belle2::debug("Failed to boot FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::load() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].load()) {
      Belle2::debug("Failed to load FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::start() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].start()) {
      Belle2::debug("Failed to start FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::stop() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].stop()) {
      Belle2::debug("Failed to stop FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::resume() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].resume()) {
      Belle2::debug("Failed to resume FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::pause() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].pause()) {
      Belle2::debug("Failed to pause FTSW:%d", slot);
      return false;
    }
  }
  return true;
}

bool TTDCallback::abort() throw()
{
  for (size_t slot = 0; slot < _ftswcon_v.size(); slot++) {
    if (!_ftswcon_v[slot].abort()) {
      Belle2::debug("Failed to abort FTSW:%d", slot);
      return false;
    }
  }
  return true;
}
