#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"

#include <daq/slc/apps/runcontrold/RunSetting.h>
#include <daq/slc/apps/runcontrold/RunSummary.h>
#include <daq/slc/apps/runcontrold/rc_status.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/LoggerObjectTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

RunControlMasterCallback::RunControlMasterCallback(const NSMNode& node,
                                                   RunControlCallback* callback)
  : RCCallback(node)
{
  NSMData& data(callback->getData());
  m_data = NSMData(data.getName(), data.getFormat(), data.getRevision());
  m_callback = callback;
  callback->setCallback(this);
}

void RunControlMasterCallback::init() throw()
{
  m_data.allocate(getCommunicator());
  update();
}

void RunControlMasterCallback::timeout() throw()
{
  update();
}

void RunControlMasterCallback::update() throw()
{
  if (m_data.isAvailable() && m_callback->getData().isAvailable() &&
      m_data.getEntry() == NULL) {
    rc_status* status_g = (rc_status*)m_data.get();
    const rc_status* status_l = (const rc_status*)(m_callback->getData().get());
    memcpy(status_g, status_l, sizeof(rc_status));
  }
}

bool RunControlMasterCallback::perform(const NSMMessage& msg) throw()
{
  msg.getNodeName();
  RCCommand cmd(msg.getRequestName());
  LogFile::debug("%s >> %s (cmd = %s)", msg.getNodeName(),
                 msg.getRequestName(), cmd.getLabel());
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}
