#include "daq/slc/apps/hvcontrold/HVControlCallback.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/Debugger.h"

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
