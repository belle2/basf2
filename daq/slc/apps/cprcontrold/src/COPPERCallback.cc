#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include "daq/slc/apps/cprcontrold/COPPERHandler.h"

#include <daq/slc/readout/ronode_status.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <cstring>

using namespace Belle2;

COPPERCallback::COPPERCallback(FEEController* fee[4])
{
  setTimeout(5);
  m_con.setCallback(this);
  for (int i = 0; i < 4; i++) {
    m_fee[i] = fee[i];
  }
  system("killall -9 basf2");
  m_force_boothslb = true;
}

COPPERCallback::~COPPERCallback() throw()
{
}

bool COPPERCallback::initialize(const DBObject& obj) throw()
{
  allocData(getNode().getName() + "_STATUS", "ronode_status",
            ronode_status_revision);
  m_con.init("cprbasf2_" + getNode().getName(), 1);
  m_ttrx.open();
  m_copper.open();
  m_flow.open(&m_con.getInfo());
  return configure(obj);
}

bool COPPERCallback::configure(const DBObject& /*obj*/) throw()
{
  try {
    add(new NSMVHandlerFifoEmpty(*this, "copper.err.fifoempty"));
    add(new NSMVHandlerFifoFull(*this, "copper.err.fifofull"));
    add(new NSMVHandlerLengthFifoFull(*this, "copper.err.lengthfifofull"));
    add(new NSMVHandlerTTRXBelle2LinkError(*this, "ttrx.err.b2linkerr"));
    add(new NSMVHandlerDownloadTTRXFirmware(*this, "ttrx.download.firmware"));
    for (int i = 0; i < 4; i++) {
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBBelle2LinkDown(*this, vname + ".err.b2linkdown", i));
      add(new NSMVHandlerHSLBCOPPERFifoFull(*this, vname + ".err.cprfifofull", i));
      add(new NSMVHandlerHSLBCOPPERLengthFifoFull(*this, vname + ".err.cprlengthfifofull", i));
      add(new NSMVHandlerHSLBFifoFull(*this, vname + ".err.fifofull", i));
      add(new NSMVHandlerHSLBCRCError(*this, vname + ".err.crc", i));
      add(new NSMVHandlerDownloadHSLBFirmware(*this, vname + ".download.firmware", i));
      vname = StringUtil::form("hslb[%d].reg", i);
      add(new NSMVHandlerInt(vname + ".adr", true, true, 0));
      add(new NSMVHandlerHSLBRegValue(*this, vname + ".val", i));
      if (!m_config.useHSLB(i)) continue;
      FEEConfig::ParameterList& pars(m_config.getFEE(i).getParameters());
      for (FEEConfig::ParameterList::iterator it = pars.begin();
           it != pars.end(); it++) {
        FEEConfig::Parameter& par(*it);
        FEEConfig::Register& reg(*m_config.getFEE(i).getRegister(par.getName()));
        int adr = reg.getAddress();
        std::string pname = StringUtil::tolower(par.getName());
        std::string vname = StringUtil::form("hslb[%d].reg.%s", i, pname.c_str());
        add(new NSMVHandlerInt(vname + ".adr", true, false, adr));
        add(new NSMVHandlerHSLBRegValue(*this, vname + ".val", i, adr));
      }
    }
    return true;
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return false;
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

void COPPERCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  m_config.read(obj);
  m_ttrx.open();
  if (m_ttrx.isError()) {
    m_ttrx.close();
    throw (RCHandlerException("TTRX Link error"));
  }
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) {
      HSLBController& hslb(m_hslb[i]);
      hslb.open(i);
      if (!hslb.load()) {
        throw (RCHandlerException("Failed to load HSLB:%c", i + 'a'));
      }
      if (m_fee[i] != NULL) {
        FEEController& fee(*m_fee[i]);
        try {
          fee.load(hslb, m_config.getFEE(i));
        } catch (const IOException& e) {
          throw (RCHandlerException(e.what()));
        }
      }
    }
  }
  bootBasf2();
}

void COPPERCallback::start(int expno, int runno) throw(RCHandlerException)
{
  ronode_status* status = (ronode_status*)getData().get();
  status->stime = Time().getSecond();
  if (!m_con.start(expno, runno)) {
    throw (RCHandlerException("Failed to start"));
  }
}

