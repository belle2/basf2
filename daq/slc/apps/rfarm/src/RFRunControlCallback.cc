#include "daq/slc/apps/rfarm/RFRunControlCallback.h"

#include "daq/slc/apps/rfarm/RFMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(RFMasterCallback* callback)
  : m_callback(callback)
{
  callback->setCallback(this);
  allocData(getNode().getName() + "_STATUS", "rfunitinfo", 3);
}

void RFRunControlCallback::initialize(const DBObject&) throw(RCHandlerException)
{
  allocData(getNode().getName() + "_STATUS", "rfunitinfo", 3);
}

bool RFRunControlCallback::perform(NSMCommunicator& com) throw()
{
  NSMCommand cmd(com.getMessage().getRequestName());
  m_callback->perform(com);
  if (cmd == NSMCommand::VGET || cmd == NSMCommand::VSET) {
    return RCCallback::perform(com);
  }
  return true;
}

