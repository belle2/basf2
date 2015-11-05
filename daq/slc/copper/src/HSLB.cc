#include "daq/slc/copper/HSLB.h"

#include <daq/slc/system/File.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstring>

#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")

using namespace Belle2;

const char* HSLB::getFEEType(int type)
{
  static const char* feetype[] = {
    "UNDEF", "SVD", "CDC", "BPID", "EPID", "ECL", "KLM", "TRG",
    "UNKNOWN-8", "UNKNOWN-9", "UNKNOWN-10", "UNKNOWN-11",
    "UNKNOWN-12", "UNKNOWN-13", "UNKNOWN-14", "TEST"
  };
  if (type >= 0 && type < 16)
    return feetype[type];
  return NULL;
}

int HSLB::open(int fin) throw(HSLBHandlerException)
{
  if (m_hslb.fd < 0) {
    m_hslb.fd = openfn(fin, false);
    m_hslb.fin = (m_hslb.fd < 0) ? -1 : fin;
  }
  if (m_hslb.fd < 0) {
    throw (HSLBHandlerException("open failed %s", hslberr()));
  }
  return m_hslb.fd;
}

void HSLB::close() throw()
{
  if (m_hslb.fd < 0) return;
  ::close(m_hslb.fd);
  m_hslb.fd = -1;
}

void HSLB::load() throw(HSLBHandlerException)
{
  if (m_hslb.fd < 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  linkfee();
  if (checkfee() == "UNKNOWN") {
    throw (HSLBHandlerException("fee on hslb-%c is not available", m_hslb.fin + 'a'));
  }
}

bool HSLB::monitor() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  m_hslb.b2lstat = readfn32(HSREGL_STAT);
  m_hslb.rxdata = readfn32(HSREGL_RXDATA);
  m_hslb.fwevt = readfn32(0x085);
  m_hslb.fwclk = readfn32(0x086);
  m_hslb.cntsec = readfn32(0x087);
  m_hslb.feecrce = readfee8(HSREG_CRCERR);

  return true;
}

void HSLB::boot(const std::string& firmware)
throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return bootfpga(firmware);
}

bool HSLB::isError() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return isBelle2LinkDown() ||
         isCOPPERFifoFull() ||
         isFifoFull() ||
         isCOPPERLengthFifoFull();
}

bool HSLB::isBelle2LinkDown() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return (m_hslb.b2lstat & 0x1);
}

bool HSLB::isCOPPERFifoFull() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return (m_hslb.b2lstat >> 2 & 0x1);
}

bool HSLB::isCOPPERLengthFifoFull() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return (m_hslb.b2lstat >> 4 & 0x1);
}

bool HSLB::isFifoFull() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return (m_hslb.b2lstat >> 3 & 0x1);
}

bool HSLB::isCRCError() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return (m_hslb.feecrce > 0);
  //return (m_hslb.rxdata >> 16 > 0);
}

int HSLB::readfn(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  int val;
  if ((val = ::readfn(m_hslb.fd, adr)) < 0) {
    throw (HSLBHandlerException("error : readfn hslb-%c : %s", m_hslb.fin + 'a', hslberr()));
  }
  return val;
}

void HSLB::writefn(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  if (::writefn(m_hslb.fd, adr, val) < 0) {
    throw (HSLBHandlerException("error : writefn hslb-%c : %s", m_hslb.fin + 'a', hslberr()));
  }
}

int HSLB::readfn32(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  //return ::readfn32(m_hslb.fd, adr);
  int val = 0;
  writefn(0x6f, adr & 0x7f);
  val |= (readfn(0x6e) << 0);
  val |= (readfn(0x6e) << 8);
  val |= (readfn(0x6e) << 16);
  val |= (readfn(0x6e) << 24);
  return val;
}

void HSLB::writefn32(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(0x6e, (val >> 24) & 0xff);
  writefn(0x6e, (val >> 16) & 0xff);
  writefn(0x6e, (val >>  8) & 0xff);
  writefn(0x6e, (val >>  0) & 0xff);
  writefn(0x6f, adr & 0xff);
  //return ::writefn32(m_hslb.fd, adr, val);
}

void HSLB::hswait_quiet() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  if (::hswait_quiet(m_hslb.fd) < 0) {
    throw (HSLBHandlerException("timeout of hswait_quiet on hslb-%c", m_hslb.fin + 'a'));
  }
}

void HSLB::hswait() throw(HSLBHandlerException)
{
  hswait_quiet();
}

