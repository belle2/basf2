#include "VMEControllerCallback.h"

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

VMEControllerCallback::VMEControllerCallback(NSMNode* node)
  : RCCallback(node)
{
}

VMEControllerCallback::~VMEControllerCallback() throw()
{

}

bool VMEControllerCallback::boot() throw()
{
  Belle2::debug("BOOT");
  return true;
}

bool VMEControllerCallback::load() throw()
{
  Belle2::debug("LOAD");
  return true;
}

bool VMEControllerCallback::start() throw()
{
  Belle2::debug("START");
  Belle2::debug("Exp  number : %d", getExpNumber());
  Belle2::debug("Cold number : %d", getColdNumber());
  Belle2::debug("Hot  number : %d", getHotNumber());
  return true;
}

bool VMEControllerCallback::stop() throw()
{
  Belle2::debug("STOP");
  return true;
}

bool VMEControllerCallback::recover() throw()
{
  Belle2::debug("RECOVER");
  return true;
}

