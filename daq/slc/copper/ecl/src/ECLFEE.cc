#include "daq/slc/copper/ecl/ECLFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/hsreg.h>

#include <unistd.h>

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

// TODO: Reduce number of calls to getCprCollector
// TODO: Reduce number of calls to getShapersInCollector
//  (Might just set the variables in init(...) method.

// TODO: Add custom operations before AND after routine operations.
// TODO: Optimize writeToShaper via intelligent masking

// TODO: Support for RELAY_G (reg 0x31) and run type register (0x221)

ECLFEE::ECLFEE() : FEE("ecl")
{
}

void ECLFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& /*obj*/)
{
  std::string vname = StringUtil::form("ecl[%d].", hslb.get_finid());
  callback.add(new FEE32Handler(vname + "reg_num_wdata",         callback, hslb, *this, 0xB0));
  callback.add(new FEE32Handler(vname + "sh_mask",               callback, hslb, *this, 0xB2));
  callback.add(new FEE8Handler(vname + "shaper_mask_low",       callback, hslb, *this, 0x20));
  callback.add(new FEE8Handler(vname + "shaper_mask_high",      callback, hslb, *this, 0x21));
  callback.add(new FEE8Handler(vname + "ttd_trg_rare_factor",   callback, hslb, *this, 0x38));
  callback.add(new FEE8Handler(vname + "ttd_trg_type",          callback, hslb, *this, 0x39));
  callback.add(new FEE8Handler(vname + "calib_ampl0_low",       callback, hslb, *this, 0x40));
  callback.add(new FEE8Handler(vname + "calib_ampl0_high",      callback, hslb, *this, 0x41));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_low",   callback, hslb, *this, 0x42));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_high",  callback, hslb, *this, 0x43));
  callback.add(new FEE8Handler(vname + "calib_delay0_low",      callback, hslb, *this, 0x44));
  callback.add(new FEE8Handler(vname + "calib_delay0_high",     callback, hslb, *this, 0x45));
  callback.add(new FEE8Handler(vname + "calib_delay_step_low",  callback, hslb, *this, 0x46));
  callback.add(new FEE8Handler(vname + "calib_delay_step_high", callback, hslb, *this, 0x47));
  callback.add(new FEE8Handler(vname + "calib_event_per_step",  callback, hslb, *this, 0x48));
}

void ECLFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  callback.log(LogFile::INFO, "Boot ecl[%d] with config %s",
               hslb.get_finid(), obj.getName().c_str());

  callback.log(LogFile::INFO, "Loading shaper firmware");
  loadShFw(callback, hslb, obj);
  callback.log(LogFile::INFO, "Loading shaper coefficients");
  loadShCoefs(callback, hslb, obj);
  callback.log(LogFile::INFO, "Loading shaper parameters");
  loadShPars(callback, hslb, obj);
}

void ECLFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  // TODO: Split into sh_data_before && sh_data_after
  const DBObjectList o_sh_datas(obj.getObjects("sh_data"));
  for (size_t i = 0; i < o_sh_datas.size(); i++) {
    const DBObject& o_sh_data(o_sh_datas[i]);
    unsigned int sh_num  = o_sh_data.getInt("sh_num"); // 0xFFF format
    unsigned int reg_num = o_sh_data.getInt("reg_num");
    unsigned int reg_wdata = o_sh_data.getInt("reg_wdata");
    rio_sh_wreg(callback, hslb, sh_num, reg_num, reg_wdata);
  }

  callback.log(LogFile::INFO, "Load ecl[%d] with config %s",
               hslb.get_finid(), obj.getName().c_str());

  hslb.writefee8(0x20, obj.getInt("shaper_mask_low"));
  hslb.writefee8(0x21, obj.getInt("shaper_mask_high"));

  callback.log(LogFile::INFO, "write fee-8 %x << %x", 0x38, obj.getInt("ttd_trg_rare_factor"));
  hslb.writefee8(0x38, obj.getInt("ttd_trg_rare_factor"));
  callback.log(LogFile::INFO, "read fee-8 %x >> %x", 0x38, hslb.readfee8(0x38));
  callback.log(LogFile::INFO, "write fee-8 %x << %x", 0x39, obj.getInt("ttd_trg_type"));
  hslb.writefee8(0x39, obj.getInt("ttd_trg_type"));
  callback.log(LogFile::INFO, "read fee-8 %x >> %x", 0x39, hslb.readfee8(0x39));

  hslb.writefee8(0x40, obj.getInt("calib_ampl0_low"));
  hslb.writefee8(0x41, obj.getInt("calib_ampl0_high"));
  hslb.writefee8(0x42, obj.getInt("calib_ampl_step_low"));
  hslb.writefee8(0x43, obj.getInt("calib_ampl_step_high"));
  hslb.writefee8(0x44, obj.getInt("calib_delay0_low"));
  hslb.writefee8(0x45, obj.getInt("calib_delay0_high"));
  hslb.writefee8(0x46, obj.getInt("calib_delay_step_low"));
  hslb.writefee8(0x47, obj.getInt("calib_delay_step_high"));
  hslb.writefee8(0x48, obj.getInt("calib_events_per_step"));


  callback.log(LogFile::INFO, "Loading shaper parameters: relays");
  writeRelays(callback, hslb, obj);

  rio_sh_wreg(callback, hslb, 0xFFF, 0x220, obj.getInt("trg2adc_data_end"));
  // 0 for normal run, 4 for testpulse
  rio_sh_wreg(callback, hslb, 0xFFF, 0x221, obj.getInt("run_type"));
  rio_sh_wreg(callback, hslb, 0xFFF, 0x223, obj.getInt("adc_data_len"));

  // Reset ADC on ShaperDSP (initialize time cycle)
  rio_sh_wreg(callback, hslb, 0xFFF, 0x800, 1);

  const DBObjectList o_reg30s(obj.getObjects("reg30"));
  for (size_t i = 0; i < o_reg30s.size(); i++) {
    const DBObject& o_reg30(o_reg30s[i]);
    unsigned int val = o_reg30.getInt("val");
    hslb.writefee8(0x30, val);
    callback.log(LogFile::INFO, "write fee-8 %x >> %x", 0x30, val);
  }
}

/********************** AUXILLARY FUNCTIONS **********************/

std::string ECLFEE::getValName(const DBObject& obj, const char* base_name,
                               int col, int sh, int ch)
{
  // Going from most detailed to least detailed.
  std::string vname;

  if (ch >= 1) {
    vname = StringUtil::form("col%d.sh%d.ch%d.%s", col, sh, ch, base_name);
    if (obj.hasValue(vname)) return vname;
  }

  vname = StringUtil::form("col%d.sh%d.%s",        col, sh, base_name);
  if (obj.hasValue(vname)) return vname;

  vname = StringUtil::form("col%d.%s",             col, base_name);
  if (obj.hasValue(vname)) return vname;

  // TODO: Make better DB table generation procedure so
  // barrel.* don't go to cpr600* tables and vice versa.
  if (col < 37) {
    vname = StringUtil::form("barrel.%s",          base_name);
  } else if (col < 45) {
    vname = StringUtil::form("fwd.%s",             base_name);
  } else {
    vname = StringUtil::form("bwd.%s",             base_name);
  }
  if (obj.hasValue(vname)) return vname;

  vname = StringUtil::form("all.%s",               base_name);
  if (obj.hasValue(vname)) return vname;

  return "";
}

/**
 * @return ECLCollector Id (1..52)
 */
int ECLFEE::getCprCollector(int finid, std::string dbobj_name)
{
  const std::string cpr_names[] = {
    // Barrel
    "cpr5001", "cpr5002", "cpr5003", "cpr5004", "cpr5005", "cpr5006",
    "cpr5007", "cpr5008", "cpr5009", "cpr5010", "cpr5011", "cpr5012",
    "cpr5013", "cpr5014", "cpr5015", "cpr5016", "cpr5017", "cpr5018",
    // Endcaps
    "cpr6001", "cpr6002", "cpr6003", "cpr6004",
    "cpr6005", "cpr6006", "cpr6007", "cpr6008"
  };
  const int cpr_count = sizeof(cpr_names) / sizeof(std::string);

  int cpr_id;

  for (cpr_id = 0; cpr_id < cpr_count; cpr_id++) {
    if (StringUtil::find(dbobj_name, cpr_names[cpr_id])) {
      break;
    }
  }

  if (cpr_id >= cpr_count) {
    // TODO: Throw exception
    // (something like out_of_range from [Abstract]DBObject)
    return -1;
  }

  if (cpr_id < 18) { // Barrel
    return 1  + cpr_id * 2 + finid;
  } else { // Endcaps
    return 37 + (cpr_id - 18) + finid * 8;
  }
}

