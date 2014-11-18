#ifndef _Belle2_ECLFEEController_h
#define _Belle2_ECLFEEController_h

#include <daq/slc/base/IOException.h>

#include <daq/slc/apps/cprcontrold/FEEController.h>

namespace Belle2 {

  class ECLFEEController : public FEEController {
    class Reg {
    public:
      Reg() {}
      Reg(int adr, int val, const char* name) {
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
    ECLFEEController();
    virtual ~ECLFEEController() throw() {}

  public:
    virtual bool boot(HSLBController& hslb, FEEConfig& conf) throw(IOException);
    virtual bool load(HSLBController& hslb, FEEConfig& conf) throw(IOException);
    virtual bool monitor(HSLBController& hslb, FEEConfig& conf) throw(IOException);

  private:
    int rio_reg_io(HSLBController& hslb, const char* opt,
                   unsigned int reg_num, unsigned int reg_wdata = 0) throw(IOException);
  };

}

#endif
