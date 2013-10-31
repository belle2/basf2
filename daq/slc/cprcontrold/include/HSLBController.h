#ifndef _Belle2_HSLBController_hh
#define _Belle2_HSLBController_hh

#include <base/HSLB.h>

#include <mgt.h>

namespace Belle2 {

  class HSLBController {

  public:
    HSLBController(int slot = -1, HSLB* hslb = NULL);
    ~HSLBController() throw();

  public:
    void setHSLB(int slot, HSLB* hslb) {
      _slot = slot;
      _hslb = hslb;
    }

  public:
    bool boot() throw();
    bool reset() throw();
    bool load() throw();
    //bool start() throw();
    //bool stop() throw();

  private:
    int _slot;
    HSLB* _hslb;
    Mgt_t* _mgt;

  };

}

#endif
