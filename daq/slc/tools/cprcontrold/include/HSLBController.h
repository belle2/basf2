#ifndef _Belle2_HSLBController_hh
#define _Belle2_HSLBController_hh

#include "base/DataObject.h"

#include <mgt/mgt.h>

namespace Belle2 {

  struct HSLBRegister {
  public:
    std::string name;
    int address;
    int size;
  };

  class HSLBController {

  public:
    HSLBController(int slot = -1, DataObject* hslb = NULL);
    ~HSLBController() throw();

  public:
    void setHSLB(int slot, DataObject* hslb) {
      _slot = slot;
      _hslb = hslb;
    }

  public:
    bool boot() throw();
    bool reset() throw();
    bool load() throw();

  private:
    int _slot;
    DataObject* _hslb;
    Mgt_t* _mgt;
    std::vector<HSLBRegister> _reg_v;

  };

}

#endif
