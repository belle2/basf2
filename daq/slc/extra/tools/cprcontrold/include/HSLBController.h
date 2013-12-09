#ifndef _Belle2_HSLBController_hh
#define _Belle2_HSLBController_hh

#include "daq/slc/base/DataObject.h"

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
    HSLBController();
    ~HSLBController() throw();

  public:
    void setBootFirmware(bool boot_firm) { _boot_firm = boot_firm; }

  public:
    bool boot(int slot = -1, DataObject* hslb = NULL) throw();
    bool reset() throw();
    bool load() throw();

  private:
    int _slot;
    DataObject* _hslb;
    Mgt_t* _mgt;
    std::vector<HSLBRegister> _reg_v;
    bool _boot_firm;

  };

}

#endif
