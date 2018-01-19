#include "daq/slc/copper/arich/ARICHMerger.h"
#include "daq/slc/copper/arich/ARICHFEBSA0x.h"
#include "daq/slc/copper/arich/sa02board.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <sstream>
#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

ARICHMerger::ARICHMerger(RCCallback& callback, HSLB& hslb) : m_callback(callback), m_hslb(hslb)
{
}

void ARICHMerger::boot(const DBObject& obj, const DBObject o_febs[6], int used[6])
{
  LogFile::debug("Booting FEBs");
  m_hslb.writefee32(0x11,  0x02400000);
  usleep(1000);
  m_hslb.writefee32(0x13,  0x000000);
  usleep(1000);
  m_hslb.writefee32(0x12, 0x050000);
  usleep(1000);
  const std::string vname = StringUtil::form("arich[%d]", m_hslb.get_finid());
  for (int i = 0; i < 6; i++) {
    m_callback.set(vname + StringUtil::form(".feb[%d].initb", i), (int)0);
    m_callback.set(vname + StringUtil::form(".feb[%d].done", i), (int)0);
  }
  int num = 0;
  std::stringstream ss;
  for (size_t i = 0; i < 6; i++) {
    ss << (used[i] > 0) << " ";
    num |= (used[i] > 0) << i;
    LogFile::debug("0x13 << 0x%x", (num << 24));
    m_hslb.writefee32(0x13, (num << 24) + 0x000000);
    usleep(10000);
  }
  LogFile::info("FEE enabled: " + ss.str());
  m_hslb.writefee32(0x12, (num << 24) + 0x050000);
  usleep(1000);
  m_hslb.writefee32(0x11, (num << 8) + 0x010000);
  usleep(1000);
  std::string firmware; //= "/usr/local/share/firmware/arich/sa03b3fe2-v05cand2.bin";
  m_callback.get(vname + ".feb.firmware", firmware);
  //firmware = "/usr/local/share/firmware/arich/" + firmware;
  firmware = "/home/usr/b2daq/arich/firmware/" + firmware;
  //std::string firmware = "/usr/local/share/firmware/arich/sa03b3fe2-v06cand1.bin";
  LogFile::info("Loging FEB firmware : " + firmware);
  try {
    m_hslb.writestream(firmware.c_str());//"sa03b3fe2_ferst.bin");
  } catch (const std::exception& e) {
    LogFile::fatal(e.what());
    return;
  }
  usleep(1000);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] > 0) {
      ARICHFEBSA0x sa03(m_hslb, i);
      sa03.write1(0x10, 0x01);
      sa03.read1(0x10);
      sa03.write1(0x03, 0x04);
      sa03.read1(0x03);
      sa03.write1(0x04, 0x010);
      sa03.read1(0x04);
      sa03.write1(0x05, 0x08);
      sa03.read1(0x05);
      sa03.write1(0x09, 0x01);
      sa03.read1(0x09);
      sa03.write1(0x0a, 0x0);
      sa03.read1(0x0a);
      sa03.write1(0x0b, 0x0);
      sa03.read1(0x0b);
    }
  }
  usleep(1000);
  unsigned int v = m_hslb.readfee32(0x12);
  m_callback.set(vname + ".reg[12]", (int)v);
  ss.str("");
  for (int i = 0; i < 6; i++) {
    int enable = (int)((v >> i) & 0x1);
    ss << enable << " ";
    m_callback.set(vname + StringUtil::form(".feb[%d].initb", i), enable);
  }
  LogFile::debug("FEE_INITB: " + ss.str());
  ss.str("");
  for (int i = 0; i < 6; i++) {
    int enable = (int)((v >> (i + 8)) & 0x1);
    ss << enable << " ";
    m_callback.set(vname + StringUtil::form(".feb[%d].done", i), enable);
  }
  LogFile::info("FEE_DONE: " + ss.str());
  v = m_hslb.readfee32(0x11);
  m_callback.set(vname + ".reg[11]", (int)v);
  v = m_hslb.readfee32(0x13);
  m_callback.set(vname + ".reg[13]", (int)v);
  LogFile::debug("---Setup is done.---");
}

