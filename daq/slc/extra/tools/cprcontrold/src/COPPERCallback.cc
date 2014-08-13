#include "COPPERCallback.h"

#include <daq/slc/readout/ronode_status.h>

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
  : RCCallback(node), m_fee(fee)
{
  m_con.setCallback(this);
}

COPPERCallback::~COPPERCallback() throw()
{
}

void COPPERCallback::init() throw()
{
  m_con.init("basf2_" + getNode().getName(), 1);
  m_ttrx.open();
  m_copper.open();
  m_flow.open(&m_con.getInfo());
  m_data = NSMData(getNode().getName() + "_STATUS", "ronode_status",
                   ronode_status_revision);
  m_data.allocate(getCommunicator());
}

void COPPERCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
  for (int i = 0; i < 4; i++) {
    m_hslb[i].close();
  }
  m_copper.close();
  m_ttrx.close();
}

void COPPERCallback::timeout() throw()
{
  m_ttrx.monitor();
  m_copper.monitor();
  int eflag = 0;
  eflag |= m_copper.isFifoFull() << 5;
  eflag |= (getNode().getState() == RCState::RUNNING_S && m_copper.isFifoEmpty()) << 6;
  eflag |= m_copper.isLengthFifoFull() << 7;
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) {
      m_hslb[i].monitor();
      eflag |= m_hslb[i].isBelle2LinkDown() << (8 + i);
      eflag |= m_hslb[i].isCOPPERFifoFull() << (12 + i);
      eflag |= m_hslb[i].isCOPPERLengthFifoFull() << (16 + i);
      eflag |= m_hslb[i].isFifoFull() << (20 + i);
      eflag |= m_hslb[i].isCRCError() << (24 + i);
    }
  }
  eflag |= m_ttrx.isBelle2LinkError() << 28;
  eflag |= m_ttrx.isLinkUpError() << 29;

  if (m_copper.isFifoFull()) {
    LogFile::warning("FIFO full");
  }
  if (m_copper.isFifoEmpty()) {
    LogFile::warning("FIFO empty");
  }
  if (m_copper.isLengthFifoFull()) {
    LogFile::warning("Length FIFO full");
  }
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) {
      if (m_hslb[i].isBelle2LinkDown()) {
        LogFile::error("Belle2 link down");
      }
      if (m_hslb[i].isCOPPERFifoFull()) {
        LogFile::warning("COPPER fifo full");
      }
      if (m_hslb[i].isCOPPERLengthFifoFull()) {
        LogFile::warning("COPPER length fifo full");
      }
      if (m_hslb[i].isFifoFull()) {
        LogFile::warning("HSLB %d fifo full", i);
      }
      if (m_hslb[i].isCRCError()) {
        LogFile::warning("HSLB %d CRC error", i);
      }
    }
  }

  if (m_ttrx.isBelle2LinkError()) {
    LogFile::error("Belle2 link error\n");
  }
  if (m_ttrx.isLinkUpError()) {
    LogFile::error("Link up error\n");
  }

  if (m_data.isAvailable() && m_flow.isAvailable()) {
    ronode_status& status(m_flow.monitor());
    status.eflag |= (eflag & 0xFF);
    if (getNode().getState() != RCState::RECOVERING_RS && eflag > 0) {
      getNode().setState(RCState::RECOVERING_RS);
      sendPause();
    }
    memcpy(m_data.get(), &status, sizeof(ronode_status));
  }
}

bool COPPERCallback::load() throw()
{
  m_config.read(getConfig().getObject());
  if (!m_ttrx.isOpened()) {
    m_ttrx.open();
    //m_ttrx.boot(m_config.getSetup().getTTRXFirmware());
  }
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) {
      m_hslb[i].open(i);
      m_hslb[i].load();
    }
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
    //m_ttrx.boot(m_config.getSetup().getTTRXFirmware());
  }
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i) && m_hslb[i].isError()) {
      //m_hslb[i].boot(m_config.getSetup().getHSLBFirmware());
      m_hslb[i].load();
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
  //getNode().setState(RCState::NOTREADY_S);
  return true;
}

bool COPPERCallback::bootBasf2() throw()
{
  //if (m_con.isAlive()) {
  system("killall basf2");
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
  m_con.addArgument("basf2_" + getNode().getName());
  if (m_con.load(30)) {
    LogFile::debug("load succeded");
    return true;
  }
  LogFile::debug("load timeout");
  return false;

}
