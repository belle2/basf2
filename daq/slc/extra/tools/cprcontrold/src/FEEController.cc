#include "FEEController.h"

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

const char* FEEController::getFEEType(int type)
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

bool FEEController::open(int i, const FEEConfig& conf) throw()
{
  hslb_info& hslb(m_info[i]);
  bool force = false;
  if (hslb.fd <= 0) {
    hslb.fd = openfn(i, false);
    hslb.fin = (hslb.fd < 0) ? -1 : i;
    force = true;
  }
  if (hslb.fd < 0) return false;
  monitor(i, conf);
  if (force || isError(i)) {
    bootHSLB(i, conf);
    bootFEE(i, conf);
  }
  linkfee(hslb.fd);
  return (!isError(i)) && checkfee(&hslb);
}

bool FEEController::close() throw()
{
  for (int i = 0; i < 4; i++) {
    hslb_info& hslb(m_info[i]);
    if (hslb.fd < 0) continue;
    unlinkfee(hslb.fd);
    ::close(hslb.fd);
    hslb.fd = -1;
  }
  return true;
}

bool FEEController::load(int i, const FEEConfig& conf) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  loadConfig(i, conf);
  return true;
}

bool FEEController::monitor(int i, const FEEConfig& conf) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd <= 0) return true;
  hslb.b2lstat = readfn32(hslb.fd, HSREGL_STAT);
  hslb.rxdata = readfn32(hslb.fd, HSREGL_RXDATA);
  hslb.fwevt = readfn32(hslb.fd, 0x085);
  hslb.fwclk = readfn32(hslb.fd, 0x086);
  hslb.cntsec = readfn32(hslb.fd, 0x087);
  readStat(i, conf);
  return true;
}

bool FEEController::bootHSLB(int i, const FEEConfig& conf) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd <= 0) return true;
  if (conf.getFirmware().size() > 0) {
    return bootfpga(hslb.fd, (char*)conf.getFirmware().c_str(),
                    false, false, 6) == 0;
  }
  return true;
}

bool FEEController::isError(int i) throw()
{
  if (m_info[i].fd < 0) return true;
  return isBelle2LinkDown(i) ||
         isCOPPERFifoFull(i) ||
         isHSLBFifoFull(i) ||
         isCOPPERLengthFifoFull(i);
}

bool FEEController::isBelle2LinkDown(int i) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  return (hslb.b2lstat & 0x1);
}

bool FEEController::isCOPPERFifoFull(int i) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  return (hslb.b2lstat >> 2 & 0x1);
}

bool FEEController::isCOPPERLengthFifoFull(int i) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  return (hslb.b2lstat >> 4 & 0x1);
}

bool FEEController::isHSLBFifoFull(int i) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  return (hslb.b2lstat >> 3 & 0x1);
}

bool FEEController::isHSLBCRCError(int i) throw()
{
  hslb_info& hslb(m_info[i]);
  if (hslb.fd < 0) return false;
  return (hslb.rxdata >> 16 > 0);
}

