#include "daq/slc/copper/HSLB.h"

#include <daq/slc/system/File.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>

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

bool HSLB::open(int fin) throw()
{
  if (m_hslb.fd < 0) {
    m_hslb.fd = openfn(fin, false);
    m_hslb.fin = (m_hslb.fd < 0) ? -1 : fin;
  }
  if (m_hslb.fd < 0) return false;
  return true;
}

bool HSLB::close() throw()
{
  if (m_hslb.fd < 0) return false;
  unlinkfee();
  ::close(m_hslb.fd);
  m_hslb.fd = -1;
  return true;
}

bool HSLB::load() throw()
{
  if (m_hslb.fd < 0) return false;
  linkfee();
  return (!isError()) && checkfee() != "UNKNOWN";
}

bool HSLB::monitor() throw()
{
  if (m_hslb.fd <= 0) return true;
  m_hslb.b2lstat = readfn32(HSREGL_STAT);
  m_hslb.rxdata = readfn32(HSREGL_RXDATA);
  m_hslb.fwevt = readfn32(0x085);
  m_hslb.fwclk = readfn32(0x086);
  m_hslb.cntsec = readfn32(0x087);
  return true;
}

bool HSLB::boot(const std::string& firmware) throw()
{
  if (m_hslb.fd <= 0) return true;
  return bootfpga(firmware);
}

bool HSLB::isError() throw()
{
  return isBelle2LinkDown() ||
         isCOPPERFifoFull() ||
         isFifoFull() ||
         isCOPPERLengthFifoFull();
}

bool HSLB::isBelle2LinkDown() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat & 0x1);
}

bool HSLB::isCOPPERFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 2 & 0x1);
}

bool HSLB::isCOPPERLengthFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 4 & 0x1);
}

bool HSLB::isFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 3 & 0x1);
}

bool HSLB::isCRCError() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.rxdata >> 16 > 0);
}

int HSLB::readfn(int adr) throw()
{
  return ::readfn(m_hslb.fd, adr);
}

int HSLB::writefn(int adr, int val) throw()
{
  return ::writefn(m_hslb.fd, adr, val);
}

int HSLB::readfn32(int adr) throw()
{
  return ::readfn32(m_hslb.fd, adr);
}

int HSLB::writefn32(int adr, int val) throw()
{
  return ::writefn32(m_hslb.fd, adr, val);
}

int HSLB::hswait_quiet() throw()
{
  return ::hswait_quiet(m_hslb.fd);
}

int HSLB::hswait() throw()
{
  return ::hswait(m_hslb.fd);
}

int HSLB::readfee8(int adr) throw()
{
  return ::readfee8(m_hslb.fd, adr);
}

int HSLB::writefee8(int adr, int val) throw()
{
  return ::writefee8(m_hslb.fd, adr, val);
}

int HSLB::readfee32(int adr, int* valp) throw()
{
  return ::readfee32(m_hslb.fd, adr, valp);
}

int HSLB::writefee32(int adr, int val) throw()
{
  return ::writefee32(m_hslb.fd, adr, val);
}

int HSLB::writestream(const char* filename) throw()
{
  return ::writestream(m_hslb.fd, (char*)filename);
}

std::string HSLB::checkfee() throw()
{
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

bool HSLB::writefee(int adr, int val) throw()
{
  return ::writefee(m_hslb.fd, adr, val) == 0;
}

int HSLB::readfee(int adr) throw()
{
  int val = -1;
  if (::readfee(m_hslb.fd, adr, &val) == 0) {
    return val;
  }
  return -1;
}

bool HSLB::linkfee() throw()
{
  return ::linkfee(m_hslb.fd) == 0;
}

bool HSLB::unlinkfee() throw()
{
  return ::unlinkfee(m_hslb.fd) == 0;
}

bool HSLB::trgofffee() throw()
{
  return ::trgofffee(m_hslb.fd) == 0;
}

bool HSLB::trgonfee() throw()
{
  return ::trgonfee(m_hslb.fd) == 0;
}

bool HSLB::bootfpga(const std::string& firmware) throw()
{
  return ::bootfpga(m_hslb.fd, firmware.c_str(), 0, 0, M012_SELECTMAP) == 0;
}
