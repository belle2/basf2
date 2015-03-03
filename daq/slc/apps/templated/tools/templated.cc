#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RCNodeDaemon(config, new TemplateCallback()).run();
  }
  return 0;
}
