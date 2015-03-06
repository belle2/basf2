#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMasterCallback.h"

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(RFMasterCallback* callback)
  : m_callback(callback)
{
  callback->setCallback(this);
  //setData(getNode().getName() + "_STATUS", "rfunitinfo", 3);
}

bool RFRunControlCallback::perform(NSMCommunicator& com) throw()
{
  return m_callback->perform(com);
}

