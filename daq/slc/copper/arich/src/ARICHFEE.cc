#include "daq/slc/copper/arich/ARICHFEE.h"
#include "daq/slc/copper/arich/ARICHFEBSA0x.h"
#include "daq/slc/copper/arich/ARICHMerger.h"
#include "daq/slc/copper/arich/ARICHFEEHandler.h"
#include "daq/slc/copper/arich/SA0xGlobalParam.h"

#include "daq/slc/copper/FEEHandler.h"

#include "daq/slc/database/DBObjectLoader.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>
#include <sstream>

using namespace Belle2;

ARICHFEE::ARICHFEE()
{
}

void ARICHFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  const std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  callback.add(new NSMVHandlerFloat(vname + "th0", true, true, 0));
  callback.add(new NSMVHandlerFloat(vname + "dth", true, true, 0));
  callback.add(new NSMVHandlerFloat(vname + "temp", true, true, 0));

  callback.add(new FEE32Handler(vname + "reg[10]", callback, hslb, *this, 0x0010));
  callback.add(new FEE32Handler(vname + "reg[11]", callback, hslb, *this, 0x0011));
  callback.add(new FEE32Handler(vname + "reg[12]", callback, hslb, *this, 0x0012));
  callback.add(new FEE32Handler(vname + "reg[13]", callback, hslb, *this, 0x0013));
  callback.add(new FEE32Handler(vname + "reg[14]", callback, hslb, *this, 0x0014));

  callback.add(new NSMVHandlerInt(vname + "syn_date", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "firm.rev", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "width", true, false, 0));
  callback.add(new NSMVHandlerText(vname + "mode", true, false, ""));
  callback.add(new NSMVHandlerInt(vname + "sckstop", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "reset", true, false, 0));
  for (int i = 0; i < 6; i++) {
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].enable", i), true, false, 0));
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].jtag.enable", i), true, false, 0));
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].done", i), true, false, 0));
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].initb", i), true, false, 0));
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].shutdown", i), true, false, 0));
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].trgcnt", i), true, false, 0));
  }
  callback.add(new NSMVHandlerInt(vname + "test.trg", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "test.trgfeb", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "jtag.clock", true, false, 0));
  for (int i = 0; i < 4; i++) {
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("trg[%d].enable", i), true, false, 0));
  }
  callback.add(new NSMVHandlerInt(vname + "internal.trg.speed", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "internal.trg.enable", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "merger.test.slow.enable", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "merger.test.module.enable", true, false, 0));
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 4; j++) {
      const std::string cvname = vname + StringUtil::form("feb[%d].chip[%d].", i, j);
      callback.add(new NSMVHandlerInt(cvname + "busy", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "phasecmps", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "gain", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "shapingtime", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "comparator", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "vrdrive", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "monitor", true, false, 0));
      callback.add(new NSMVHandlerInt(cvname + "id", true, false, 0));
      callback.add(new ARICHHandlerAsicGain(cvname + "gain", callback, hslb, *this, i, j));//yone
    }
  }
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 4; j++) {
      for (int k = 0; k < 36; k++) {
        const std::string cvname = vname + StringUtil::form("feb[%d].chip[%d].ch[%d].", i, j, k);
        callback.add(new ARICHHandlerAsicDecayTime(cvname + "decaytime", callback, hslb, *this, i, j, k));
      }
    }
    const DBObject& o_feb(obj("feb", i));
    bool used = o_feb.getBool("used");
    if (used) {
      m_o_feb[i] = callback.dbload(o_feb.getText("path"));
    }
    callback.add(new NSMVHandlerInt(vname + StringUtil::form("feb[%d].used", i), true, true, (int)used));
  }
  callback.add(new NSMVHandlerText(vname + "mode_set", true, true, obj.getText("mode")));
  callback.add(new NSMVHandlerFloat(vname + "vth", true, true, obj.getFloat("vth")));

  callback.add(new ARICHHandlerLoadParam(vname + "loadparam", callback, hslb, *this));
  callback.add(new ARICHHandlerThIndex(vname + "thindex", callback, hslb, *this));
  //callback.add(new ARICHHandlerThreshold(vname + "vth", callback, hslb, *this));
  callback.add(new ARICHHandlerReset(vname + "reset", callback, hslb, *this));
  for (int i = 0; i < 6; i++) {
    const std::string cvname = vname + StringUtil::form("feb[%d].", i);
    callback.add(new NSMVHandlerInt(cvname + "thval", true, false, 0));
    callback.add(new ARICHHandlerPTM1(cvname + "ptm1", callback, hslb, *this, i));
    callback.add(new ARICHHandlerHDCycle(cvname + "hdcycle", callback, hslb, *this, i));//yone
    callback.add(new ARICHHandlerTrgDelay(cvname + "trgdelay", callback, hslb, *this, i));//yone
    callback.add(new NSMVHandlerFloat(cvname + "mona", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "monb", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "monc", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "mond", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vdd", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vp2", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vm2", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vss", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vth1", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vth2", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vcc12", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vcc15", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vcc25", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "vp38", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "t1", true, false, 0));
    callback.add(new NSMVHandlerFloat(cvname + "t2", true, false, 0));
  }
}

void ARICHFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int used[6] = { 1, 1, 1, 1, 1, 1 };
  const std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  for (size_t i = 0; i < 6; i++) {
    callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
    callback.set(vname + StringUtil::form("feb[%d].used", i), used[i]);
  }
  std::string mode;
  callback.get(vname + "mode_set", mode);
  float vth = 0;
  callback.get(vname + "vth", vth);
  ARICHMerger mer(callback, hslb);
  mer.boot(obj, m_o_feb, used);
}

