#include <daq/slc/copper/top/b2l_top.h>
#include <daq/slc/copper/top/topreg.h>
#include <daq/slc/copper/top/TOPFEE.h>

#include <daq/slc/copper/HSLB.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

namespace Belle2 {

  namespace TOPDAQ {

    int b2l_fulladdr(int reg, int carrier, int asic)
    {
      return (reg + carrier * 0x2000 + asic * 0x400);
    }

    int b2l_read(HSLB& hslb, int reg, int carrier, int asic, int read_retry)
    {
      if (reg == SCROD_UNDEFINED || reg == CARRIER_UNDEFINED) {
        return -1;
      }
      int addr = b2l_fulladdr(reg, carrier, asic);
      unsigned int ret = hslb.readfee32(addr);
      for (int i = 0; ret == TIMEOUT && i < read_retry; i++) {
        ret = hslb.readfee32(addr);
      }
      LogFile::debug("addr=0x%x, value=0x%x", addr, ret);
      if (ret == 0xDEADBEEF) {
        throw (HSLBHandlerException(" DEADBEEF returned (addr=0x%x)"));
      }
      return ret;
    }
    int b2l_write(HSLB& hslb, int reg, unsigned int value,
                  int carrier, int asic, int write_retry)
    {
      if (reg == SCROD_UNDEFINED || reg == CARRIER_UNDEFINED) {
        return -1;
      }
      int addr = b2l_fulladdr(reg, carrier, asic);
      hslb.writefee32(addr, value);
      unsigned int ret = 0;
      for (int i = 0; i < write_retry; i++) {
        ret = hslb.readfee32(addr);
        if (ret != value) {
          hslb.writefee32(addr, value);
        } else {
          return ret;
        }
        usleep(1000);
      }
      return ret;
    }

