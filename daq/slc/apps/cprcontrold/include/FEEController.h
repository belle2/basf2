#ifndef _Belle2_FEEController_h
#define _Belle2_FEEController_h

#include <daq/slc/database/ConfigObject.h>
#include <daq/slc/base/IOException.h>

#include <daq/slc/apps/cprcontrold/FEEConfig.h>

#include <mgt/hslb_info.h>

namespace Belle2 {

  class FEEController {

  public:
    static const char* getFEEType(int type);

  public:
    FEEController() {}
    ~FEEController() throw() {}

  public:
    bool open(int i, const FEEConfig& conf) throw();
    bool close() throw();
    bool load(int i, const FEEConfig& conf) throw();
    bool monitor(int i, const FEEConfig& conf) throw();
    const hslb_info& getInfo(int i) const throw() {
      return m_info[i];
    }
    bool isError(int i) throw();
    bool isBelle2LinkDown(int i) throw();
    bool isCOPPERFifoFull(int i) throw();
    bool isCOPPERLengthFifoFull(int i) throw();
    bool isHSLBFifoFull(int i) throw();
    bool isHSLBCRCError(int i) throw();

  protected:
    bool bootHSLB(int i, const FEEConfig& conf) throw();
    virtual bool bootFEE(int i, const FEEConfig& conf)
    throw() = 0;
    virtual bool loadConfig(int i, const FEEConfig& conf)
    throw() = 0;
    virtual bool readStat(int i, const FEEConfig& conf)
    throw() = 0;

  private:
    hslb_info m_info[4];

  };

}

#endif
