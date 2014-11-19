#include "daq/slc/apps/eclcollectord/ECLCollectorController.h"

extern "C" {
#include <ecl/ecl_collector_lib.h>
}

#include <errno.h>
#include <cstring>

using namespace Belle2;

bool ECLShaper::boot(int sh_num, int mem_addr) throw(IOException)
{
  int err;
  if ((err = sh_boot(m_host.c_str(), sh_num, mem_addr)) > 0) {
    std::string msg;
    if (err == 1) msg = "Connection error";
    else if (err == 2) msg = "Failed to recieve reply";
    else if (err == 3) msg = "Boot failed";
    throw (IOException(err, "Failed to boot Shaper : %s", msg.c_str()));
  }
  return true;
}

bool ECLShaper::init(int sh_num, int mem_addr) throw(IOException)
{
  int err;
  if ((err = sh_init_ecldsp(m_host.c_str(), sh_num, mem_addr)) > 0) {
    std::string msg;
    if (err == 1) msg = "Connection error";
    else if (err == 2) msg = "Failed to recieve reply";
    else if (err == 3) msg = "Init failed";
    throw (IOException(err, "Failed to boot Shaper : %s", msg.c_str()));
  }
  return true;
}

int ECLShaper::read(int sh_num, int mem_addr) throw(IOException)
{
  int err;
  char msg[255];
  memset(m_reg_data, 0, 12 * sizeof(int));
  memset(msg, 0, 255);
  if ((err = sh_reg_io(m_host.c_str(), "r", sh_num, mem_addr,
                       0, m_reg_data, msg)) > 0) {
    throw (IOException(err, "Failed to read from Shaper : %s", msg));
  }
  return true;
}

int ECLShaper::write(int sh_num, int mem_addr, int reg_wdata) throw(IOException)
{
  int err;
  char msg[255];
  memset(msg, 0, 255);
  if ((err = sh_reg_io(m_host.c_str(), "w", sh_num, mem_addr,
                       reg_wdata, m_reg_data, msg)) > 0) {
    throw (IOException(err, "Failed to write to Shaper : %s", msg));
  }
  return true;
}

