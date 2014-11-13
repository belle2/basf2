#include "daq/slc/apps/cprcontrold/HSLBController.h"

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/File.h>
#include <daq/slc/system/LogFile.h>

#include <mgt/libhslb.h>
#include <mgt/hsreg.h>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string.h>

using namespace Belle2;

std::string perform(const std::string& cmd)
{
  FILE* file = popen(cmd.c_str(), "r");
  static char str[1024];
  memset(str, '\0', 1024);
  fread(str, 1, 1024 - 1, file);
  pclose(file);
  std::string s = str;
  return s;//.substr(0, s.find_last_of("\n"));
}

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
  if (m_hslb.fd < 0) {
    m_hslb.fd = openfn(fin, false);
    m_hslb.fin = (m_hslb.fd < 0) ? -1 : fin;
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
  m_hslb.b2lstat = readfn32(HSREGL_STAT);
  m_hslb.rxdata = readfn32(HSREGL_RXDATA);
  m_hslb.fwevt = readfn32(0x085);
  m_hslb.fwclk = readfn32(0x086);
  m_hslb.cntsec = readfn32(0x087);
  return true;
}

bool HSLBController::boot(const std::string& runtype,
                          const std::string& firmware) throw()
{
  //if (m_hslb.fd <= 0) return true;
  if (firmware.size() > 0 && File::exist(firmware)) {
    std::string cmd = StringUtil::form("booths -%c %s", (char)('a' + m_hslb.fin),
                                       firmware.c_str());
    LogFile::debug(cmd);
    system(cmd.c_str());
    std::string FEE("FEE");
    int ntry = 0;
    while (true) {
      cmd = StringUtil::form("reghs -%c checkfee", (char)('a' + m_hslb.fin));
      LogFile::debug(cmd);
      std::string ret = perform(cmd);
      LogFile::debug(ret);
      if (ret.compare(0, FEE.size(), FEE) == 0) break;
      cmd = StringUtil::form("reghs -%c 82 1000", (char)('a' + m_hslb.fin));
      LogFile::debug(cmd);
      system(cmd.c_str());
      cmd = StringUtil::form("reghs -%c 82 10", (char)('a' + m_hslb.fin));
      LogFile::debug(cmd);
      system(cmd.c_str());
      ntry++;
      if (ntry > 50) {
        m_errmsg = StringUtil::form("Can not establich b2link at HSLB %c", (char)('a' + m_hslb.fin));
        LogFile::error(m_errmsg);
        return false;
      }
      usleep(10);
    }
    if (runtype.find("dumhslb") != std::string::npos) {
      StringList str_v = StringUtil::split(runtype, ':');
      const std::string datfile = StringUtil::form("/home/usr/b2daq/run/dumhslb/%s.dat",
                                                   str_v[str_v.size() - 1].c_str());
      cmd = StringUtil::form("write-dumhslb -%c %s", (char)('a' + m_hslb.fin),
                             datfile.c_str());
      LogFile::debug(cmd);
      system(cmd.c_str());
    }
    //return bootfpga(m_hslb.fd, (char*)firmware.c_str(),
    //                false, false, 6) == 0;
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

int HSLBController::readfn(int adr) throw()
{
  return ::readfn(m_hslb.fd, adr);
}

int HSLBController::writefn(int adr, int val) throw()
{
  return ::writefn(m_hslb.fd, adr, val);
}

int HSLBController::readfn32(int adr) throw()
{
  return ::readfn32(m_hslb.fd, adr);
}

int HSLBController::writefn32(int adr, int val) throw()
{
  return ::writefn32(m_hslb.fd, adr, val);
}

int HSLBController::hswait_quiet() throw()
{
  return ::hswait_quiet(m_hslb.fd);
}

int HSLBController::hswait() throw()
{
  return ::hswait(m_hslb.fd);
}

int HSLBController::readfee(int adr) throw()
{
  return ::readfee(m_hslb.fd, adr);
}

int HSLBController::writefee(int adr, int val) throw()
{
  return ::writefee(m_hslb.fd, adr, val);
}

int HSLBController::readfee8(int adr) throw()
{
  return ::readfee8(m_hslb.fd, adr);
}

int HSLBController::writefee8(int adr, int val) throw()
{
  return ::writefee8(m_hslb.fd, adr, val);
}

int HSLBController::readfee32(int adr, int* valp) throw()
{
  return ::readfee32(m_hslb.fd, adr, valp);
}

int HSLBController::writefee32(int adr, int val) throw()
{
  return ::writefee32(m_hslb.fd, adr, val);
}

int HSLBController::writestream(char* filename) throw()
{
  return ::writestream(m_hslb.fd, filename);
}