void COPPERCallback::stop() throw(RCHandlerException)
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = 0;
  m_con.stop();
}

void COPPERCallback::recover() throw(RCHandlerException)
{
  abort();
}

void COPPERCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
  setState(RCState::NOTREADY_S);
}

void COPPERCallback::logging(bool err, LogFile::Priority pri,
                             const char* str, ...) throw()
{
  if (err) {
    va_list ap;
    static char ss[1024];
    va_start(ap, str);
    vsprintf(ss, str, ap);
    va_end(ap);
    reply(NSMMessage(DAQLogMessage(getNode().getName(), pri, ss)));
    if (pri >= LogFile::ERROR)
      setState(RCState::NOTREADY_S);
  }
}

void COPPERCallback::timeout(NSMCommunicator&) throw()
{
  RCState state = getNode().getState();
  if (!m_dummymode) {
    m_ttrx.monitor();
    m_copper.monitor();
    if (state != RCState::NOTREADY_S && state.isStable()) {
      logging(m_copper.isFifoEmpty(), LogFile::NOTICE, "COPPER FIFO empty");
      logging(m_copper.isFifoFull(), LogFile::WARNING, "COPPER FIFO full");
      logging(m_copper.isLengthFifoFull(), LogFile::WARNING, "COPPER length FIFO full");
      for (int i = 0; i < 4; i++) {
        if (m_config.useHSLB(i)) {
          HSLBController hslb(m_hslb[i]);
          hslb.monitor();
          logging(hslb.isBelle2LinkDown(), LogFile::ERROR,
                  "HSLB %c Belle2 link down", (char)(i + 'a'));
          logging(hslb.isCOPPERFifoFull(), LogFile::WARNING,
                  "HSLB %c COPPER fifo full", (char)(i + 'a'));
          logging(hslb.isCOPPERLengthFifoFull(), LogFile::WARNING,
                  "HSLB %c COPPER length fifo full", (char)(i + 'a'));
          logging(hslb.isFifoFull(), LogFile::WARNING, "HSLB %c fifo full", (char)(i + 'a'));
          logging(hslb.isCRCError(), LogFile::WARNING, "HSLB %c CRC error", (char)(i + 'a'));
        }
      }
      logging(m_ttrx.isBelle2LinkError(), LogFile::ERROR, "TTRX Belle2 link error");
      logging(m_ttrx.isLinkUpError(), LogFile::ERROR, "TTRX Link up error");
    }
  }
  NSMData& data(getData());
  if (data.isAvailable()) {
    ronode_status* nsm = (ronode_status*)data.get();
    if (m_flow.isAvailable()) {
      ronode_status& status(m_flow.monitor());
      memcpy(nsm, &status, sizeof(ronode_status));
    }
    double loads[3];
    if (getloadavg(loads, 3) > 0) {
      nsm->loadavg = (float)loads[0];
    } else {
      nsm->loadavg = -1;
    }
    data.flush();
  }
}

void COPPERCallback::bootBasf2() throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  int flag = 0;
  for (size_t i = 0; i < 4; i++) {
    if (m_config.useHSLB(i)) flag += 1 << i;
  }
  m_con.clearArguments();
  m_con.setExecutable("basf2");
  m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                     m_config.getBasf2Script().c_str()));
  m_con.addArgument(m_config.getHostname());
  int copperid = atoi(m_config.getCopperId().substr(4).c_str());
  int detectorid = atoi(m_config.getCopperId().substr(3).c_str()) / 1000;
  char str[64];
  sprintf(str, "0x%d000000", detectorid);
  copperid += strtol(str, NULL, 0);
  m_con.addArgument(StringUtil::form("%d", copperid));
  m_con.addArgument(StringUtil::form("%d", flag));
  m_con.addArgument("1");
  m_con.addArgument("cprbasf2_" + getNode().getName());
  if (!m_con.load(10)) {
    LogFile::debug("load timeout");
    throw (RCHandlerException("Failed to start readout : " + m_config.getBasf2Script()));
  }
  LogFile::debug("load succeded");
}

bool COPPERCallback::isError() throw()
{
  if (m_copper.isError()) return true;
  for (int i = 0; i < 4; i++) {
    if (m_config.useHSLB(i) && m_hslb[i].isError())
      return true;
  }
  if (m_ttrx.isBelle2LinkError())
    return true;
  return false;
}
