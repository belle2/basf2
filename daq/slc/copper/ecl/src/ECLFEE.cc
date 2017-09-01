#include "daq/slc/copper/ecl/ECLFEE.h"
#include "daq/slc/copper/FEEHandler.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/hsreg.h>

#include <unistd.h>

const int HSLB_FIRMWARE_VERSION = 0xA;
const int HSLB_HARDWARE_VERSION = 0xA;

using namespace Belle2;

ECLFEE::ECLFEE() : FEE("ecl")
{
}

void ECLFEE::init(RCCallback& callback, HSLB& hslb, const DBObject& /*obj*/)
{
  std::string vname = StringUtil::form("ecl[%d].", hslb.get_finid());
  callback.add(new FEE32Handler(vname + "reg_num_wdata", callback, hslb, *this, 0xB0));
  callback.add(new FEE32Handler(vname + "sh_mask", callback, hslb, *this, 0xB2));
  callback.add(new FEE8Handler(vname + "shaper_mask_low", callback, hslb, *this, 0x20));
  callback.add(new FEE8Handler(vname + "shaper_mask_high", callback, hslb, *this, 0x21));
  callback.add(new FEE8Handler(vname + "ttd_trg_rare_factor", callback, hslb, *this, 0x38));
  callback.add(new FEE8Handler(vname + "ttd_trg_type", callback, hslb, *this, 0x39));
  callback.add(new FEE8Handler(vname + "calib_ampl0_low", callback, hslb, *this, 0x40));
  callback.add(new FEE8Handler(vname + "calib_ampl0_high", callback, hslb, *this, 0x41));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_low", callback, hslb, *this, 0x42));
  callback.add(new FEE8Handler(vname + "calib_ampl_step_high", callback, hslb, *this, 0x43));
  callback.add(new FEE8Handler(vname + "calib_delay0_low", callback, hslb, *this, 0x44));
  callback.add(new FEE8Handler(vname + "calib_delay0_high", callback, hslb, *this, 0x45));
  callback.add(new FEE8Handler(vname + "calib_delay_step_low", callback, hslb, *this, 0x46));
  callback.add(new FEE8Handler(vname + "calib_delay_step_high", callback, hslb, *this, 0x47));
  callback.add(new FEE8Handler(vname + "calib_event_per_step", callback, hslb, *this, 0x48));
}

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
  callback.log(LogFile::INFO, "read 0x00B8>>0x%x", hslb.readfee32(0x00B8));
}

void ECLFEE::boot(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
}

void ECLFEE::load(RCCallback& callback, HSLB& hslb, const DBObject& obj)
{
  callback.log(LogFile::INFO, "Load ECL config");
  const DBObjectList o_sh_datas(obj.getObjects("sh_data"));
  for (size_t i = 0; i < o_sh_datas.size(); i++) {
    const DBObject& o_sh_data(o_sh_datas[i]);
    unsigned int sh_num  = o_sh_data.getInt("sh_num");//0xFFF;
    unsigned int reg_num = o_sh_data.getInt("reg_num");
    unsigned int reg_wdata = o_sh_data.getInt("reg_wdata");
    rio_sh_wreg(callback, hslb, sh_num, reg_num, reg_wdata);
  }
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

  hslb.writefee8(0x45, obj.getInt("calib_delay0_low"));
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
  //hslb.writefee8(0x30, 0x03D);
  //hslb.writefee8(0x30, 0x039);
}

extern "C" {
  void* getECLFEE()
  {
    return new Belle2::ECLFEE();
  }
}
