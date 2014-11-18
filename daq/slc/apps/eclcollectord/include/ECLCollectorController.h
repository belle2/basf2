#ifndef Belle2_ECLCollectorController_h
#define Belle2_ECLCollectorController_h

#include "daq/slc/apps/eclcollectord/ECLShaperController.h"

#include <daq/slc/base/IOException.h>

#include <string>
#include <vector>

namespace Belle2 {

  class ECLCollectorController {
  public:
    class ShaperReg {
    public:
      ShaperReg() {}
      ShaperReg(int adr, int val, const char* name) {
        this->adr = adr;
        this->val = val;
        this->name = name;
      }
    public:
      int adr;
      int val;
      std::string name;
    };

  public:
    ECLCollectorController() {}
    ~ECLCollectorController() {}

  public:
    bool boot(const std::string& config) throw(IOException);
    bool init(int mode) throw(IOException);

  public:
    //bool status(int status_type = 0);

  private:
    std::vector<ECLShaperController> m_shaper_v;

  };

}

#endif
