#include "daq/slc/apps/eclcollectord/ECLCollectorController.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/system/LogFile.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fstream>

using namespace Belle2;

bool ECLCollectorController::connect()
{
  try {
    m_socket.connect();
    return true;
  } catch (const IOException& e) {
    close();
  }
  return false;
}

void ECLCollectorController::close()
{
  m_socket.close();
}

bool ECLCollectorController::boot(const std::string& config)
{
  std::ifstream fin(config.c_str());
  Shaper sh;
  m_shaper_v = std::vector<Shaper>();
  while (fin >> sh.id) {
    std::string hostname = StringUtil::form("192.168.1.%d", sh.id);
    sh.host = hostname;
    m_shaper_v.push_back(sh);
  }
  for (size_t i = 0; i < m_shaper_v.size(); i++) {
    Shaper& sh(m_shaper_v[i]);
    system(StringUtil::form("sh_boot %s %s", sh.host.c_str(), "A7000000").c_str());
  }
  return true;
}

bool ECLCollectorController::initialize(int mode)
//mode : 1 - without sparsification (All channel readout
//     : 2 - Threshold 50 channels (~1.5 MeV)
//     : 3 Load parameters of Run
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
    Shaper& sh(m_shaper_v[i]);
    std::string cmd = StringUtil::form("sh_init_ecldsp %s %s", sh.host.c_str(),
                                       (mode == 2) ? "AA000000" : "A8000000");
    LogFile::debug(cmd);
    system(cmd.c_str());
    usleep(100000);
    for (size_t i2 = 0; i2 < reg_v.size(); i2++) {
      ShaperReg& reg(reg_v[i2]);
      cmd = StringUtil::form("sh_reg_io %s wq %x %x", sh.host.c_str(),
                             reg.adr, reg.val);
      system(cmd.c_str());
      LogFile::debug(cmd);
    }
    cmd = StringUtil::form("col_reg_io %s w C000 1", sh.host.c_str());
    system(cmd.c_str());
    LogFile::debug(cmd);
    usleep(100000);
    for (size_t i2 = 0; i2 < 16; i2++) {
      cmd = StringUtil::form("sh_reg_io %s wq 30 3f", sh.host.c_str());
      system(cmd.c_str());
      LogFile::debug(cmd);
      cmd = StringUtil::form("sh_reg_io %s wq 31 %x", sh.host.c_str(), 0x10 + i2);
      system(cmd.c_str());
      LogFile::debug(cmd);
      usleep(100000);
    }
  }
  return true;
}

bool ECLCollectorController::sendRequest(const char* param,
                                         unsigned int param_size,
                                         char* reply, int reply_max)
{
  try {
    m_socket.write(param, param_size);
    memset(reply, 0, reply_max + 1);
    m_socket.read_once(reply, reply_max);
    return true;
  } catch (const IOException& e) {
    close();
  }
  return false;
}

bool ECLCollectorController::bootShapers(int sh_num, int mem_addr)
{
  sh_num--;
  char param[100];
  param[0] = 'B';
  param[1] = sh_num;
  param[2] = 200;
  param[3] = 0;
  *(int*)(param + 4) = htonl(mem_addr);
  char reply[1004];
  return sendRequest(param, 4 + sizeof(int), reply, sizeof(reply));
}

bool ECLCollectorController::initDSP(int sh_num, int mem_addr)
{
  sh_num--;
  char param[100];
  param[0] = 'I';
  param[1] = sh_num;
  param[2] = 200;
  param[3] = 0;
  *(int*)(param + 4) = htonl(mem_addr);
  char reply[1004];
  return sendRequest(param, 3 + sizeof(int), reply, sizeof(reply));
}

bool ECLCollectorController::status(int status_type)
{
  char param[100];
  param[0] = 'S';
  param[1] = (char)status_type;
  char reply[1004];
  return sendRequest(param, 2, reply, sizeof(reply));
}
