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

COPPERCallback::COPPERCallback(FEE* fee[4])
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

void COPPERCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName() + "_STATUS", "ronode_status",
            ronode_status_revision);
  m_con.init("cprbasf2_" + getNode().getName(), 1);
  m_ttrx.open();
  m_copper.open();
  m_flow.open(&m_con.getInfo());
  configure(obj);
}

void COPPERCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  try {
    add(new NSMVHandlerFifoEmpty(*this, "copper.err.fifoempty"));
    add(new NSMVHandlerFifoFull(*this, "copper.err.fifofull"));
    add(new NSMVHandlerLengthFifoFull(*this, "copper.err.lengthfifofull"));
    add(new NSMVHandlerTTRXBelle2LinkError(*this, "ttrx.err.b2linkerr"));
    const DBObject& o_ttrx(obj("ttrx"));
    add(new NSMVHandlerTTRXFirmware(*this, "ttrx.firm", 0,
                                    o_ttrx.hasText("firm") ? o_ttrx.getText("firm") : ""));
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (!o_hslb.getBool("used")) continue;
      HSLB& hslb(m_hslb[i]);
      hslb.open(i);
      std::string vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerHSLBBelle2LinkDown(*this, vname + ".err.b2linkdown", i));
      add(new NSMVHandlerHSLBCOPPERFifoFull(*this, vname + ".err.cprfifofull", i));
      add(new NSMVHandlerHSLBCOPPERLengthFifoFull(*this, vname + ".err.cprlengthfifofull", i));
      add(new NSMVHandlerHSLBFifoFull(*this, vname + ".err.fifofull", i));
      add(new NSMVHandlerHSLBCRCError(*this, vname + ".err.crc", i));

      add(new NSMVHandlerHSLBFirmware(*this, vname + ".firm", i,
                                      o_hslb.hasText("firm") ? o_hslb.getText("firm") : ""));
      vname = StringUtil::form("hslb[%d]", i);
      add(new NSMVHandlerInt(vname + ".reg.adr", true, true, -1));
      add(new NSMVHandlerInt(vname + ".reg.size", true, true, -1));
      add(new NSMVHandlerHSLBRegValue(*this, vname + ".par.val", i));
      FEEConfig fconf;
      int j = 0;
      const DBObject o_fee(obj("fee", i));
      if (m_fee[i] != NULL && fconf.read(o_fee)) {
        vname = StringUtil::form("fee[%d]", i);
        add(new NSMVHandlerText(vname + ".name", true, false, m_fee[i]->getName()));
        const FEEConfig::RegList& regs(fconf.getRegList());
        for (FEEConfig::RegList::const_iterator it = regs.begin();
             it != regs.end(); it++) {
          const FEEConfig::Reg& reg(*it);
          std::string pname = StringUtil::tolower(reg.name);
          std::string vname = StringUtil::form("fee[%d].reg[%d].adr", i, j);
          add(new NSMVHandlerInt(vname, true, false, reg.adr));
          vname = StringUtil::form("fee[%d].reg[%d].size", i, j);
          add(new NSMVHandlerInt(vname, true, false, reg.size));
          vname = StringUtil::form("fee[%d].par[%d].val", i, j++);
          add(new NSMVHandlerHSLBRegValue(*this, vname, i, reg.adr, reg.size));
        }
        vname = StringUtil::form("fee[%d]", i);
        add(new NSMVHandlerFEEStream(*this, vname + ".stream", i,
                                     o_fee.hasText("stream") ? o_fee.getText("stream") : ""));
        add(new NSMVHandlerFEEBoot(*this, vname + ".boot", i));
        vname = StringUtil::form("hslb[%d]", i);
        add(new NSMVHandlerHSLBLinkFee(*this, vname + ".link", i));
        add(new NSMVHandlerHSLBUnLinkFee(*this, vname + ".unlink", i));
        add(new NSMVHandlerHSLBTrgOnFee(*this, vname + ".trigon", i));
        add(new NSMVHandlerHSLBTrgOffFee(*this, vname + ".trigoff", i));
        add(new NSMVHandlerHSLBCheckFee(*this, vname + ".checkfee", i));
        bool checked = hslb.checkfee() != "UNKNOWN";
        const hslb_info& info(hslb.getInfo());
        add(new NSMVHandlerInt(vname + ".hw", true, false, checked ? info.feehw : -1));
        add(new NSMVHandlerInt(vname + ".serial", true, false, checked ? info.feeserial : -1));
        add(new NSMVHandlerText(vname + ".type", true, false,
                                checked ? HSLB::getFEEType(info.feehw) : "UNKNOWN"));
        add(new NSMVHandlerInt(vname + ".ver", true, false, checked ? info.feever : -1));
        add(new NSMVHandlerInt(vname + ".hslbid", true, false, checked ? info.hslbid : -1));
        add(new NSMVHandlerInt(vname + ".hslbver", true, false, checked ? info.hslbver : -1));
      }
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fmver", i, 0x81, 5));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".b2lstat", i, 0x83, 5));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".rxdata", i, 0x84, 5));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwevt", i, 0x85, 5));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".fwclk", i, 0x86, 5));
      add(new NSMVHandlerHSLBRegFixed(*this, vname + ".cntsec", i, 0x87, 5));
    }
  } catch (const std::exception& e) {
    throw (RCHandlerException(e.what()));
  }
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
  m_ttrx.open();
  if (m_ttrx.isError()) {
    m_ttrx.close();
    throw (RCHandlerException("TTRX Link error"));
  }
  try {
    for (int i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj("hslb", i));
      if (o_hslb.getBool("used")) {
        HSLB& hslb(m_hslb[i]);
        hslb.open(i);
        if (!hslb.load()) {
          throw (RCHandlerException("Failed to load HSLB:%c", i + 'a'));
        }
        FEEConfig fconf;
        if (m_fee[i] != NULL && obj.hasObject("fee") &&
            fconf.read(obj("fee", i))) {
          FEE& fee(*m_fee[i]);
          try {
            fee.load(hslb, fconf);
          } catch (const IOException& e) {
            throw (RCHandlerException(e.what()));
          }
        }
      }
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  bootBasf2(obj);
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

void COPPERCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
}

