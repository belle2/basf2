#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(const NSMNode& node,
                                           RFMaster* master,
                                           RFMasterCallback* callback)
  : RCCallback(node), m_master(master), m_callback(callback)
{
  callback->setCallback(this);
}

RFRunControlCallback::~RFRunControlCallback() throw()
{

}

void RFRunControlCallback::init() throw()
{
  m_data = NSMData(getNode().getName() + "_STATUS", "rfunitinfo", 1);
  m_data.allocate(getCommunicator());
}

bool RFRunControlCallback::load() throw()
{
  m_callback->setMessage(getMessage());
  return m_callback->configure();
}

bool RFRunControlCallback::start() throw()
{
  m_callback->setMessage(getMessage());
  return m_callback->start();
}

bool RFRunControlCallback::stop() throw()
{
  m_callback->setMessage(getMessage());
  return m_callback->stop();
}

bool RFRunControlCallback::recover() throw()
{
  m_callback->setMessage(getMessage());
  return m_callback->unconfigure() && m_callback->configure();
}

bool RFRunControlCallback::abort() throw()
{
  m_callback->setMessage(getMessage());
  return m_callback->unconfigure();
}

