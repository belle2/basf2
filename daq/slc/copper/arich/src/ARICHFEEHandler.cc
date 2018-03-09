#include "daq/slc/copper/arich/ARICHFEEHandler.h"
#include "daq/slc/copper/arich/ARICHMerger.h"
#include "daq/slc/copper/arich/ARICHFEBSA0x.h"
#include "daq/slc/copper/arich/sa02board.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <unistd.h>

using namespace Belle2;

bool ARICHHandlerGo::handleGetInt(int& index)
{
  return NSMVHandlerInt::handleGetInt(index);
}

bool ARICHHandlerGo::handleSetInt(const int flag)
{
  for (size_t i = 0; i < 6; i++) {
    if ((flag >> i) & 0x1) {
      ARICHFEBSA0x sa03(m_hslb, i);
      sa03.go();
    }
  }
  return true;
}

bool ARICHHandlerLoadParam::handleSetText(const std::string& path)
{
  LogFile::info("loadparam << " + path);
  try {
    ARICHMerger mer(m_callback, m_hslb);
    int id = m_hslb.get_finid();
    const std::string vname = StringUtil::form("arich[%d].", id);
    DBObject o_febs[6];
    int used[6] = { 1, 1, 1, 1, 1, 1 };
    DBObject obj = m_callback.dbload(path);
    for (size_t i = 0; i < 6; i++) {
      m_callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
      if (used[i] > 0) {
        o_febs[i] = obj;
      }
    }
    std::string mode;
    m_callback.get(vname + "mode_set", mode);
    mer.load(obj, o_febs, used, mode);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  LogFile::info("set th end");
  return true;
}

bool ARICHHandlerThreshold::handleGetFloat(float& val)
{
  return NSMVHandlerFloat::handleGetFloat(val);
}

bool ARICHHandlerThreshold::handleSetFloat(float Vth)
{
  try {
    LogFile::info("set th start");
    ARICHMerger mer(m_callback, m_hslb);
    int id = m_hslb.get_finid();
    const std::string vname = StringUtil::form("arich[%d].", id);
    std::string mode;
    int used[6] = { 1, 1, 1, 1, 1, 1 };
    for (size_t i = 0; i < 6; i++) {
      m_callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
    }
    m_callback.get(vname + "mode_set", mode);
    m_hslb.writefee32(0x11, 0x01403f00);
    m_hslb.writefee32(0x13, 0x3f010000); //Trigger Off
    unsigned int val = int(((Vth / 1000) - VTH_MIN) / VTH_STEP);
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
    ARICHFEB::mysleep(1000);
    int num = 0;
    for (size_t i = 0; i < 6; i++) {
      num |= (used[i] > 0) << i;
    }
    usleep(2000);
    if (mode == "suppress") {
      m_hslb.writefee32(0x11, (num << 8) + 0x01410000);
    } else if (mode == "raw") {
      m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
    } else {
      m_hslb.writefee32(0x11, (num << 8) + 0x01210000);
    }
    usleep(20000);
    for (size_t i = 0; i < 6; i++) {
      if (used[i] != true) continue;
      ARICHFEBSA0x sa03(m_hslb, i);
      unsigned int val_rb = sa03.ptm1_data();
      if (val_rb != val) {
        LogFile::error("Failed to set threshold to FEB#%d : %d (%d)", i, val_rb, val);
      } else {
        LogFile::info("Set threshold to FEB#%d : %d (%d)", i, val_rb, val);
      }
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  LogFile::info("set th end");
  return true;
}

bool ARICHHandlerThIndex::handleGetInt(int& index)
{
  return NSMVHandlerInt::handleGetInt(index);
}

bool ARICHHandlerThIndex::handleSetInt(int index)
{
  try {
    LogFile::info("set th start");
    ARICHMerger mer(m_callback, m_hslb);
    int id = m_hslb.get_finid();
    float th0 = 0, dth = 0;
    const std::string vname = StringUtil::form("arich[%d].", id);
    int used[6] = { 1, 1, 1, 1, 1, 1 };
    for (size_t i = 0; i < 6; i++) {
      m_callback.get(vname + StringUtil::form("feb[%d].used", i), used[i]);
    }
    m_callback.get(vname + "th0", th0);
    m_callback.get(vname + "dth", dth);
    std::string mode;
    m_callback.get(vname + "mode_set", mode);
    int val_in = mer.setThreshold(th0, dth, used, index, mode);
    usleep(1000);
    for (size_t i = 0; i < 6; i++) {
      if (used[i] != true) continue;
      ARICHFEBSA0x sa03(m_hslb, i);
      //sa03.ptm1_cmd();
      int val_rb = sa03.ptm1_data();
      //sa03.ptm1_rb_cmd();
      //int val_rb = sa03.ptm1_rb_data();
      if (val_rb != val_in) {
        LogFile::error("Failed to set threshold to FEB#%d : %d (%d)", i, val_rb, val_in);
      } else {
        LogFile::info("Set threshold to FEB#%d : %d (%d)", i, val_rb, val_in);
      }
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  LogFile::info("set th end");
  return true;
}

bool ARICHHandlerReset::handleGetInt(int& v)
{
  return NSMVHandlerInt::handleGetInt(v);
}

bool ARICHHandlerReset::handleSetInt(int)
{
  try {
    LogFile::info("reset FEBs");
    unsigned int val = m_hslb.readfee32(0x11);
    m_hslb.writefee32(0x11, (val & 0x0FFFF00) | 0x010000);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  LogFile::info("reset FEBs done");
  return true;
}

bool ARICHHandlerPTM1::handleGetInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  val = sa03.ptm1_data();
  LogFile::debug("read ptm1_data : %d", val);
  return true;
}

bool ARICHHandlerPTM1::handleSetInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  LogFile::debug("write ptm1_data : %d", val);
  sa03.ptm1_data(val);
  //usleep(1000);
  sa03.ptm1_cmd(PTM1_WR_ADDR);
  //usleep(1000);
  return true;
}

bool ARICHHandlerHDCycle::handleGetInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  val = sa03.hdcycle();
  //usleep(1000);
  LogFile::debug("read hdcycle : %d", val);
  return true;
}

bool ARICHHandlerHDCycle::handleSetInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  LogFile::debug("write hdcycle : %d", val);
  sa03.hdcycle(val);
  //usleep(1000);
  return true;
}

bool ARICHHandlerTrgDelay::handleGetInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  val = sa03.trgdelay();
  //usleep(1000);
  LogFile::debug("read trgdelay : %d", val);
  return true;
}

bool ARICHHandlerTrgDelay::handleSetInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  LogFile::debug("write trgdelay : %d", val);
  sa03.trgdelay(val);
  //usleep(1000);
  return true;
}


