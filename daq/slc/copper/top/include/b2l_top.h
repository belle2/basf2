#ifndef _b2l_top_h_
#define _b2l_top_h_

namespace Belle2 {

  class HSLB;
  class RCCallback;

  namespace TOPDAQ {

    int b2l_fulladdr(int reg, int carrier, int asic = 0);
    int b2l_read(HSLB& hslb, int reg, int carrier = 0, int asic = 0, int read_retry = 3);
    int b2l_write(HSLB& hslb, int reg, unsigned int value,
                  int carrier = 0, int asic = 0, int write_retry = 25);
    /*
    void scrod_status(HSLB& hslb, RCCallback& callback);
    void carrier_status(HSLB& hslb, RCCallback& callback, int carrier);
    void get_status(HSLB& hslb, RCCallback& callback);
    double convert_FPGA_temp(int temp);
    double convert_FPGA_voltage(int volt);
    double convert_STTS751_temp(int temp);
    */
  }

}

#endif
