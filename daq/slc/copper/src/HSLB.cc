#include "daq/slc/copper/HSLB.h"

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstring>
#include <cstdlib>

#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")

using namespace Belle2;

int HSLB::open(int fin) throw(HSLBHandlerException)
{
  if (m_hslb.fd < 0) {
    m_hslb.fd = openfn(fin, false, NULL);
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

long long HSLB::readfn32(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return ::readfn32(m_hslb.fd, adr);
}

void HSLB::writefn32(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  ::writefn32(m_hslb.fd, adr, val);
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
  int val2 = ::readfee8(m_hslb.fd, adr);
  if (val2 < 0) {
    throw (HSLBHandlerException("no response from FEE at HSLB:%c", m_hslb.fin + 'a'));
  } else {
    val2 = ::readfn(m_hslb.fd, adr);
  }
  return val2;
}

void HSLB::writefee8(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  ::writefee8(m_hslb.fd, adr, val);
}

long long HSLB::readfee32(int adr) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  int val;
  ::readfee32(m_hslb.fd, adr, &val);
  return val;
}

void HSLB::writefee32(int adr, int val) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  ::writefee32(m_hslb.fd, adr, val);
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

void HSLB::bootfpga(const std::string& firmware) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  if (::boot_fpga(m_hslb.fd, firmware.c_str(), 0, 0, M012_SELECTMAP) < 0) {
    throw (HSLBHandlerException("Failed to boot FPGA on hslb-%c : %s", m_hslb.fin + 'a', hslberr()));
  }
}

int HSLB::reset_b2l(int& csr) throw(HSLBHandlerException)
{
  int i = 0;
  csr = 0;
  int force = 1;
  for (i = 0; i < 100; i++) {
    int j = 0;
    csr = readfn32(HSREGL_STAT); /* 0x83 */

    if (csr & 0x80) {
      writefn32(HSREGL_RESET, 0x100000);
      usleep(100000);
    } else if (csr & 0x20000000) {
      writefn32(HSREGL_RESET, 0x10000);
      usleep(100000);
    }

    for (j = 0; j < 100; j++) {
      int csr2 = readfn32(HSREGL_STAT); /* 0x83 */
      if ((csr ^ csr2) & 0x100) break;
    }
    if (j == 100) csr |= 0x20000000;

    if ((csr & 0x200000e1) == 0 && force == 0) break;
    force = 0;
    writefn32(HSREGL_RESET, 0x1000);
    usleep(100000);
    writefn32(HSREGL_RESET, 0x10);
    usleep(100000);
  }
  return i;
}

int HSLB::hsreg_getfee(hsreg_t& hsp) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return ::hsreg_getfee(m_hslb.fd, &hsp);
}

int HSLB::hsreg_read(hsreg_t& hsp) throw(HSLBHandlerException)
{
  if (m_hslb.fd <= 0) {
    throw (HSLBHandlerException("hslb-%c is not available", m_hslb.fin + 'a'));
  }
  return ::hsreg_read(m_hslb.fd, &hsp);
}
