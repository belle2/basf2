#include "HVControlCallback.h"

#include "HVCommand.h"
#include "HVState.h"

#include "nsm/NSMCommunicator.h"

#include "base/Debugger.h"

using namespace Belle2;

HVControlCallback::HVControlCallback(NSMNode* node) throw()
  : HVCallback(node)
{
}

bool HVControlCallback::ok() throw()
{
  //  Belle2::debug("%s>>OK", getMessage().getNodeName());
  return true;
}

bool HVControlCallback::error() throw()
{
  //  Belle2::debug("%s>>ERROR", getMessage().getNodeName());
  return true;
}

bool HVControlCallback::load() throw()
{
  return true;
}

bool HVControlCallback::switchOn() throw()
{
  return true;
}

bool HVControlCallback::switchOff() throw()
{
  return true;
}
