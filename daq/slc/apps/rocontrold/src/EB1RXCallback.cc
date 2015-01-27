#include "daq/slc/apps/rocontrold/EB1RXCallback.h"

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

EB1RXCallback::EB1RXCallback(const NSMNode& node, const std::string& conf)
  : RCCallback(node), m_file(conf)
{
  //  system("killall basf2");
}

EB1RXCallback::~EB1RXCallback() throw()
{
}

void EB1RXCallback::init() throw()
{
  setAutoReply(false);
  m_data = NSMData(getNode().getName() + "_STATUS", "ronode_status",
                   ronode_status_revision);
  m_data.allocate(getCommunicator());
  m_con.setCallback(this);
  m_con.init("recvStream1", 1);
}

void EB1RXCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool EB1RXCallback::load() throw()
{
  if (m_con.isAlive()) return true;
  m_con.clearArguments();
  m_con.setExecutable("/home/usr/b2daq/eb/eb0");
  m_con.addArgument("-l");
  m_con.addArgument("5101");
  for (int i = 1; i <= 16; i++)
    m_con.addArgument(StringUtil::form("localhost:%d", 34000 + i));
  m_con.load(0);

  return true;
}

bool EB1RXCallback::start() throw()
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

bool EB1RXCallback::stop() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = 0;
  getNode().setState(RCState::READY_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB1RXCallback::resume() throw()
{
  getNode().setState(RCState::RUNNING_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB1RXCallback::pause() throw()
{
  getNode().setState(RCState::PAUSED_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

bool EB1RXCallback::recover() throw()
{
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    getCommunicator()->replyOK(getNode());
    return true;
  }
  return false;
}

bool EB1RXCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::NOTREADY_S);
  getCommunicator()->replyOK(getNode());
  return true;
}

void EB1RXCallback::timeout() throw()
{
  if (getNode().getState() == RCState::LOADING_TS) {
    getNode().setState(RCState::READY_S);
    getCommunicator()->replyOK(getNode());
  }
}

