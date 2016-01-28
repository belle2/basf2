#include "daq/slc/hvcontrol/HVTUINSM.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <stdlib.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  int crate = -1;
  if (argc <= 2) {
    LogFile::debug("Usage : %s <config> [-n <cuiname>] [-c crateid]", argv[0]);
    return 1;
  }
  char* uiname = getenv("USER");
  if (uiname == NULL) uiname = getenv("USERNAME");
  std::string cuiname = (uiname != NULL) ? uiname : "HVCUI";
  cuiname = StringUtil::replace(cuiname, "-", "_");
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-c") {
      crate = atoi(argv[i + 1]);
    } else if (std::string(argv[i]) == "-n") {
      cuiname = argv[i + 1];
    }
  }
  const std::string cfilename = StringUtil::form("hvcontrol/%s", argv[1]);
  HVTUINSM tui(crate);
  tui.initNSM(cfilename, cuiname);
  tui.run();
  return (0);
}
