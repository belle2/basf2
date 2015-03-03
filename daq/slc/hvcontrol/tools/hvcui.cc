#include "daq/slc/hvcontrol/HVTUINSM.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : %s <config> <cuiname>", argv[0]);
    return 1;
  }
  const std::string cfilename = StringUtil::form("hvcontrol/%s", argv[1]);
  const std::string cuiname = argv[2];
  HVTUINSM tui;
  tui.initNSM(cfilename, cuiname);
  tui.run();
  return (0);
}
