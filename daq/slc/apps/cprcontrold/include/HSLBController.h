#ifndef _Belle2_HSLBController_h
#define _Belle2_HSLBController_h

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
    bool isError() throw();
    bool isBelle2LinkDown() throw();
    bool isCOPPERFifoFull() throw();
    bool isCOPPERLengthFifoFull() throw();
    bool isFifoFull() throw();
    bool isCRCError() throw();

  public:
    const hslb_info& getInfo() const throw() {
      return m_hslb;
    }
    int fd() const throw() { return m_hslb.fd; }
    operator int() const throw() { return m_hslb.fd; }
    int readfn(int adr) throw();
    int writefn(int adr, int val) throw();
    int readfn32(int adr) throw();
    int writefn32(int adr, int val) throw();
    int hswait_quiet() throw();
    int hswait() throw();
    int readfee8(int adr) throw();
    int writefee8(int adr, int val) throw();
    int readfee32(int adr, int* valp) throw();
    int writefee32(int adr, int val) throw();
    int writestream(char* filename) throw();

  protected:
    bool boot(const std::string firmware) throw();

  private:
    hslb_info m_hslb;

  };

}

#endif