int ECLFEE::getShapersInCollector(int col)
{
  // barrel
  if (col < 37) return 12;
  // fwd
  if (col < 45) return 10;
  // bwd
  return 8;
}

/********************** BOOT PROCEDURES **********************/

void ECLFEE::loadShFw(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  const int flash_addr = 0xA7000000;
  const int mask = 0xFFF;

  hslb.writefee32(0x00B0, flash_addr);
  hslb.writefee32(0x00B2, mask);
  // Issue 'boot shapers' command
  hslb.writefee32(0x00B8, 0x1);
}

void ECLFEE::loadShCoefs(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int col      = getCprCollector(hslb.get_finid(), obj.getName());
  int sh_count = getShapersInCollector(col);

  const unsigned int ecldsp_addr[12] = {
    0xA8000000, 0xAA000000, 0xAC000000, 0xAE000000,
    0xB0000000, 0xB2000000, 0xB4000000, 0xB6000000,
    0xB8000000, 0xBA000000, 0xBC000000, 0xBE000000
  };

  for (int sh = 0; sh < sh_count; sh++) {
    hslb.writefee32(0x00B0, ecldsp_addr[sh]);
    hslb.writefee32(0x00B2, 1 << sh);
    // Issue 'write DSP coefs from flash' command
    hslb.writefee32(0x00B8, 0x2);
  }
}

void ECLFEE::loadShPars(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  rio_sh_wreg(callback, hslb, 0xFFF, 0x502, obj.getInt("thread_af"));
  rio_sh_wreg(callback, hslb, 0xFFF, 0x500, obj.getInt("trbuf"));
  rio_sh_wreg(callback, hslb, 0xFFF, 0x501, obj.getInt("uthread_af"));

  rio_sh_wreg(callback, hslb, 0xFFF, 0x200, obj.getInt("shaper_proc_mask"));
  rio_sh_wreg(callback, hslb, 0xFFF, 0x210, obj.getInt("shaper_proc_num"));

  rio_sh_wreg(callback, hslb, 0xFFF, 0x208, obj.getInt("shaper_adc_mask"));
  rio_sh_wreg(callback, hslb, 0xFFF, 0x218, obj.getInt("shaper_adc_num"));

  // TODO: Add RELAY_G and connected values

  callback.log(LogFile::INFO, "Loading shaper parameters: ADC comp");
  writeADCComp(callback, hslb, obj);

  callback.log(LogFile::INFO, "Loading shaper parameters: potentiometer");
  writeAttnData(callback, hslb, obj);

  callback.log(LogFile::INFO, "Loading shaper parameters: thresholds");
  for (int thr_type = 0; thr_type < 3; thr_type++)
    writeThresholds(callback, hslb, obj, thr_type);
}

/********************** SHAPER OPERATIONS **********************/

void ECLFEE::rio_sh_wreg(RCCallback& callback, HSLB& hslb, unsigned int sh_num, unsigned int reg_num, unsigned int reg_wdata)
{
  unsigned int value = (reg_num << 16) + reg_wdata;
  callback.log(LogFile::INFO, "write 0x00B0<<0x%x", value);
  callback.log(LogFile::INFO, "write 0x00B2<<0x%x", sh_num);
  hslb.writefee32(0x00B0, value);
  hslb.writefee32(0x00B2, sh_num);
  hslb.writefee32(0x00B8, 0x4);
  callback.log(LogFile::INFO, "read 0x00B8>>0x%x", hslb.readfee32(0x00B8));
  callback.log(LogFile::INFO, "read 0x00B9>>0x%x", hslb.readfee32(0x00B9));
  // callback.log(LogFile::INFO, "read 0x00B8>>0x%x", hslb.readfee32(0x00B8));
}

