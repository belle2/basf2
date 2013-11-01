#include "VMEControllerCallback.h"

#include <nsm/RunStatus.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

VMEControllerCallback::VMEControllerCallback(NSMNode* node)
  : RCCallback(node)
{
  add(Command::TRIGFT);
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
  Belle2::debug("Exp no: %d", (int)getMessage().getParam(0));
  Belle2::debug("Run no: %d", (int)getMessage().getParam(1));
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

