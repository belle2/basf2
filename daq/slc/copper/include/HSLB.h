#ifndef _Belle2_HSLB_h
#define _Belle2_HSLB_h

#include <daq/slc/copper/FEEConfig.h>

#include <mgt/hslb_info.h>

namespace Belle2 {

  class HSLB {

  public:
    static const char* getFEEType(int type);

  public:
    HSLB() { m_hslb.fd = -1; }
    ~HSLB() throw() {}

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
    const hslb_info& getInfo() const throw() { return m_hslb; }
    int fd() const throw() { return m_hslb.fd; }
    int get_finid() const throw() { return m_hslb.fin; }
    operator int() const throw() { return m_hslb.fd; }
    int readfn(int adr) throw();
    int writefn(int adr, int val) throw();
    int readfn32(int adr) throw();
    int writefn32(int adr, int val) throw();
    int hswait_quiet() throw();
    int hswait() throw();
    int readfee(int adr) throw();
    int writefee(int adr, int val) throw();
    int readfee8(int adr) throw();
    int writefee8(int adr, int val) throw();
    int readfee32(int adr, int* valp) throw();
    int writefee32(int adr, int val) throw();
    int writestream(const char* filename) throw();
    bool checkfee() throw();
    bool linkfee() throw();
    bool unlinkfee() throw();
    bool trgofffee() throw();
    bool trgonfee() throw();
    bool bootfpga(const std::string& firmware) throw();

  public:
    bool boot(const std::string& firmware) throw();
    const std::string getErrMessage() {
      return m_errmsg;
    }

  private:
    hslb_info m_hslb;
    std::string m_errmsg;

  };

}

#endif