void COPPERCallback::abort() throw(RCHandlerException)
{
  m_con.abort();
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

void COPPERCallback::monitor() throw(RCHandlerException)
{
  try {
    RCState state = getNode().getState();
    try {
      m_ttrx.monitor();
    } catch (const IOException& e) {
      return;
    }
    try {
      m_copper.monitor();
    } catch (const IOException& e) {
      return;
    }
    if (state != RCState::NOTREADY_S && state.isStable()) {
      logging(m_copper.isFifoEmpty(), LogFile::NOTICE, "COPPER FIFO empty");
      logging(m_copper.isFifoFull(), LogFile::WARNING, "COPPER FIFO full");
      logging(m_copper.isLengthFifoFull(), LogFile::WARNING, "COPPER length FIFO full");
      for (int i = 0; i < 4; i++) {
        int used = 0;
        get(StringUtil::form("hslb[%d].used", i), used);
        if (used) {
          HSLB& hslb(m_hslb[i]);
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
  } catch (const std::exception& e) {

  }
}

void COPPERCallback::bootBasf2(const DBObject& obj) throw(RCHandlerException)
{
  if (m_con.isAlive()) return;
  std::string script;
  try {
    int flag = 0;
    for (size_t i = 0; i < 4; i++) {
      const DBObject& o_hslb(obj.getObject("hslb", i));
      if (o_hslb.getBool("use")) flag += 1 << i;
    }
    m_con.clearArguments();
    m_con.setExecutable("basf2");
    script = obj.getObject("basf2").getText("script");
    m_con.addArgument("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                      script.c_str());
    m_con.addArgument(obj.getText("hostname"));
    std::string copperid_s = obj.getText("copperid");
    int copperid = atoi(copperid_s.substr(4).c_str());
    int detectorid = atoi(copperid_s.substr(3).c_str()) / 1000;
    char str[64];
    sprintf(str, "0x%d000000", detectorid);
    copperid += strtol(str, NULL, 0);
    m_con.addArgument(StringUtil::form("%d", copperid));
    m_con.addArgument(StringUtil::form("%d", flag));
    m_con.addArgument("1");
    m_con.addArgument("cprbasf2_" + getNode().getName());
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException(e.what()));
  }
  if (!m_con.load(10)) {
    LogFile::debug("load timeout");
    throw (RCHandlerException("Failed to start readout : " + script));
  }
  LogFile::debug("load succeded");
}

bool COPPERCallback::isError() throw()
{
  if (m_copper.isError()) return true;
  for (int i = 0; i < 4; i++) {
    int used = 0;
    get(StringUtil::form("hslb[%d].used", i), used);
    if (used && m_hslb[i].isError()) return true;
  }
  if (m_ttrx.isBelle2LinkError())
    return true;
  return false;
}
