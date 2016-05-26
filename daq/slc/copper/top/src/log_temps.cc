#include <daq/slc/copper/top/log_temps.h>
#include <daq/slc/copper/top/b2l_top.h>
#include <daq/slc/copper/top/topreg.h>
#include <daq/slc/copper/top/TOPFEE.h>

#include <daq/slc/copper/HSLB.h>

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMVHandler.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

#define vRAW_low_margin  0.050 // safety margin on raw voltages
#define vRAW_high_margin 0.150 // safety margin on raw voltages
#define vRAW1_nominal    1.715 // nominal Voltage on Raw1 line
#define vRAW2_nominal    3.049 // nominal Voltage on Raw2 line
#define vRAW3_nominal    4.374 // nominal Voltage on Raw3 line

namespace Belle2 {

  namespace TOPDAQ {

    ////////////////////////////////////////////////////////////////////////////

    float convert_FPGA_temp(int temp)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // rest is calibration from Xilinx UG480
      const float tempCalib = 1.0 / 16.0 * (503.975 / 4096.0);
      const float tempOffset = 273.15;
      return temp * tempCalib - tempOffset;
    }

    ////////////////////////////////////////////////////////////////////////////

    float convert_FPGA_voltage(int volt)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // 1/2^12 is due to 12 bit ADC conversion
      // it's a mystery why the factor of 3 is necessary...
      const float voltageCalib = 1.0 / 16.0 * 3.0 / 4096.0;
      return volt * voltageCalib;
    }

    ////////////////////////////////////////////////////////////////////////////

    float convert_FPGA_voltage_from_external_voltage_divider(int volt)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // 8.15 is from voltage divider ratio 1k / (1k + 7.15k)
      // 1/2^12 is due to 12 bit ADC conversion
      const float voltageCalib = 1.0 / 16.0 * 8.15 / 4096.0;
      return volt * voltageCalib;
    }

    ////////////////////////////////////////////////////////////////////////////

    float convert_FPGA_current(int volt)
    {
      // 1/16 due to MSB-justification of 12-bit value
      // 8.15 is from voltage divider (1k & 7.15k) ratio
      // 121/500 and 240/1000 are from LT3055 and LT3086 current monitor output + resistors
      // 1/2^12 is due to 12 bit ADC conversion
      const float voltageCalib = 1.0 / 16.0 * 8.15 / 4096.0 / (121.0 / 500.0);
      return volt * voltageCalib;
    }

    ////////////////////////////////////////////////////////////////////////////

    float convert_STTS751_temp(int temp)
    {
      // calibration for extern STTS751 temp sensor (already divided by 16 in firmware)
      const float temp751Calib = 1.0 / 16.0;
      if ((temp & (1 < 15)) != 0)
        temp = temp - (1 << 16);
      return temp * temp751Calib;
    }

    ////////////////////////////////////////////////////////////////////////////

    void scrod_status(HSLB& hslb, RCCallback& callback)
    {
      // create a dictionary of values to return
      std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
      int scrod = b2l_read(hslb, SCROD_AxiVersion_UserID) / 256;
      callback.set(vname + "scrod", scrod);
      callback.set(vname + "fwVersion", b2l_read(hslb, SCROD_AxiVersion_FpgaVersion));
      callback.set(vname + "elfVersion", b2l_read(hslb, SCROD_PS_elfVersion));
      callback.set(vname + "trigMask", b2l_read(hslb, SCROD_AxiCommon_trigMask));

      // on-FPGA XADC readings
      callback.set(vname + "tempFPGA", convert_FPGA_temp(b2l_read(hslb, SCROD_XADC_Temperature)));
      float vRAW1 = convert_FPGA_voltage_from_external_voltage_divider(b2l_read(hslb, SCROD_XADC_V_RAW1));
      float vRAW2 = convert_FPGA_voltage_from_external_voltage_divider(b2l_read(hslb, SCROD_XADC_V_RAW2));
      float vRAW3 = convert_FPGA_voltage_from_external_voltage_divider(b2l_read(hslb, SCROD_XADC_V_RAW3));
      callback.set(vname + "vRAW1", vRAW1);
      callback.set(vname + "vRAW2", vRAW2);
      callback.set(vname + "vRAW3", vRAW3);
      callback.set(vname + "vINT", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_INT)));
      callback.set(vname + "vAUX", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_AUX)));
      callback.set(vname + "vBRAM", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_BRAM)));
      callback.set(vname + "vPINT", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_PINT)));
      callback.set(vname + "vPAUX", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_PAUX)));
      callback.set(vname + "vDDR", convert_FPGA_voltage(b2l_read(hslb, SCROD_XADC_V_DDR)));
      std::string sLive = "Running";
      int sLive1 = b2l_read(hslb, SCROD_PS_liveCounter);
      usleep(100000);
      int sLive2 = b2l_read(hslb, SCROD_PS_liveCounter);
      if (sLive1 == sLive2) {
        sLive = StringUtil::form("Stopped?  (0x%x)", sLive1);
      }
      callback.set(vname + "live", sLive);
      // external sensor readings
      callback.set(vname + "tempWall", 0);
      callback.set(vname + "tempASIC01", 0);
      callback.set(vname + "tempASIC23", 0);
      callback.set(vname + "humidity", 0);

      // log_temps_BS
      std::string str = StringUtil::form("(BS: %d, SCROD: %d)", hslb.get_finid(), scrod);
      if (vRAW1_nominal + vRAW_high_margin < vRAW1) {
        callback.log(LogFile::WARNING, "RAW1 voltage unnecessarily high " + str);
        if (vRAW1 < vRAW1_nominal - vRAW_low_margin) {
          callback.log(LogFile::ERROR, "RAW1 voltage too low! " + str);
        }
      }
      if (vRAW2_nominal + vRAW_high_margin < vRAW2) {
        callback.log(LogFile::WARNING, "RAW2 voltage unnecessarily high " + str);
        if (vRAW2 < vRAW2_nominal - vRAW_low_margin) {
          callback.log(LogFile::ERROR, "RAW2 voltage too low! " + str);
        }
      }
      if (vRAW3_nominal + vRAW_high_margin < vRAW3) {
        callback.log(LogFile::WARNING, "RAW3 voltage unnecessarily high " + str);
        if (vRAW3 < vRAW3_nominal - vRAW_low_margin) {
          callback.log(LogFile::ERROR, "RAW3 voltage too low!" + str);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////

    void carrier_status(HSLB& hslb, RCCallback& callback, int carrier)
    {
      // create a dictionary of values to return
      std::string vname = StringUtil::form("top[%d].carrier[%d].", hslb.get_finid(), carrier);
      callback.set(vname + "fwVersion", b2l_read(hslb, CARRIER_AxiVersion_FpgaVersion, carrier));
      callback.set(vname + "elfVersion", b2l_read(hslb, CARRIER_PS_version, carrier));
      callback.set(vname + "trigMask", b2l_read(hslb, CARRIER_AxiCommon_trigMask, carrier));
      callback.set(vname + "temp2", convert_STTS751_temp(b2l_read(hslb, CARRIER_PS_temp, carrier)));

      // on-FPGA XADC readings
      callback.set(vname + "tempFPGA", convert_FPGA_temp(b2l_read(hslb, CARRIER_XADC_Temperature, carrier)));
      callback.set(vname + "vRAW1", 0);
      callback.set(vname + "vRAW2", 0);
      callback.set(vname + "vRAW3", 0);
      callback.set(vname + "vINT", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_INT, carrier)));
      callback.set(vname + "vAUX", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_AUX, carrier)));
      callback.set(vname + "vBRAM", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_BRAM, carrier)));
      callback.set(vname + "vPINT", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_PINT, carrier)));
      callback.set(vname + "vPAUX", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_PAUX, carrier)));
      callback.set(vname + "vDDR", convert_FPGA_voltage(b2l_read(hslb, CARRIER_XADC_V_DDR , carrier)));
      std::string cLive = "Running";
      int cLive1 = b2l_read(hslb, CARRIER_PS_counter, carrier);
      usleep(100000);
      int cLive2 = b2l_read(hslb, CARRIER_PS_counter, carrier);
      if (cLive1 == cLive2) {
        cLive = StringUtil::form("Stopped?  (0x%x)", cLive1);
      }
      callback.set(vname + "live", cLive);
      // external sensor readings
      callback.set(vname + "tempWall", convert_FPGA_temp(b2l_read(hslb, CARRIER_XADC_Temperature, carrier)));
      callback.set(vname + "tempASIC01", 0);
      callback.set(vname + "tempASIC23", 0);
      callback.set(vname + "humidity", 0);
    }

    ////////////////////////////////////////////////////////////////////////////

    void get_status(HSLB& hslb, RCCallback& callback)
    {
      scrod_status(hslb, callback);
      for (int carrier = 0; carrier < 4; carrier++) {
        carrier_status(hslb, callback, carrier);
      }
    }

    ////////////////////////////////////////////////////////////////////////////

    void add_scrod(HSLB& hslb, RCCallback& callback)
    {
      // create a dictionary of values to return
      std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
      callback.add(new NSMVHandlerInt(vname + "scrod", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "fwVersion", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "elfVersion", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "trigMask", true, false, 0));
      // on-FPGA XADC readings
      callback.add(new NSMVHandlerFloat(vname + "tempFPGA", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW1", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW2", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW3", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vINT", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vAUX", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vBRAM", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vPINT", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vPAUX", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vDDR", true, false, 0));
      callback.add(new NSMVHandlerText(vname + "live", true, false, ""));
      // external sensor readings
      callback.add(new NSMVHandlerFloat(vname + "tempWall", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "tempASIC01", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "tempASIC23", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "humidity", true, false, 0));

    }

    ////////////////////////////////////////////////////////////////////////////

    void add_carrier(HSLB& hslb, RCCallback& callback, int carrier)
    {
      // create a dictionary of values to return
      std::string vname = StringUtil::form("top[%d].carrier[%d].", hslb.get_finid(), carrier);
      callback.add(new NSMVHandlerInt(vname + "fwVersion", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "elfVersion", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "trigMask", true, false, 0));
      callback.add(new NSMVHandlerInt(vname + "temp2", true, false, 0));

      // on-FPGA XADC readings
      callback.add(new NSMVHandlerFloat(vname + "tempFPGA", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW1", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW2", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vRAW3", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vINT", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vAUX", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vBRAM", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vPINT", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vPAUX", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "vDDR", true, false, 0));
      callback.add(new NSMVHandlerText(vname + "live", true, false, ""));
      // external sensor readings
      callback.add(new NSMVHandlerFloat(vname + "tempWall", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "tempASIC01", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "tempASIC23", true, false, 0));
      callback.add(new NSMVHandlerFloat(vname + "humidity", true, false, 0));
    }

    ////////////////////////////////////////////////////////////////////////////

  }

}
