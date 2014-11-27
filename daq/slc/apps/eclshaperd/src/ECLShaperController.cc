#include "daq/slc/apps/eclshaperd/ECLShaperController.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fstream>

using namespace Belle2;

bool ECLShaperController::boot(ECLShaperConfig& config)
throw(IOException)
{
  m_shaper_v = std::vector<ECLShaper>();
  for (ECLShaperConfig::iterator it = config.begin();
       it != config.end(); it++) {
    int sh_num = it->first;
    std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
    m_shaper_v.push_back(ECLShaper(sh_num, hostname));
  }
  for (size_t i = 0; i < m_shaper_v.size(); i++) {
    ECLShaper& sh(m_shaper_v[i]);
    sh.boot(0, 0xA7000000);
  }
  return true;
}

bool ECLShaperController::init(ECLShaperConfig& config, int mode)
throw(IOException)
{
  for (size_t i = 0; i < m_shaper_v.size(); i++) {
    ECLShaper& sh(m_shaper_v[i]);
    sh.init(0, ((mode == 2) ? 0xAA000000 : 0xA8000000));
    usleep(100000);
    int sh_num = m_shaper_v[i].getId();
    std::ifstream fin("/home/usr/b2daq/belle2/release/daq/slc/data/database/eclshaper.txt");
    std::string name;
    unsigned int adr, val;
    while (fin >> name, adr, val) {
      LogFile::debug("write val=%x to adr=%x (%s)", val, adr, name.c_str());
      sh.write(0, adr, val);
    }
    /*
    ECLShaperRegisterList& regs(config[sh_num]);
    for (ECLShaperRegisterList::iterator it = regs.begin();
         it != regs.end(); it++) {
      ECLShaperRegister& reg(it->second);
      sh.write(0, reg.adr, reg.val);
    }
    */
    sh.write(0,  0x800, 0x1);
    sh.write(0, 0xC000, 0x1);
    usleep(100000);
    for (size_t i2 = 0; i2 < 16; i2++) {
      sh.write(0, 0x30, 0x3f);
      sh.write(0, 0x31, 0x10 + i2);
      usleep(100000);
    }
  }
  return true;
}

