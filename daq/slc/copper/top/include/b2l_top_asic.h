#ifndef _b2l_top_asic_h_
#define _b2l_top_asic_h_

namespace Belle2 {

  class HSLB;
  class RCCallback;
  namespace TOPDAQ {

    void write_irs_reg(HSLB& hslb, RCCallback& callback, int carNum, int asicNum,
                       int address, int value, int irsxwrite_retry = 5);
    void verify_dll_asic(HSLB& hslb, RCCallback& callback, int carrier, int asic);
    void config1asic(HSLB& hslb, RCCallback& callback, int carrier, int asic);
    void check1asicConfig(HSLB& hslb, RCCallback& callback, int carrier, int asic);

  }

}

#endif
