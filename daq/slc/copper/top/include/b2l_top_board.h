#ifndef _b2l_top_board_h_
#define _b2l_top_board_h_

namespace Belle2 {

  class HSLB;
  class RCCallback;

  namespace TOPDAQ {

    void config1boardstack(HSLB& hslb, RCCallback& callback);
    void config1carrier(HSLB& hslb, RCCallback& callback, int carrier);

  }
}

#endif
