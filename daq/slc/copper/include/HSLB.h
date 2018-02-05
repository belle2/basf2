#ifndef _Belle2_HSLB_h
#define _Belle2_HSLB_h

#include <daq/slc/copper/HSLBHandlerException.h>

#include <mgt/hslb_info.h>
#include <mgt/hsreg.h>

#include <string>

#define EHSLB_NOTFOUND   (-1001)
#define EHSLB_TOOOLD     (-1002)
#define EHSLB7_TOOOLD    (-1003)
#define EHSLB_CLOCKNONE  (-1004)
#define EHSLB_CLOCKLOST  (-1005)
#define EHSLB_CLOCKFAST  (-1006)
#define EHSLB_PLLLOST    (-1007)
#define EHSLB_BADSTATE   (-1008)
#define EHSLB_DISABLED   (-1009)
#define EHSLB_INTERNAL   (-1010)

#define EHSLB_B2LDOWN    (-1011)
#define EHSLB_CLOCKBAD   (-1012)
#define EHSLB_PLL2LOST   (-1013)
#define EHSLB_GTPPLL     (-1014)
#define EHSLB_FFCLOCK    (-1015)

#define WHSLB_PLLLOST    (1)
#define WHSLB_B2LDOWN    (2)
#define WHSLB_CLOCKBAD   (3)
#define WHSLB_PLL2LOST   (4)
#define WHSLB_GTPPLL     (5)
#define WHSLB_FFCLOCK    (6)

namespace Belle2 {

  class HSLB {

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
    int reset_b2l(int& csr) throw(HSLBHandlerException);
    int hsreg_getfee(hsreg_t& hsp) throw(HSLBHandlerException);
    int hsreg_read(hsreg_t& hsp) throw(HSLBHandlerException);

  private:
    hslb_info m_hslb;

  };

}

#endif
