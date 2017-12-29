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
    //printf("reg%02x = %02x\n", adr, val2);
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

int HSLB::reset_b2l(int& csr)
{
  int i = 0;
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

#define WHSLB_PLLLOST    (1)
#define WHSLB_B2LDOWN    (2)
#define WHSLB_CLOCKBAD   (3)
#define WHSLB_PLL2LOST   (4)
#define WHSLB_GTPPLL     (5)
#define WHSLB_FFCLOCK    (6)

int HSLB::test() throw (HSLBHandlerException)
{
  int warning = 0;
  int id;
  int ver;
  int csr;
  int j;

  id  = readfn32(HSREGL_ID);   /* 0x80 */
  ver = readfn32(HSREGL_VER);  /* 0x81 */
  csr = readfn32(HSREGL_STAT); /* 0x83 */

  if (id != 0x48534c42 && id != 0x48534c37) {
    throw (HSLBHandlerException("hslb-%c not found (id=0x%08x != 0x48534c42)\n", 'a' + m_hslb.fin, id));
  }
  if (id == 0x48534c42 && ver < 34) {
    throw (HSLBHandlerException("hslb-%c too old firmware (ver=0.%02d < 0.34)\n", 'a' + m_hslb.fin, ver));
  }
  if (id == 0x48534c37 && ver < 6) {
    throw (HSLBHandlerException("hslb7-%c too old firmware (ver=0.%02d < 0.06)\n", 'a' + m_hslb.fin, ver));
  }
  if (csr & 0x20000000) {
    int j;
    int recvok = 0;
    int uptime0 = readfn32(HSREGL_UPTIME);
    int uptime1;
    usleep(1000 * 1000);
    uptime1 = readfn32(HSREGL_UPTIME);

    if (uptime0 == 0) {
      throw (HSLBHandlerException("hslb-%c clock is missing\n", 'a' + m_hslb.fin));
    } else if (uptime0 == uptime1) {
      throw (HSLBHandlerException("hslb-%c clock is lost or too slow\n", 'a' + m_hslb.fin));
    } else if (uptime1 > uptime0 + 1) {
      throw (HSLBHandlerException("hslb-%c clock is too fast\n", 'a' + m_hslb.fin));
    }

    for (j = 0; j < 100; j++) {
      int recv = readfn32(HSREGL_RXDATA) & 0xffff;
      if (recv == 0x00bc) recvok++;
    }
    if (recvok < 80) {
      throw (HSLBHandlerException("hslb-%c PLL lost and can't receive data (csr=%08x)\n",
                                  'a' + m_hslb.fin, csr));
    }
    throw (HSLBHandlerException("hslb-%c PLL lost (csr=%08x) is probably harmless and ignored\n",
                                'a' + m_hslb.fin, csr));
    csr &= ~0x20000000;
    warning = WHSLB_PLLLOST;
  }
  for (j = 0; j < 100; j++) {
    int csr2 = readfn32(HSREGL_STAT); /* 0x83 */
    if ((csr ^ csr2) & 0x100) break;
  }
  if (j == 100) csr |= 0x20000000;

  /*
    bit 00000001 - link is not established
    bit 00000002 - hslb is disabled
    bit 00000020 - bad 127MHz detected
    bit 00000040 - GTP PLL not locked (never happen?)
    bit 00000080 - PLL2 not locked
    bit 00000100 - LSB of statff, should be toggling
    bit 20000000 - PLL1 not locked, but somehow not correctly working,
    so it is reused for j=100 condition
    bit 80000000 - link down happened in the past
  */

  if ((csr & 0x200000e1)) {
    int count;
    int oldcsr = csr;

    count = reset_b2l(csr);

    if ((csr & 0x200000e1)) {
      if (csr & 1) {
        throw (HSLBHandlerException("hslb-%c Belle2link is down, csr=%08x\n", 'a' + m_hslb.fin, csr));
      } else if (csr & 0x20) {
        throw (HSLBHandlerException("hslb-%c bad clock detected, csr=%08x\n", 'a' + m_hslb.fin, csr));
      } else if (csr & 0x80) {
        throw (HSLBHandlerException("hslb-%c PLL2 lock lost, csr=%08x\n", 'a' + m_hslb.fin, csr));
      } else if (csr & 0x40) {
        throw (HSLBHandlerException("hslb-%c GTP PLL lock lost, csr=%08x\n", 'a' + m_hslb.fin, csr));
      } else if (csr & 0x20000000) {
        throw (HSLBHandlerException("hslb-%c FF clock is stopped, csr=%08x\n", 'a' + m_hslb.fin, csr));
      }
    } else {
      if (oldcsr & 1) {
        warning = WHSLB_B2LDOWN;
        LogFile::warning("hslb-%c Belle2link recovered, csr=%08x (retry %d)\n", 'a' + m_hslb.fin, csr, count);
      } else if (oldcsr & 0x20) {
        warning = WHSLB_B2LDOWN;
        LogFile::warning("hslb-%c bad clock recovered, csr=%08x (retry %d)\n", 'a' + m_hslb.fin, csr, count);
      } else if (oldcsr & 0x80) {
        warning = WHSLB_PLL2LOST;
        LogFile::warning("hslb-%c PLL2 lock recovered, csr=%08x (retry %d)\n", 'a' + m_hslb.fin, csr, count);
      } else if (oldcsr & 0x40) {
        warning = WHSLB_GTPPLL;
        throw (HSLBHandlerException("hslb-%c GTP PLL lock recovered, csr=%08x (retry %d)\n",
                                    'a' + m_hslb.fin, csr, count));
      } else if (oldcsr & 0x20000000) {
        warning = WHSLB_FFCLOCK;
        throw (HSLBHandlerException("hslb-%c FF clock is recovered, csr=%08x (retry %d)\n",
                                    'a' + m_hslb.fin, csr, count));
      }
    }
  }

  /* 2015.0605.1524 this doesn't seem to be a solution when statepr != 0 */
  /* 2015.0618.1408 although not perfect, still better than nothing */
  if (csr & 0x000f0000) {
    readfee32(0);
    csr = readfn32(HSREGL_STAT); /* 0x83 */
  }

  if ((csr & 0x5fffeec1) != 0x18000000 && (csr & 0x200000e1) == 0) {
    throw (HSLBHandlerException("hslb-%c hslb in bad state (csr=%08x)\n", 'a' + m_hslb.fin, csr));
  }
  if (csr & 2) {
    throw (HSLBHandlerException("hslb-%c is disabled, ttrx reg 130 bit%d=0\n", 'a' + m_hslb.fin, csr));
  }
  //LogFile::info("hslb-%c 0.%02d %08x\n", 'a'+m_hslb.fin, ver, csr));
  return warning;
}
