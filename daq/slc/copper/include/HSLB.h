#ifndef _Belle2_HSLB_h
#define _Belle2_HSLB_h

#include <daq/slc/copper/HSLBHandlerException.h>

#include <mgt/hslb_info.h>

#include <string>

namespace Belle2 {

  class HSLB {

  public:
    static const char* getFEEType(int type);

  public:
    HSLB() { m_hslb.fd = -1; }
    ~HSLB() throw() {}

  public:
    int open(int fin) throw(HSLBHandlerException);
    void close() throw();
    bool monitor() throw(HSLBHandlerException);
    void boot(const std::string& firmware) throw(HSLBHandlerException);
    bool isError() throw(HSLBHandlerException);
    bool isBelle2LinkDown() throw(HSLBHandlerException);
    bool isCOPPERFifoFull() throw(HSLBHandlerException);
    bool isCOPPERLengthFifoFull() throw(HSLBHandlerException);
    bool isFifoFull() throw(HSLBHandlerException);
    bool isCRCError() throw(HSLBHandlerException);

  public:
    const hslb_info& getInfo() const throw() { return m_hslb; }
    int fd() const throw() { return m_hslb.fd; }
    int get_finid() const throw() { return m_hslb.fin; }
    operator int() const throw() { return m_hslb.fd; }

  public:
    int readfn(int adr) throw(HSLBHandlerException);
    void writefn(int adr, int val) throw(HSLBHandlerException);
    long long readfn32(int adr) throw(HSLBHandlerException);
    void writefn32(int adr, int val) throw(HSLBHandlerException);
    void hswait_quiet() throw(HSLBHandlerException);
    void hswait() throw(HSLBHandlerException);
    int readfee8(int adr) throw(HSLBHandlerException);
    void writefee8(int adr, int val) throw(HSLBHandlerException);
    long long readfee32(int adr) throw(HSLBHandlerException);
    void writefee32(int adr, int val) throw(HSLBHandlerException);
    void writestream(const char* filename) throw(HSLBHandlerException);
    void bootfpga(const std::string& firmware) throw(HSLBHandlerException);
    int reset_b2l(int& csr);
    int test() throw (HSLBHandlerException);

  private:
    hslb_info m_hslb;

  };

}

#endif
