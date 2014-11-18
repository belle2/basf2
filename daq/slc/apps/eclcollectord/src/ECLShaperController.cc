#include "daq/slc/apps/eclcollectord/ECLCollectorController.h"

extern "C" {
#include <ecl/ecl_collector_lib.h>
}

using namespace Belle2;

bool ECLShaperController::boot(int sh_num, int mem_addr) throw(IOException)
{
  int ret;
  if ((ret = sh_boot(m_host.c_str(), sh_num, mem_addr)) > 0) {
    std::string msg;
    if (ret == 1) msg = "Connection error";
    else if (ret == 2) msg = "Failed to recieve reply";
    else if (ret == 3) msg = "Boot failed";
    throw (IOException(ret, "Failed to boot Shaper %s", msg.c_str()));
  }
  return true;
}

bool ECLShaperController::init(int sh_num, int mem_addr) throw(IOException)
{
  int ret;
  if ((ret = sh_init_ecldsp(m_host.c_str(), sh_num, mem_addr)) > 0) {
    std::string msg;
    if (ret == 1) msg = "Connection error";
    else if (ret == 2) msg = "Failed to recieve reply";
    else if (ret == 3) msg = "Init failed";
    throw (IOException(ret, "Failed to boot Shaper %s", msg.c_str()));
  }
  return true;
}

