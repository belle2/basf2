#include "daq/slc/apps/eclcollectord/ECLCollectorController.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fstream>

using namespace Belle2;

bool ECLCollectorController::boot(const std::string& config)
throw(IOException)
{
  std::ifstream fin(config.c_str());
  int id;
  m_shaper_v = std::vector<ECLShaperController>();
  while (fin >> id) {
    std::string hostname = StringUtil::form("192.168.1.%d", id);
    m_shaper_v.push_back(ECLShaperController(id, hostname));
  }
  for (size_t i = 0; i < m_shaper_v.size(); i++) {
    ECLShaperController& sh(m_shaper_v[i]);
    sh.boot(0, 0xA7000000);
    //system(StringUtil::form("sh_boot %s %s", sh.getHost().c_str(), "A7000000").c_str());
  }
  return true;
}

bool ECLCollectorController::init(int mode)
throw(IOException)
{
  std::vector<ShaperReg> reg_v;
  reg_v.push_back(ShaperReg(0x502, 0xD, "THREAD_AF"));
  reg_v.push_back(ShaperReg(0x500, 0xc8, "TRBUF"));
  reg_v.push_back(ShaperReg(0x501, 0xc0, "UTHREAD_AF"));
  reg_v.push_back(ShaperReg(0x200, 0xffff, "SHAPER_PROC_MASK"));
  reg_v.push_back(ShaperReg(0x210, 0x10, "SHAPER_PROC_NUM"));
  reg_v.push_back(ShaperReg(0x208, 0xffff, "SHAPER_ADC_MASK"));
  reg_v.push_back(ShaperReg(0x218, 0x10, "SHAPER_ADC_NUM"));
  reg_v.push_back(ShaperReg(0x220, 0x2D, "TRG2ADC_DATA_END"));
  reg_v.push_back(ShaperReg(0x20, 0xB6, "ADC1_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x21, 0xB6, "ADC2_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x22, 0xB6, "ADC3_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x23, 0xB6, "ADC4_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x24, 0xB6, "ADC5_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x25, 0xB6, "ADC6_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x26, 0xB6, "ADC7_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x27, 0xB6, "ADC8_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x28, 0xB6, "ADC9_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x29, 0xB6, "ADC10_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2A, 0xB6, "ADC11_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2B, 0xB6, "ADC12_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2C, 0xB6, "ADC13_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2D, 0xB6, "ADC14_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2E, 0xB6, "ADC15_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x2F, 0xB6, "ADC16_COMP_VALUE"));
  reg_v.push_back(ShaperReg(0x40, 0x0000, "SH_RELAY"));
  reg_v.push_back(ShaperReg(0x223, 0x40, "ADC_DATA_LEN"));
  reg_v.push_back(ShaperReg(0x800, 0x1, ""));
  for (size_t i = 0; i < m_shaper_v.size(); i++) {
    ECLShaperController& sh(m_shaper_v[i]);
    sh.init(0, ((mode == 2) ? 0xAA000000 : 0xA8000000));
    //std::string cmd = StringUtil::form("sh_init_ecldsp %s %s", sh.getHost().c_str(),
    //                                   (mode == 2) ? "AA000000" : "A8000000");
    //LogFile::debug(cmd);
    //system(cmd.c_str());
    usleep(100000);
    for (size_t i2 = 0; i2 < reg_v.size(); i2++) {
      ShaperReg& reg(reg_v[i2]);
      sh.write(0, reg.adr, reg.val);
      //cmd = StringUtil::form("sh_reg_io %s wq %x %x", sh.getHost().c_str(),
      //                       reg.adr, reg.val);
      //system(cmd.c_str());
      //LogFile::debug(cmd);
    }
    sh.write(0, 0xC000, 0x1);
    //cmd = StringUtil::form("col_reg_io %s w C000 1", sh.getHost().c_str());
    //system(cmd.c_str());
    //LogFile::debug(cmd);
    usleep(100000);
    for (size_t i2 = 0; i2 < 16; i2++) {
      sh.write(0, 0x30, 0x3f);
      sh.write(0, 0x31, 0x10 + i2);
      //cmd = StringUtil::form("sh_reg_io %s wq 30 3f", sh.getHost().c_str());
      //system(cmd.c_str());
      //LogFile::debug(cmd);
      //cmd = StringUtil::form("sh_reg_io %s wq 31 %x", sh.getHost().c_str(), 0x10 + i2);
      //system(cmd.c_str());
      //LogFile::debug(cmd);
      usleep(100000);
    }
  }
  return true;
}

