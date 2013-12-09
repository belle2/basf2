#include "TTDCallback.h"

#include <daq/slc/system/Fork.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

TTDCallback::TTDCallback(NSMNode* node)
  : RCCallback(node)
{
}

TTDCallback::~TTDCallback() throw()
{

}

bool TTDCallback::boot() throw()
{
  if (_ftswcon.boot()) {
    Belle2::debug("Failed to boot FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::load() throw()
{
  if (!_ftswcon.load()) {
    Belle2::debug("Failed to load FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::start() throw()
{
  if (!_ftswcon.start()) {
    Belle2::debug("Failed to start FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::stop() throw()
{
  if (!_ftswcon.stop()) {
    Belle2::debug("Failed to stop FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::resume() throw()
{
  if (!_ftswcon.resume()) {
    Belle2::debug("Failed to resume FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::pause() throw()
{
  if (!_ftswcon.pause()) {
    Belle2::debug("Failed to pause FTSW:%d", slot);
    return false;
  }
  return true;
}

bool TTDCallback::abort() throw()
{
  if (!_ftswcon.abort()) {
    Belle2::debug("Failed to abort FTSW:%d", slot);
    return false;
  }
  return true;
}
