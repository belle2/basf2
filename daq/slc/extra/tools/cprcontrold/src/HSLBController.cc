#include "HSLBController.h"

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

const char* HSLBController::getFEEType(int type)
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

bool HSLBController::open(int fin) throw()
{
  //bool force = false;
  if (m_hslb.fd < 0) {
    m_hslb.fd = openfn(fin, false);
    m_hslb.fin = (m_hslb.fd < 0) ? -1 : fin;
    //force = true;
  }
  if (m_hslb.fd < 0) return false;
  monitor();
  return true;
}

bool HSLBController::close() throw()
{
  if (m_hslb.fd < 0) return false;
  unlinkfee(m_hslb.fd);
  ::close(m_hslb.fd);
  m_hslb.fd = -1;
  return true;
}

bool HSLBController::load(/*int triggermode*/) throw()
{
  if (m_hslb.fd < 0) return false;
  linkfee(m_hslb.fd);
  return (!isError()) && checkfee(&m_hslb);
}

bool HSLBController::monitor() throw()
{
  if (m_hslb.fd <= 0) return true;
  m_hslb.b2lstat = readfn32(m_hslb.fd, HSREGL_STAT);
  m_hslb.rxdata = readfn32(m_hslb.fd, HSREGL_RXDATA);
  m_hslb.fwevt = readfn32(m_hslb.fd, 0x085);
  m_hslb.fwclk = readfn32(m_hslb.fd, 0x086);
  m_hslb.cntsec = readfn32(m_hslb.fd, 0x087);
  return true;
}

bool HSLBController::boot(const std::string firmware) throw()
{
  if (m_hslb.fd <= 0) return true;
  if (firmware.size() > 0) {
    return bootfpga(m_hslb.fd, (char*)firmware.c_str(),
                    false, false, 6) == 0;
  }
  return true;
}

bool HSLBController::isError() throw()
{
  return isBelle2LinkDown() ||
         isCOPPERFifoFull() ||
         isFifoFull() ||
         isCOPPERLengthFifoFull();
}

bool HSLBController::isBelle2LinkDown() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat & 0x1);
}

bool HSLBController::isCOPPERFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 2 & 0x1);
}

bool HSLBController::isCOPPERLengthFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 4 & 0x1);
}

bool HSLBController::isFifoFull() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.b2lstat >> 3 & 0x1);
}

bool HSLBController::isCRCError() throw()
{
  if (m_hslb.fd < 0) return false;
  return (m_hslb.rxdata >> 16 > 0);
}