void logmask(const std::string& name, int mask, int max)
{
  std::stringstream ss;
  ss << name << StringUtil::form(": %x", mask);
  for (int i = 0; i < max; i++) {
    ss << i << ":" << ((mask >> i) & 0x1) << " ";
  }
  LogFile::debug(ss.str().c_str());
}

void ARICHFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int used[6] = { 1, 1, 1, 1, 1, 1 };
  const std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  for (size_t i = 0; i < 6; i++) {
    callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
  }
  std::string mode;
  callback.get(vname + "mode_set", mode);
  ARICHMerger mer(callback, hslb);
  mer.load(obj, m_o_feb, used, mode);
}

void ARICHFEE::start(RCCallback& callback, HSLB& hslb)
{
  try {
    unsigned int val = hslb.readfee32(0x11);
    hslb.writefee32(0x11, (val & 0x0FFFF00) | 0x010000);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
}

void ARICHFEE::stop(RCCallback& callback, HSLB& hslb)
{
  try {
    unsigned int val = hslb.readfee32(0x11);
    hslb.writefee32(0x11, (val & 0x0FFFF00) | 0x010000);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
}

float convert_temp(unsigned int d)
{
  unsigned int flag = d & 0x8000;
  unsigned int val = (d & 0x7ff8) >> 3;
  if (d & 0x4) {
    return -1;
  }
  return (!flag) ? 0.0625 * val : -0.0625 * ((~val) + 1);
}

void ARICHFEE::monitor(RCCallback& callback, HSLB& hslb)
{
  std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  m_reg[0x0010] = hslb.readfee32(0x0010);
  m_reg[0x0011] = hslb.readfee32(0x0011);
  m_reg[0x0012] = hslb.readfee32(0x0012);
  m_reg[0x0013] = hslb.readfee32(0x0013);
  m_reg[0x0014] = hslb.readfee32(0x0014);
  /*
  hslb.writefee32(0x0011, m_reg[0x0011] & 0xFFFFF0);
  hslb.writefee32(0x0011, m_reg[0x0011] | 0x1);
  unsigned int d = hslb.readfee32(0x0015);// & 0xFFFF;
  float temp = convert_temp(d);
  LogFile::info("d=0x%x", d, temp);
  callback.set(vname + "temp", temp);
  */
  hslb.writefee32(0x0100, 0x10);
  hslb.writefee32(0x0100, 0x00);
  unsigned int d = (hslb.readfee32(0x0100) & 0xFFFF) >> 6;
  float temp = d * 0.49 - 273;
  //callback.set(vname + "tempfpga", temp);
  callback.set(vname + "temp", temp);
  //LogFile::info("d=0x%x", d, temp);
  callback.set(vname + "reg[10]", (int)m_reg[0x0010]);
  callback.set(vname + "reg[11]", (int)m_reg[0x0011]);
  callback.set(vname + "reg[12]", (int)m_reg[0x0012]);
  callback.set(vname + "reg[13]", (int)m_reg[0x0013]);
  callback.set(vname + "reg[14]", (int)m_reg[0x0014]);
  unsigned int val = hslb.readfee32(0x0014);
  callback.set(vname + "feb[0].trgcnt", (int)((val >> 0) & 0xF));
  callback.set(vname + "feb[1].trgcnt", (int)((val >> 4) & 0xF));
  callback.set(vname + "feb[2].trgcnt", (int)((val >> 8) & 0xF));
  callback.set(vname + "feb[3].trgcnt", (int)((val >> 12) & 0xF));
  callback.set(vname + "feb[4].trgcnt", (int)((val >> 16) & 0xF));
  callback.set(vname + "feb[5].trgcnt", (int)((val >> 20) & 0xF));

  int used[6] = { 1, 1, 1, 1, 1, 1 };
  for (size_t i = 0; i < 6; i++) {
    callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
    if (used[i]) {
      const std::string cvname = vname + StringUtil::form("feb[%d].", i);
      ARICHFEBSA0x sa03(hslb, i);
      float mona = sa03.voltage(0x00);
      float monb = sa03.voltage(0x01);
      float mond = sa03.voltage(0x02);
      float monc = sa03.voltage(0x03);
      float vdd = sa03.voltage(0x10);
      float vp2 = sa03.voltage(0x11);
      float vm2 = 2 * sa03.voltage(0x12) - ADC_VCC;
      float vss = 2 * sa03.voltage(0x13) - ADC_VCC;
      float vth1 = sa03.voltage(0x20);
      float vth2 = sa03.voltage(0x30);
      float vcc12 = sa03.voltage(0x40);
      float vcc15 = sa03.voltage(0x50);
      float vcc25 = sa03.voltage(0x60);
      float vp38 = 2 * sa03.voltage(0x70);
      callback.set(cvname + "mona", mona);
      callback.set(cvname + "monb", monb);
      callback.set(cvname + "monc", monc);
      callback.set(cvname + "mond", mond);
      callback.set(cvname + "vdd", vdd);
      callback.set(cvname + "vp2", vp2);
      callback.set(cvname + "vm2", vm2);
      callback.set(cvname + "vss", vss);
      callback.set(cvname + "vth1", vth1);
      callback.set(cvname + "vth2", vth2);
      callback.set(cvname + "vcc12", vcc12);
      callback.set(cvname + "vcc15", vcc15);
      callback.set(cvname + "vcc25", vcc25);
      callback.set(cvname + "vp38", vp38);
      sa03.ts1_update();
      sa03.ts2_update();
      usleep(500);
      unsigned int d1 = sa03.ts1_data();
      unsigned int d2 = sa03.ts2_data();
      float t1 = convert_temp(d1);
      float t2 = convert_temp(d2);
      //LogFile::debug("ts1(ASIC) = %8.4f (0x%x)", t1, d1);
      //LogFile::debug("ts2(FPGA) = %8.4f (0x%x)", t2, d2);
      callback.set(cvname + "t1", t1);
      callback.set(cvname + "t2", t2);
    }
  }
}

/*
void ARICHFEE::go(HSLB& hslb)
{
  for (size_t i = 0; i < 6; i++) {
    ARICHFEBSA0x sa03(hslb, i);
    sa03.go();
  }
}
*/

void ARICHFEE::readback(RCCallback& callback, HSLB& hslb, const DBObject& /*obj*/)
{
  m_reg[0x0010] = hslb.readfee32(0x0010);
  m_reg[0x0011] = hslb.readfee32(0x0011);
  m_reg[0x0012] = hslb.readfee32(0x0012);
  m_reg[0x0013] = hslb.readfee32(0x0013);
  const std::string vname = StringUtil::form("arich[%d].", hslb.get_finid());
  callback.set(vname + "syn_date", (int)mask(m_reg[0x0010], -1, -1));
  callback.set(vname + "firm.rev", (int)mask(m_reg[0x0011], 31, 24));
  callback.set(vname + "width", (int)mask(m_reg[0x0011], 23, 23));
  int mode = (int)mask(m_reg[0x0011], 22, 21);
  std::string mode_s = "unknown";
  switch (mode) {
    case 2: mode_s = "raw"; break;
    case 1: mode_s = "suppress"; break;
    case 3: mode_s = "raw-suppress"; break;
    default: break;
  }
  callback.set(vname + "mode", mode_s);
  callback.set(vname + "sckstop", (int)mask(m_reg[0x0011], 17, 17));
  callback.set(vname + "reset", (int)mask(m_reg[0x0011], 16, 16));
  bool enabled[6];
  for (int i = 0; i < 6; i++) {
    enabled[i] = true;
    int val = (int)mask(m_reg[0x0011], 8 + i, 8 + i);
    callback.set(vname + StringUtil::form("feb[%d].enable", i), val);
    enabled[i] &= val;
    val = (int)mask(m_reg[0x0012], 24 + i, 24 + i);
    callback.set(vname + StringUtil::form("feb[%d].jtag.enable", i), val);
    enabled[i] &= val;
    val = (int)mask(m_reg[0x0012], 8 + i, 8 + i);
    callback.set(vname + StringUtil::form("feb[%d].done", i), val);
    enabled[i] &= val;
    val = (int)mask(m_reg[0x0012], i, i);
    callback.set(vname + StringUtil::form("feb[%d].initb", i), val);
    enabled[i] &= val;
    val = (int)mask(m_reg[0x0013], 24 + i, 24 + i);
    callback.set(vname + StringUtil::form("feb[%d].shutdown", i), val);
    enabled[i] &= val;
  }
  callback.set(vname + "test.trg", (int)mask(m_reg[0x0011], 7, 7));
  callback.set(vname + "test.trgfee", (int)mask(m_reg[0x0011], 6, 6));
  callback.set(vname + "jtag.clock", (int)mask(m_reg[0x0012], 23, 16));
  for (int i = 0; i < 4; i++) {
    callback.set(vname + StringUtil::form("trg[%d].enable", i), (int)mask(m_reg[0x0013], 16 + i, 16 + i));
  }
  callback.set(vname + "internal.trg.speed", (int)mask(m_reg[0x0013], 14, 12));
  callback.set(vname + "internal.trg.enable", (int)mask(m_reg[0x0013], 8, 8));
  callback.set(vname + "merger.test.slow.enable", (int)mask(m_reg[0x0013], 4, 4));
  callback.set(vname + "merger.test.module.enable", (int)mask(m_reg[0x0013], 0, 0));
  for (int ifeb = 0; ifeb < 6; ifeb++) {
    if (!enabled[ifeb]) continue;
    ARICHFEBSA0x sa03(hslb, ifeb);
    sa03.init();
    for (int ichip = 0; ichip < 4; ichip++) {
      SA0xGlobalParam par;
      sa03.select(ichip, -1);
      sa03.ndro();
      int val = sa03.rparam();
      LogFile::debug("feb = %d chip=%d val=0x%x", ifeb, ichip, val);
      par.rbparam(val);
      std::string cvname = vname + StringUtil::form("feb[%d].", ifeb);
      callback.set(cvname + "busy", (val >> 12) & 0x1);
      cvname = vname + StringUtil::form("feb[%d].chip[%d].", ifeb, ichip);
      callback.set(cvname + "phasecmps", par.getrb_phasecmps());
      callback.set(cvname + "gain", par.getrb_gain());
      callback.set(cvname + "shapingtime", par.getrb_shapingtime());
      callback.set(cvname + "comparator", par.getrb_comparator());
      callback.set(cvname + "vrdrive", par.getrb_vrdrive());
      callback.set(cvname + "monitor", par.getrb_monitor());
      callback.set(cvname + "id", par.getrb_id());
    }
  }
}

extern "C" {
  void* getARICHFEE()
  {
    return new Belle2::ARICHFEE();
  }
}