void ARICHMerger::load(const DBObject& o_mgr, const DBObject o_febs[6], int used[6], const std::string& mode)
{
  int csr = 0;
  m_callback.get("csr", csr);
  const std::string vname = StringUtil::form("arich[%d].", m_hslb.get_finid());
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    const DBObject& o_feb(o_febs[i]);
    LogFile::debug("HSLB:%c, FEE # = %d", ('a' + m_hslb.get_finid()), i);
    ARICHFEBSA0x sa03(m_hslb, i);
    // initialization
    sa03.init();//debug. it should be removed
    sa03.csr1(csr);//o_mgr.getInt("csr1"));
    const std::string cvname = vname + StringUtil::form("feb[%d].", i);
    int hdcycle = -1;
    m_callback.get(cvname + "hdcycle_set", hdcycle);
    if (hdcycle >= 0) {
      LogFile::info("hdcycle_set=%d", hdcycle);
      sa03.hdcycle(hdcycle);
    } else {
      sa03.hdcycle(o_mgr.getInt("hdcycle"));
    }
    int trgdelay = -1;
    m_callback.get(cvname + "trgdelay_set", trgdelay);
    if (trgdelay >= 0) {
      sa03.trgdelay(trgdelay);
    } else {
      sa03.trgdelay(o_mgr.getInt("trgdelay"));
    }
    //sa03.trigen(o_mgr.getInt("trigen"));
    //sa03.init();

    // global parameter
    const DBObjectList& o_chips(o_feb.getObjects("chip"));
    for (size_t ichip = 0; ichip < o_chips.size(); ichip++) {
      const DBObject& o_chip(o_chips[ichip]);
      SA0xGlobalParam& gparam(m_globalparam[ichip]);
      gparam.param(o_chip.getInt("param_global"));
      gparam.set_phasecmps(o_chip.getInt("phasecmps"));
      gparam.set_gain(o_chip.getInt("gain"));
      gparam.set_shapingtime(o_chip.getInt("shapingtime"));
      gparam.set_comparator(o_chip.getInt("comparator"));
      gparam.set_vrdrive(o_chip.getInt("vrdrive"));
      gparam.set_monitor(o_chip.getInt("monitor"));
      load_global(sa03, ichip);
    }

    // channel parameter
    for (size_t ichip = 0; ichip < o_chips.size(); ichip++) {
      const DBObject& o_chip(o_chips[ichip]);
      const DBObjectList& o_chs(o_chip.getObjects("ch"));
      for (size_t ich = 0; ich < o_chs.size(); ich++) {
        const DBObject& o_ch(o_chs[ich]);
        SA0xChannelParam& cparam(m_channelparam[ichip][ich]);
        cparam.param(o_ch.getInt("param_ch"));
        cparam.set_decaytime(o_ch.getInt("decaytime"));
        cparam.set_offset(o_ch.getInt("offset"));
        cparam.set_fineadj_unipol(o_ch.getInt("fineadj_unipol"));
        cparam.set_fineadj_diff(o_ch.getInt("fineadj_diff"));
        cparam.set_tpenb(o_ch.getInt("tpenb"));
        cparam.set_kill(o_ch.getInt("kill"));
        load_ch(sa03, ichip, ich);
      }
    }

    // select chip-channel
    const DBObjectList& o_selects(o_mgr.getObjects("select"));
    for (size_t is = 0; is < o_selects.size(); is++) {
      const DBObject& o_select(o_selects[is]);
      if (o_select.hasValue("chip") && o_select.getValue("ch")) {
        sa03.select(o_select.getInt("chip"), o_select.getInt("ch"));
      }
    }
  }

  m_hslb.writefee32(0x11, 0x01403f00); //SCK STOP disable (transfer mode is changed)
  m_callback.set(vname + ".reg[11]", (int)m_hslb.readfee32(0x11));
  m_hslb.writefee32(0x13, 0x3f010000); //Trigger Off
  m_callback.set(vname + ".reg[13]", (int)m_hslb.readfee32(0x13));
  usleep(5000);

  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.trigen(0);
  }
  usleep(5000);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.utilbuf(0x0000);
  }
  usleep(5000);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.trigen(1);
  }
  float vth = 0;
  m_callback.get(vname + "vth", vth);
  m_callback.set(vname + "vth", vth);
  unsigned int val = int(((vth / 1000) - VTH_MIN) / VTH_STEP);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.ptm1_data(val);
    ARICHFEB::mysleep(1000);
    sa03.ptm1_cmd(PTM1_WR_ADDR);
    ARICHFEB::mysleep(1000);
    sa03.utilbuf(0);
  }
  ARICHFEB::mysleep(1000);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.trigen(1);
  }

  usleep(2000);
  int num = 0;
  for (size_t i = 0; i < 6; i++) {
    num |= (used[i] > 0) << i;
  }
  if (mode == "raw") {
    m_hslb.writefee32(0x11, (num << 8) + 0x01410000);
  } else if (mode == "suppress") {
    m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
  } else {
    m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
  }
  unsigned int v = m_hslb.readfee32(0x11);
  m_callback.set(vname + ".reg[11]", (int)v);
  usleep(2000);
  LogFile::info("set %s mode", mode.c_str());
}

