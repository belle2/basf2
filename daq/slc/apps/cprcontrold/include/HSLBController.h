#ifndef _Belle2_HSLBController_h
#define _Belle2_HSLBController_h

#include <daq/slc/database/ConfigObject.h>
#include <daq/slc/base/IOException.h>

#include <daq/slc/apps/cprcontrold/FEEConfig.h>

#include <mgt/hslb_info.h>

namespace Belle2 {

  class HSLBController {

  public:
    static const char* getFEEType(int type);

  public:
    HSLBController() { m_hslb.fd = -1; }
    ~HSLBController() throw() {}

  public:
    bool open(int id) throw();
    bool close() throw();
    bool load() throw();
    bool monitor() throw();
    const hslb_info& getInfo() const throw() {
      return m_hslb;
    }
    bool isError() throw();
    bool isBelle2LinkDown() throw();
    bool isCOPPERFifoFull() throw();
    bool isCOPPERLengthFifoFull() throw();
    bool isFifoFull() throw();
    bool isCRCError() throw();

  protected:
    bool boot(const std::string firmware) throw();

  private:
    hslb_info m_hslb;

  };

}

#endif