int HSLB::readfee8(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR, 0x05); /* reset read fifo */
  writefn(adr, 0);          /* dummy value write to pass address */
  writefn(HSREG_CSR, 0x07); /* parameter read */
  hswait_quiet();
  return readfn(adr);
  //return ::readfee8(m_hslb.fd, adr);
}

void HSLB::writefee8(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR, 0x05); /* reset read fifo */
  writefn(adr, val & 0xff);
  writefn(HSREG_CSR, 0x0a); /* parameter write */
  //return ::writefee8(m_hslb.fd, adr, val);
}

int HSLB::readfee32(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR,    0x05); /* reset read fifo */
  writefn(HSREG_CSR,    0x0c); /* 32-bit parameter read */
  writefn(HSREG_SERIAL, (adr >> 8) & 0xff);
  writefn(HSREG_SERIAL, (adr >> 0) & 0xff);
  writefn(HSREG_CSR,    0x08); /* end of stream */
  hswait_quiet();
  return ((readfn(HSREG_D32D) & 0xff) << 24) |
         ((readfn(HSREG_D32C) & 0xff) << 16) |
         ((readfn(HSREG_D32B) & 0xff) <<  8) |
         ((readfn(HSREG_D32A) & 0xff) <<  0);
  //return ::readfee32(m_hslb.fd, adr, valp);
}

void HSLB::writefee32(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR,    0x05); /* reset read fifo */
  writefn(HSREG_CSR,    0x0b); /* 32-bit parameter write */
  writefn(HSREG_SERIAL, (adr >>  8) & 0xff);
  writefn(HSREG_SERIAL, (adr >>  0) & 0xff);
  writefn(HSREG_SERIAL, (val >> 24) & 0xff);
  writefn(HSREG_SERIAL, (val >> 16) & 0xff);
  writefn(HSREG_SERIAL, (val >>  8) & 0xff);
  writefn(HSREG_SERIAL, (val >>  0) & 0xff);
  writefn(HSREG_CSR,    0x08); /* end of stream */
  //return ::writefee32(m_hslb.fd, adr, val);
}

void HSLB::writestream(const char* filename) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  if (::writestream(m_hslb.fd, (char*)filename) < 0) {
    throw (HSLBHandlerException("Failed to write stream to hslb-%c : %s", m_hslb.fin + 'a'), hslberr());
  }
}

std::string HSLB::checkfee() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  static const char* feetype[] = {
    "UNDEF", "SVD", "CDC", "BPID", "EPID", "ECL", "KLM", "TRG",
    "UNKNOWN-8", "UNKNOWN-9", "UNKNOWN-10", "UNKNOWN-11",
    "UNKNOWN-12", "UNKNOWN-13", "DEMO", "TEST"
  };
  if (::checkfee(&m_hslb) > 0 && m_hslb.feehw < 16 && m_hslb.feetype >= 0) {
    return StringUtil::form("FEE type: %s \nserial: %d \nfirm type: %d \nfirm version: %d",
                            feetype[m_hslb.feehw], m_hslb.feeserial, m_hslb.feetype, m_hslb.feever);
  }
  return std::string("UNKNOWN");
}

std::string HSLB::test() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  char msg[256];
  memset(msg, 0, 256);
  if (check_hslb(&m_hslb, msg) > 0) {
    throw (HSLBHandlerException("tesths failed : %s", msg));
  }
  return msg;
}

void HSLB::linkfee() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR,     0x05); // reset read fifo
  writefn(HSREG_CSR,     0x06); // reset read ack
  writefn(HSREG_FEECONT, 0x01); // HSREG_FEECONT
  writefn(HSREG_CSR,     0x0a); // parameter write
}

void HSLB::trgofffee() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR,     0x05); // reset read fifo
  writefn(HSREG_CSR,     0x06); // reset read ack
  writefn(HSREG_FEECONT, 0x03); // HSREG_FEECONT
  writefn(HSREG_CSR,     0x0a); // parameter write
}

void HSLB::trgonfee() throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  writefn(HSREG_CSR,     0x05); // reset read fifo
  writefn(HSREG_CSR,     0x06); // reset read ack
  writefn(HSREG_FEECONT, 0x03); // HSREG_FEECONT
  writefn(HSREG_CSR,     0x0a); // parameter write
}

void HSLB::bootfpga(const std::string& firmware) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  if (::bootfpga(m_hslb.fd, firmware.c_str(), 0, 0, M012_SELECTMAP) < 0) {
    throw (HSLBHandlerException("Failed to boot FPGA on hslb-%c : %s", m_hslb.fin + 'a', hslberr()));
  }
}
