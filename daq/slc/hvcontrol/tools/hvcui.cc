#include "daq/slc/hvcontrol/HVTUINSM.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  int crate = -1;
  if (argc <= 2) {
    LogFile::debug("Usage : %s <config> <cuiname> [-c crateid]", argv[0]);
    return 1;
  }
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-c") {
      crate = atoi(argv[i + 1]);
      break;
    }
  }
  const std::string cfilename = StringUtil::form("hvcontrol/%s", argv[1]);
  const std::string cuiname = (crate < 0) ? argv[2] : StringUtil::form("%s_%d", argv[2], crate);
  HVTUINSM tui(crate);
  tui.initNSM(cfilename, cuiname);
  tui.run();
  return (0);
}
