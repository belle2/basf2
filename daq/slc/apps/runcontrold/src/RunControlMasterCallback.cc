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
  m_data = NSMData(node.getName() + "_STATUS",
                   "rc_status", rc_status_revision);
  m_callback = callback;
  callback->addCallback(this);
}

void RunControlMasterCallback::init() throw()
{
  m_data.allocate(getCommunicator());
  update();
}

void RunControlMasterCallback::timeout() throw()
{
}

void RunControlMasterCallback::update() throw()
{
  if (m_data.isAvailable() && m_callback->getData().isAvailable()) {
    rc_status* status_g = (rc_status*)m_data.get();
    const rc_status* status_l = (const rc_status*)(m_callback->getData().get());
    memcpy(status_g, status_l, sizeof(rc_status));
  }
}

bool RunControlMasterCallback::perform(const NSMMessage& msg) throw()
{
  msg.getNodeName();
  m_callback->getCommunicator()->setMessage(msg);
  RCCommand cmd(msg.getRequestName());
  //LogFile::debug("%s >> %s", msg.getNodeName(), cmd.getLabel());
  if (cmd == RCCommand::STATECHECK) {
    getNode().setState(m_callback->getNode().getState());
    getCommunicator()->replyOK(getNode());
    return true;
  }
  return m_callback->perform(msg);
}