int ARICHMerger::setThreshold(double th0, double dth/*, const DBObject& obj*/, int used[6], int index, const std::string& mode)
{
  const std::string vname = StringUtil::form("arich[%d].", m_hslb.get_finid());
  m_hslb.writefee32(0x11, 0x01403f00); //SCK STOP disable
  m_hslb.writefee32(0x13, 0x3f010000); //Trigger Off
  unsigned int val_utilbuf = 0x1001 + index;
  double Vth = th0 + index * dth;
  m_callback.set(vname + "vth", (float)Vth * 1000);
  LogFile::info("dth = %f, th0 = %f, Vth = %f", dth, th0, Vth);
  unsigned int val = int((Vth - VTH_MIN) / VTH_STEP);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.ptm1_data(val);
    //usleep(1000);
    ARICHFEB::mysleep(1000);
    sa03.ptm1_cmd(PTM1_WR_ADDR);
    //usleep(1000);
    ARICHFEB::mysleep(1000);
    sa03.utilbuf(val_utilbuf);
  }
  ARICHFEB::mysleep(1000);
  for (size_t i = 0; i < 6; i++) {
    if (used[i] == 0) continue;
    ARICHFEBSA0x sa03(m_hslb, i);
    sa03.trigen(1);
  }
  //usleep(2000);
  ARICHFEB::mysleep(1000);
  int num = 0;
  //const DBObject& o_feb(o_febs[i]);
  for (size_t i = 0; i < 6; i++) {
    num |= (used[i] > 0) << i;
  }
  usleep(2000);
  if (mode == "raw") {
    m_hslb.writefee32(0x11, (num << 8) + 0x01410000);
  } else if (mode == "raw") {
    m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
  } else {
    m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
  }
  usleep(20000);

  return val;
}

void ARICHMerger::load_global(ARICHFEBSA0x& sa03, unsigned int chip)
{
  SA0xGlobalParam& prm(m_globalparam[chip]);
  for (int i = 0; i < 5; i++) {
    sa03.select(chip, -1);
    printf("param: 0x%x\n", prm.param());
    sa03.wparam(prm.param());
    sa03.prmset(); // load parameter
    sa03.ndro();   // ndro
    int val = sa03.rparam();
    prm.rbparam(val);
    printf("rbparam: 0x%x\n", val);
    /* compare */
    if (prm.compare(false) == 0) {
      LogFile::debug("HLSB:%c FEB:%d  global param 0x%07x correctly loaded for chip %d (id=0x%x).",
                     ('a' + m_hslb.get_finid()), sa03.num(), prm.param(), chip, prm.getrb_id());
      return;
    } else {
      if (i == 0) {
        m_callback.log(LogFile::WARNING, "HLSB:%c FEB:%d  Error in global param for chip %d (id=0x%x):"
                       "(set 0x%07x readback 0x%07x).", ('a' + m_hslb.get_finid()),
                       sa03.num(), chip,
                       prm.getrb_id(), prm.param(), prm.getrb_masked());
      }
    }
  }
  /*
  throw (RCHandlerException("HLSB:%c FEB:%d  Error in global param for chip %d (id=0x%x):"
          "(set 0x%07x readback 0x%07x).", ('a'+m_hslb.get_finid()),
          sa03.num(), chip,
          prm.getrb_id(), prm.param(), prm.getrb_masked()));
  */
}

void ARICHMerger::load_ch(ARICHFEBSA0x& sa03, unsigned int chip, unsigned int ch)
{
  SA0xChannelParam& prm(m_channelparam[chip][ch]);
  for (int i = 0; i < 5; i++) {
    sa03.select(chip, ch);
    sa03.wparam(prm.param());
    sa03.prmset(); // load parameter
    sa03.ndro(); // load parameter once more
    int val = sa03.rparam();
    prm.rbparam(val);
    /* compare */
    if (prm.compare(false) == 0) {
      //LogFile::debug("HLSB:%c FEB:%d  ch. param 0x%07x correctly loaded for chip %d ch %2d.",
      //       ('a'+m_hslb.get_finid()), sa03.num(), prm.param(), chip, ch);
      usleep(500);
      return;
    } else {
      if (i == 0) {
        m_callback.log(LogFile::WARNING, "HLSB:%c FEB:%d Error in ch. param for chip %d ch %2d:"
                       "(set 0x%07x readback 0x%07x).",
                       ('a' + m_hslb.get_finid()), sa03.num(), chip, ch,
                       prm.param(), prm.rbparam());
      }
    }
    usleep(400);
  }
  /*
  throw (RCHandlerException("HLSB:%c FEB:%d  Error in ch. param for chip %d ch %2d:"
          "(set 0x%07x readback 0x%07x).",
          ('a'+m_hslb.get_finid()), sa03.num(), chip, ch,
          prm.param(), prm.rbparam()));
  */
}

