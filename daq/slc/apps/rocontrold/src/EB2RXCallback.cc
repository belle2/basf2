#include "daq/slc/apps/rocontrold/EB2RXCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <sstream>

#include <unistd.h>

using namespace Belle2;

EB2RXCallback::EB2RXCallback(const NSMNode& node, const std::string& conf)
  : RCCallback(node), m_file(conf)
{
  //  system("killall basf2");
}

EB2RXCallback::~EB2RXCallback() throw()
{
}

void EB2RXCallback::init() throw()
{
  setAutoReply(false);
  m_data = NSMData(getNode().getName() + "_STATUS", "ronode_status",
                   ronode_status_revision);
  m_data.allocate(getCommunicator());
  m_con.setCallback(this);
  m_con.init("eb2rx", 1);
}

void EB2RXCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool EB2RXCallback::load() throw()
{
  if (m_con.isAlive()) return true;
  m_con.clearArguments();
  m_con.setExecutable("/home/usr/b2daq/eb/eb2rx");
  m_con.addArgument("-l");
  m_con.addArgument("5141");
  m_con.addArgument("192.168.60.12:4000");
  m_con.addArgument("192.168.100.18:24");
  m_con.load(0);

  return true;
}

bool EB2RXCallback::start() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = Time().getSecond();
  const bool use_recv0 = m_file.getBool("ropc.nrecv0") > 0;
  if (use_recv0) {
    m_con.start();
  } else {
    m_con.start();
  }
  return true;
}

bool EB2RXCallback::stop() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = 0;
  getNode().setState(RCState::READY_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB2RXCallback::resume() throw()
{
  getNode().setState(RCState::RUNNING_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB2RXCallback::pause() throw()
{
  getNode().setState(RCState::PAUSED_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB2RXCallback::recover() throw()
{
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    getCommunicator()->replyOK(getNode());
    return true;
  }
  return false;
}

bool EB2RXCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::NOTREADY_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

void EB2RXCallback::timeout() throw()
{
  if (getNode().getState() == RCState::LOADING_TS) {
    getNode().setState(RCState::READY_S);
    getCommunicator()->replyOK(getNode());
  }
}