//ASIC global param.
#ifdef tmp1
bool ARICHHandlerAsicPhasecmps::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_phasecmps();

  LogFile::debug("read asic phasecmps : %d", val);
  return true;
}

bool ARICHHandlerAsicPhasecmps::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);


  prm.set_phasecmps(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic phasecmps : %d", val);
  return true;

}

#endif


bool ARICHHandlerAsicGain::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_gain();

  LogFile::debug("read asic gain : %d", val);
  return true;
}

bool ARICHHandlerAsicGain::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);


  prm.set_gain(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic gain : %d", val);
  return true;

}

#ifdef tmp2
bool ARICHHandlerAsicShapingTime::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_shapingtime();

  LogFile::debug("read asic shapingtime : %d", val);
  return true;
}

bool ARICHHandlerAsicShapingTime::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);


  prm.set_shapingtime(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic shapingtime : %d", val);
  return true;

}

bool ARICHHandlerAsicComparator::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_comparator();

  LogFile::debug("read asic comparator : %d", val);
  return true;
}

bool ARICHHandlerAsicComparator::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  prm.set_comparator(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic comparator : %d", val);
  return true;

}

bool ARICHHandlerAsicVRDrive::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_vrdrive();

  LogFile::debug("read asic vrdrive : %d", val);
  return true;
}

bool ARICHHandlerAsicVRDrive::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);


  prm.set_vrdrive(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic vrdrive : %d", val);
  return true;

}

bool ARICHHandlerAsicMonitor::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_monitor();

  LogFile::debug("read asic monitor : %d", val);
  return true;
}

bool ARICHHandlerAsicMonitor::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);


  prm.set_monitor(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic monitor : %d", val);
  return true;

}

bool ARICHHandlerAsicId::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_id();

  LogFile::debug("read asic id : %d", val);
  return true;
}

bool ARICHHandlerAsicId::handleSetAsicInt(int val)
{
  return true;
}

bool ARICHHandlerAsicMasked::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xGlobalParam tmp[4];
  SA0xGlobalParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_masked();

  LogFile::debug("read asic masked : %d", val);
  return true;
}

bool ARICHHandlerAsicMasked::handleSetAsicInt(int val)
{
  return true;
}

#endif


//ASIC channel param.
bool ARICHHandlerAsicDecayTime::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_decaytime();

  LogFile::debug("read asic decaytime : %d", val);
  return true;
}

bool ARICHHandlerAsicDecayTime::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_decaytime(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic decaytime : %d", val);
  return true;

}

#ifdef tmp3
bool ARICHHandlerAsicOffset::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_offset();

  LogFile::debug("read asic doffset : %d", val);
  return true;
}

bool ARICHHandlerAsicOffset::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_offset(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic offset : %d", val);
  return true;

}

bool ARICHHandlerAsicFineadj_Unipol::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_fineadj_unipol();

  LogFile::debug("read asic fineadj_unipol : %d", val);
  return true;
}

bool ARICHHandlerAsicFineadj_Unipol::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_fineadj_unipol(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic fineadj_unipol : %d", val);
  return true;

}

bool ARICHHandlerAsicFineadj_Diff::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_fineadj_diff();

  LogFile::debug("read asic fineadj_diff : %d", val);
  return true;
}

bool ARICHHandlerAsicFineadj_Diff::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_fineadj_diff(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic fineadj_diff : %d", val);
  return true;

}

bool ARICHHandlerAsicTpenb::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_tpenb();

  LogFile::debug("read asic tpenb : %d", val);
  return true;
}

bool ARICHHandlerAsicTpenb::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_tpenb(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic Tpenb : %d", val);
  return true;

}

bool ARICHHandlerAsicKill::handleGetAsicInt(int& val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);

  sa03.select(m_chip, m_ch);
  sa03.ndro();

  int val0 = sa03.rparam();
  prm.rbparam(val0);
  val = prm.getrb_kill();

  LogFile::debug("read asic kill : %d", val);
  return true;
}

bool ARICHHandlerAsicKill::handleSetAsicInt(int val)
{
  ARICHFEBSA0x sa03(m_hslb, m_index);
  SA0xChannelParam tmp[4];
  SA0xChannelParam& prm(tmp[m_chip]);


  prm.set_kill(val);

  sa03.select(m_chip, m_ch);
  sa03.wparam(prm.param());
  sa03.prmset(); // load parameter

  LogFile::debug("write asic kill : %d", val);
  return true;

}

#endif
