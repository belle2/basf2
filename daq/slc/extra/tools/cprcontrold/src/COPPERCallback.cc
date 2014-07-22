#include "COPPERCallback.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

COPPERCallback::COPPERCallback(const NSMNode& node,
                               FEEController* fee)
  : RCCallback(node)
{
  m_con.setCallback(this);
  m_fee = fee;
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  m_con.init(getNode().getName());
  m_copper.open();
  m_flow.open(&m_con.getInfo());
  m_data = NSMData(getNode().getName() + "_STATUS", "ronode_status", 1);
  m_data.allocate(getCommunicator());
}

void COPPERCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  m_fee->close();
  m_copper.close();
  m_ttrx.close();
}

void COPPERCallback::timeout() throw()
{
  m_ttrx.monitor();
  m_copper.monitor();
  ronode_status& status(m_flow.monitor());
  int eflag = (status.eflag & 0xFF);
  eflag |= m_copper.isFifoFull() << 5;
  eflag |= m_copper.isFifoEmpty() << 6;
  eflag |= m_copper.isLengthFifoFull() << 7;
  for (int i = 0; i < 4; i++) {
    m_fee->monitor(i, m_config.getFEE(i));
    eflag |= m_fee->isBelle2LinkDown(i) << (8 + i);
    eflag |= m_fee->isCOPPERFifoFull(i) << (12 + i);
    eflag |= m_fee->isCOPPERLengthFifoFull(i) << (16 + i);
    eflag |= m_fee->isHSLBFifoFull(i) << (20 + i);
    eflag |= m_fee->isHSLBCRCError(i) << (24 + i);
  }
  eflag |= m_ttrx.isBelle2LinkError() << 28;
  eflag |= m_ttrx.isLinkUpError() << 29;
  status.eflag = eflag;
  if (getNode().getState() != RCState::RECOVERING_RS && eflag > 0) {
    getNode().setState(RCState::RECOVERING_RS);
    sendPause();
  }
  memcpy(m_data.get(), &status, sizeof(ronode_status));
}

bool COPPERCallback::load() throw()
{
  m_config.read(getConfig().getObject());
  if (!m_ttrx.isOpened()) {
    m_ttrx.open();
    m_ttrx.boot(m_config.getSetup().getTTRXFirmware());
    m_ttrx.monitor();
  }
  return recover();
}

bool COPPERCallback::start() throw()
{
  return m_con.start();
}

bool COPPERCallback::stop() throw()
{
  return m_con.stop();
}

bool COPPERCallback::resume() throw()
{
  return true;
}

bool COPPERCallback::pause() throw()
{
  return true;
}

bool COPPERCallback::recover() throw()
{
  if (m_ttrx.isError()) {
    m_ttrx.boot(m_config.getSetup().getTTRXFirmware());
  }
  for (size_t i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) {
      if (m_fee->open(i, m_config.getFEE(i))) {
        m_fee->load(i, m_config.getFEE(i));
      }
    }
  }
  if (bootBasf2()) {
    getNode().setState(RCState::RUNNING_S);
    return true;
  }
  return false;

}

bool COPPERCallback::abort() throw()
{
  m_con.abort();
  m_fee->close();
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

bool COPPERCallback::bootBasf2() throw()
{
  if (!m_con.isAlive()) {
    int flag = 0;
    for (size_t i = 0; i < 4; i++) {
      if (m_config.useHSLB(i)) flag += 1 << i;
    }
    m_con.clearArguments();
    m_con.addArgument(m_config.getBasf2Script());
    m_con.addArgument(m_config.getHostname());
    m_con.addArgument(m_config.getCopperId().substr(3));
    m_con.addArgument(StringUtil::form("%d", flag));
    m_con.addArgument("1");
    m_con.addArgument("basf2");
    if (m_con.load(30)) {
      LogFile::debug("load succeded");
      return true;
    }
    LogFile::debug("load timeout");
    return false;
  }
  return true;
}
