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

// TODO: Optimize writeToShaper via intelligent masking
// (Note: probably won't be necessary)

ECLFEE::ECLFEE() : FEE("ecl")
{
}

void ECLFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& /*obj*/)
{
  // See detailed description of ECL registers @ https://confluence.desy.de/display/~remnev/ECL+Registers

  std::string vname = StringUtil::form("ecl[%d].", hslb.get_finid());

  //== Shaper data packet
  // (In case register writing operation)
  // high 2 bytes -- shaper register address
  // low  2 bytes -- shaper register value
  callback.add(new FEE32Handler(vname + "sh_packet",            callback, hslb, *this, 0xB0));
  callback.add(new FEE32Handler(vname + "sh_mask",              callback, hslb, *this, 0xB2));
  //== Threshold type
  // 0-hit, 1-skip ampl, 2-low ampl, 3-ADC always
  callback.add(new FEE32Handler(vname + "thr_type",             callback, hslb, *this, 0xB4));
  //== Shaper control register
  // !!! Sets its content to 0 when operation is finished.
  //
  // 0x1     - boot shapers with firmware stored in flash memory of ECL Collector
  //           at 4-byte address set in 0x00B0.
  // 0x2     - write DSP coefficients from flash memory of ECL Collector
  //           using 4-byte address set in 0x00B0.
  // 0x4     - write to shaper register, for all shapers in `0x00B2`.
  //           2 high bytes of 0x00B0 contain register address, 2 low bytes
  //           contain value to be written.
  //           If shaper mask at 0x00B2 contains non-existent shapers (max
  //           shaper mask is 0xFFF for barrel (cpr50__:a,b),
  //           0x3FF for forward (cpr600_:a), 0x0FF for backward (cpr600_:b)),
  //           operation will fail without writing anything at all!!!
  // 0x40    - init write thresholds operations. Uses 0x00B4.
  //           See threshold registers per channel below.
  // 0x200   - read from shaper register with address set at `0x00B0`,
  //           for all shapers in `0x00B2`.
  //           Results are saved in registers 0xC0, 0xC2... 0xD6
  callback.add(new FEE32Handler(vname + "sh_ctrl",              callback, hslb, *this, 0xB8));
  //== Operation status, 0 == success.
  callback.add(new FEE32Handler(vname + "sh_retval",            callback, hslb, *this, 0xB9));
  callback.add(new FEE8Handler(vname + "shaper_mask_low",       callback, hslb, *this, 0x20));
  callback.add(new FEE8Handler(vname + "shaper_mask_high",      callback, hslb, *this, 0x21));
  //== Register #30
  // bit 0 - output to rio
  // bit 1 - round-robin rare event mode
  // bit 2 - reset of calibration counter
  // bit 3 - ttd busy
  // bit 4 - compressing
  // bit 5 - packing before send
  // bit 6 - disable random trigger
  callback.add(new FEE8Handler(vname + "reg30",                 callback, hslb, *this, 0x30));
  //== Generator relay.
  // Register to control two generator relays.
  //  - 0 - relays off, 1 - first relay on, 2 - second relay on, 3 - both relays on
  //  One relay decreases the signal by a factor of 8.
  callback.add(new FEE8Handler(vname + "relay_g",               callback, hslb, *this, 0x31));
  //== Rare factor
  // Write waveform every n * 10^P event (Binary format: PPPn nnnn)
  callback.add(new FEE8Handler(vname + "ttd_trg_rare_factor",   callback, hslb, *this, 0x38));
  //== Trigger type
  // data to write for each
  // normal event low byte
  // wave form event second byte
  //  1-dsp data 2-wave form data
  callback.add(new FEE8Handler(vname + "ttd_trg_type",          callback, hslb, *this, 0x39));
  //== Pulse height
  // Set in DAQ units, 1 DAQ unit ~= 10 ADC energy units ~= 0.5 MeV
  callback.add(new FEE8Handler(vname + "calib_ampl0_low",       callback, hslb, *this, 0x40));
  callback.add(new FEE8Handler(vname + "calib_ampl0_high",      callback, hslb, *this, 0x41));
  //TODO: more documentation.
  callback.add(new FEE8Handler(vname + "calib_ampl_step_low",   callback, hslb, *this, 0x42));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_high",  callback, hslb, *this, 0x43));
  callback.add(new FEE8Handler(vname + "calib_delay0_low",      callback, hslb, *this, 0x44));
  callback.add(new FEE8Handler(vname + "calib_delay0_high",     callback, hslb, *this, 0x45));
  callback.add(new FEE8Handler(vname + "calib_delay_step_low",  callback, hslb, *this, 0x46));
  callback.add(new FEE8Handler(vname + "calib_delay_step_high", callback, hslb, *this, 0x47));
  callback.add(new FEE8Handler(vname + "calib_event_per_step",  callback, hslb, *this, 0x48));
  //== Waveform saving mode
  // 0x00 -- normal run, don't save waveforms
  // 0x20 -- save waveforms using ttd_trg_rare_factor
  callback.add(new FEE32Handler(vname + "calib_mode_num",       callback, hslb, *this, 0x80));
  //== Data values (16 channels), used in threshold setting and in shaper register readout.
  int addr;
  for (int i = 0; i < 16; i++) {
    std::string thrname = StringUtil::form("data_reg%d", i + 1);
    addr = 0xC0 + i;
    callback.add(new FEE32Handler(vname + thrname,              callback, hslb, *this, addr));
  }
}

void ECLFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void ECLFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  int slot = hslb.get_finid();

  callback.log(LogFile::INFO, "Load ecl[%d] with config %s",
               slot, obj.getName().c_str());

  if (obj.hasObject("sh_data_before")) {
    const DBObjectList o_sh_datas(obj.getObjects("sh_data_before"));
    for (size_t i = 0; i < o_sh_datas.size(); i++) {
      const DBObject& o_sh_data(o_sh_datas[i]);
      unsigned int sh_num    = o_sh_data.getInt("mask"); // 0xFFF format
      unsigned int reg_num   = o_sh_data.getInt("addr");
      unsigned int reg_wdata = o_sh_data.getInt("data");
      // Print description for custom register into log
      callback.log(LogFile::INFO, "ecl[%d]: write shaper reg0x%x val0x%x (%s)",
                   slot, reg_num, reg_wdata,
                   o_sh_data.getText("info").c_str());
      rio_sh_wreg(callback, hslb, sh_num, reg_num, reg_wdata);
    }
  }

  hslb.writefee8(0x20, obj.getInt("shaper_mask_low"));
  hslb.writefee8(0x21, obj.getInt("shaper_mask_high"));

  callback.log(LogFile::INFO, "write fee-8 %x << %x", 0x38, obj.getInt("ttd_trg_rare_factor"));
  hslb.writefee8(0x38, obj.getInt("ttd_trg_rare_factor"));
  callback.log(LogFile::INFO, "read  fee-8 %x >> %x", 0x38, hslb.readfee8(0x38));
  callback.log(LogFile::INFO, "write fee-8 %x << %x", 0x39, obj.getInt("ttd_trg_type"));
  hslb.writefee8(0x39, obj.getInt("ttd_trg_type"));
  callback.log(LogFile::INFO, "read  fee-8 %x >> %x", 0x39, hslb.readfee8(0x39));

  hslb.writefee8(0x40, obj.getInt("calib_ampl0_low"));
  hslb.writefee8(0x41, obj.getInt("calib_ampl0_high"));
  hslb.writefee8(0x42, obj.getInt("calib_ampl_step_low"));
  hslb.writefee8(0x43, obj.getInt("calib_ampl_step_high"));
  hslb.writefee8(0x44, obj.getInt("calib_delay0_low"));
  hslb.writefee8(0x45, obj.getInt("calib_delay0_high"));
  hslb.writefee8(0x46, obj.getInt("calib_delay_step_low"));
  hslb.writefee8(0x47, obj.getInt("calib_delay_step_high"));
  hslb.writefee8(0x48, obj.getInt("calib_events_per_step"));

  const DBObjectList o_reg30s(obj.getObjects("reg30"));
  for (size_t i = 0; i < o_reg30s.size(); i++) {
    const DBObject& o_reg30(o_reg30s[i]);
    unsigned int val = o_reg30.getInt("val");
    hslb.writefee8(0x30, val);
    callback.log(LogFile::INFO, "write fee-8 %x >> %x", 0x30, val);
  }

  if (obj.hasObject("col_data")) {
    const DBObjectList o_col_datas(obj.getObjects("col_data"));
    for (size_t i = 0; i < o_col_datas.size(); i++) {
      const DBObject& o_col_data(o_col_datas[i]);
      unsigned int reg_num   = o_col_data.getInt("addr");
      unsigned int reg_wdata = o_col_data.getInt("data");
      std::string type       = o_col_data.getText("type");
      // Print description for custom register into log
      callback.log(LogFile::INFO, "ecl[%d]: write%s reg0x%x val0x%x (%s)",
                   slot, type.c_str(), reg_num, reg_wdata,
                   o_col_data.getText("info").c_str());
      if (type == "fee8") {
        hslb.writefee8(reg_num, reg_wdata);
      } else if (type == "fee32") {
        hslb.writefee32(reg_num, reg_wdata);
      }
    }
  }

  if (obj.hasObject("sh_data_after")) {
    const DBObjectList o_sh_datas(obj.getObjects("sh_data_after"));
    for (size_t i = 0; i < o_sh_datas.size(); i++) {
      const DBObject& o_sh_data(o_sh_datas[i]);
      unsigned int sh_num    = o_sh_data.getInt("mask"); // 0xFFF format
      unsigned int reg_num   = o_sh_data.getInt("addr");
      unsigned int reg_wdata = o_sh_data.getInt("data");
      // Print description for custom register into log
      callback.log(LogFile::INFO, "ecl[%d]: write shaper reg0x%x val0x%x (%s)",
                   slot, reg_num, reg_wdata,
                   o_sh_data.getText("info").c_str());
      rio_sh_wreg(callback, hslb, sh_num, reg_num, reg_wdata);
    }
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

/************************************************/

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}