void ECLFEE::writeToShaper(RCCallback& callback, HSLB& hslb, const DBObject& obj,
                           const char* base_name, int b8_cmd,
                           std::function<void (int mask, int vals[16])> write_reg,
                           bool value_per_channel)
{
  // ECL_MAX_SHAPERS = 12;
  // ECL_SHAPER_CHAN = 16;
  int vals[12][16];
  int mask     = 1;
  int col      = getCprCollector(hslb.get_finid(), obj.getName());
  int sh_count = getShapersInCollector(col);

  // This function tries to optimize writing to shapers:
  // If several consecutive shapers have same parameters, they are combined
  // into single data package with SHAPER_MASK == write_mask.
  int write_mask = 0;

  //== Reading values from the database

  for (int sh = 1; sh <= sh_count; sh++) {
    if (value_per_channel) {
      for (int ch = 1; ch <= 16; ch++) {
        vals[sh - 1][ch - 1] = obj.getInt(getValName(obj, base_name, col, sh, ch));
      }
    } else {
      vals[sh - 1][0] = obj.getInt(getValName(obj, base_name, col, sh, -1));
    }

    // Check if values of previous shaper are the same.

    if (sh == 1) {
      write_mask = mask;
    } else {
      if (value_per_channel) {
        int ch;
        for (ch = 1; ch <= 16; ch++) {
          if (vals[sh - 1][ch - 1] != vals[sh - 2][ch - 1]) break;
        }
        // If all elements are equal, add to write_mask:
        if (ch == 17) write_mask |= mask;
        else {
          write_reg(write_mask, vals[sh]);
          write_mask = mask;
        }
      } else {
        // If elements are equal, add to write_mask:
        if (vals[sh - 1][0] == vals[sh - 2][0]) write_mask |= mask;
        else {
          write_reg(write_mask, vals[sh]);
          write_mask = mask;
        }
      }
    }

    if (sh == sh_count) {
      write_reg(write_mask, vals[sh - 1]);
    }

    mask <<= 1;
  }

  if (b8_cmd > 0) {
    // Issue write command into ShaperDSPs
    hslb.writefee32(0x00B8, b8_cmd);
  }
}

/**
 * @param threshold_type
 *        0-hit, 1-low ampl, 2-skip ampl
 */
void ECLFEE::writeThresholds(RCCallback& callback, HSLB& hslb, const DBObject& obj,
                             int threshold_type)
{
  std::string base_name = StringUtil::form("thr%d", threshold_type);

  writeToShaper(callback, hslb, obj, base_name.c_str(), 0x40,
  [&](int mask, int vals[16]) {
    hslb.writefee32(0x00B2, mask);
    hslb.writefee32(0x00B4, threshold_type);

    int thr_addr = 0x00C0;
    for (int ch = 0; ch < 16; ch++) {
      // Converting low ampl threshold and skip threshold values from
      // ADC energy units to internal format used by shaper board.
      if (threshold_type != 0) vals[ch] += 128;

      hslb.writefee32(thr_addr, vals[ch]);
      thr_addr += 2;
    }
  }
               );
}

// Write potentiometer (aka attenuator) data.
void ECLFEE::writeAttnData(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  const int attn_data_reg = 0x30;
  const int attn_addr_reg = 0x31;

  writeToShaper(callback, hslb, obj, "pot", 0,
  [&](int mask, int vals[16]) {
    int pot_addr = 0x10;
    for (int ch = 0; ch < 16; ch++) {
      rio_sh_wreg(callback, hslb, mask, attn_data_reg, vals[ch]);
      rio_sh_wreg(callback, hslb, mask, attn_addr_reg, pot_addr);
      pot_addr++;
    }
  }
               );
}

void ECLFEE::writeADCComp(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  writeToShaper(callback, hslb, obj, "comp", 0,
  [&](int mask, int vals[16]) {
    int adc_comp_addr = 0x020;
    for (int ch = 0; ch < 16; ch++) {
      rio_sh_wreg(callback, hslb, mask, adc_comp_addr, vals[ch]);
      adc_comp_addr++;
    }
  }
               );
}

void ECLFEE::writeRelays(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  const int sh_relay_addr = 0x040;

  writeToShaper(callback, hslb, obj, "relay", 0,
  [&](int mask, int vals[16]) {
    rio_sh_wreg(callback, hslb, mask, sh_relay_addr, vals[0]);
    usleep(100000);
  },
  false
               );
}

/************************************************/

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}

