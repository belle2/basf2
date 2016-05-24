#ifndef _log_temps_h_
#define _log_temps_h_

namespace Belle2 {

  class HSLB;
  class RCCallback;

  namespace TOPDAQ {
    float convert_FPGA_temp(int temp);
    float convert_FPGA_voltage(int volt);
    float convert_FPGA_voltage_from_external_voltage_divider(int volt);
    float convert_FPGA_current(int volt);
    float convert_STTS751_temp(int temp);
    void scrod_status(HSLB& hslb, RCCallback& callback);
    void carrier_status(HSLB& hslb, RCCallback& callback, int carrier);
    void get_status(HSLB& hslb, RCCallback& callback);
    void add_scrod(HSLB& hslb, RCCallback& callback);
    void add_carrier(HSLB& hslb, RCCallback& callback, int carrier);
  }

}

#endif