    /*
    double convert_FPGA_temp(int temp)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // rest is calibration from Xilinx UG480
      const double tempCalib = 1.0/16.0 * (503.975/4096.0);
      const double tempOffset = 273.15;
      return temp * tempCalib - tempOffset;
    }

    double convert_FPGA_voltage(int volt)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // rest is calibration from Xilinx UG480
      const double voltageCalib = 1.0/16.0 * 8.15/4096.0;
      return volt * voltageCalib;
    }

    double convert_STTS751_temp(int temp)
    {
      // calibration for extern STTS751 temp sensor (already divided by 16 in firmware)
      const double temp751Calib = 1.0/16.0;
      // convert 16-bit two's compliment value to an int
      if ((temp & (1 < 15)) != 0) {
    temp = temp - (1 << 16);
      }
      return temp * temp751Calib;
    }

    void scrod_status(HSLB& hslb, RCCallback& callback)
    {
      const int id = hslb.get_finid();
      // print SCROD information
      int sFwVer = b2l_read(hslb, SCROD_AxiVersion_FpgaVersion, id);
      int sSwVer = b2l_read(hslb, SCROD_PS_elfVersion, id);
      int sTrig  = b2l_read(hslb, SCROD_AxiCommon_trigMask, id);
      int sTemp  = b2l_read(hslb, SCROD_XADC_Temperature, id);
      int Raw1   = b2l_read(hslb, SCROD_XADC_V_RAW1, id);
      int Raw2   = b2l_read(hslb, SCROD_XADC_V_RAW2, id);
      int Raw3   = b2l_read(hslb, SCROD_XADC_V_RAW3, id);
      std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
      callback.set(vname + "fwver", sFwVer);
      callback.set(vname + "swver", sSwVer);
      callback.set(vname + "trig", sTrig);
      //callback.set(vname + "temp", sTemp);
      //callback.set(vname + "raw1", Raw1);
      //callback.set(vname + "raw2", Raw2);
      //callback.set(vname + "raw3", Raw3);
      double Raw1_n = convert_FPGA_voltage(Raw1);
      double Raw2_n = convert_FPGA_voltage(Raw2);
      double Raw3_n = convert_FPGA_voltage(Raw3);
      callback.set(vname + "raw1", (float)Raw1_n);
      callback.set(vname + "raw2", (float)Raw2_n);
      callback.set(vname + "raw3", (float)Raw3_n);
      double sTemp_n = convert_FPGA_temp(sTemp);
      callback.set(vname + "temp", (float)sTemp_n);

      std::string sLive = "Running";
      int sLive1 = b2l_read(hslb, SCROD_PS_liveCounter);
      usleep(100000);
      int sLive2 = b2l_read(hslb, SCROD_PS_liveCounter);
      if (sLive1 == sLive2) {
    sLive = StringUtil::form("Stopped?  (%0x%x)", sLive1);
      }

      LogFile::debug("RawVoltages: %1.3f V %1.3f V %1.3f V", Raw1_n, Raw2_n, Raw3_n);
      LogFile::debug("           FPGA_ver    ARM_ver     trigMask    Temp(s)             ARM_status"
         "SCROD:     %d  %d  %d  %d            %s",
     sFwVer, sSwVer, sTrig, sTemp, sLive.c_str());
      if (Raw1_n<(Raw1_normal+Raw_margin)) {
    callback.log(LogFile::WARNING,
         "Raw1 TOO SMALL!!! Measured Value: %1.3f V, Ideal value: %1.3f",
         Raw1_n, Raw1_normal);
      }

      if (Raw2_n<(Raw2_normal+Raw_margin)) {
    callback.log(LogFile::WARNING,
         "Raw1 TOO SMALL!!! Measured Value: %1.3f V, Ideal value: %1.3f",
         Raw2_n, Raw2_normal);
      }

      if (Raw3_n<(Raw3_normal+Raw_margin)) {
    callback.log(LogFile::WARNING,
         "Raw1 TOO SMALL!!! Measured Value: %1.3f V, Ideal value: %1.3f",
         Raw3_n, Raw3_normal);
      }
    }

    void carrier_status(HSLB& hslb, RCCallback& callback, int carrier)
    {
      // print carrier information
      int cFwVer = b2l_read(hslb, CARRIER_AxiVersion_FpgaVersion, carrier);
      int cSwVer = b2l_read(hslb, CARRIER_PS_version, carrier);
      int cTrig  = b2l_read(hslb, CARRIER_AxiCommon_trigMask, carrier);
      int cTemp  = b2l_read(hslb, CARRIER_XADC_Temperature, carrier);
      int cTemp2 = b2l_read(hslb, CARRIER_PS_temp, carrier);
      std::string vname = StringUtil::form("top[%d].carrier[%d].", hslb.get_finid(), carrier);
      callback.set(vname + "fwver", cFwVer);
      callback.set(vname + "swver", cSwVer);
      callback.set(vname + "trig", cTrig);
      //callback.set(vname + "temp", cTemp);
      //callback.set(vname + "temp2", cTemp2);

      double cTemp_n  = convert_FPGA_temp(cTemp);
      double cTemp2_n = convert_STTS751_temp(cTemp2);
      callback.set(vname + "temp", (float)cTemp_n);
      callback.set(vname + "temp2", (float)cTemp2_n);

      std::string cLive = "Running";
      int cLive1 = b2l_read(hslb, CARRIER_PS_counter, carrier);
      usleep(100000);
      int cLive2 = b2l_read(hslb, CARRIER_PS_counter, carrier);
      if (cLive1 == cLive2) {
    cLive = StringUtil::form("Stopped?  (0x%x)", cLive1);
      }

      LogFile::debug("Carrier %d: %d  %d  %d  %5.2f C  %5.2f C  %s",
         carrier, cFwVer, cSwVer, cTrig,
         cTemp_n, cTemp2_n, cLive.c_str());
    }

    void get_status(HSLB& hslb, RCCallback& callback)
    {
      scrod_status(hslb, callback);
      for (int i = 0; i < 4; i++) {
    carrier_status(hslb, callback, i);
      }
    }
    */

  }

}

