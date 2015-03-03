#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

typedef void* (*getfee_t)(void);

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", "copper", argv[1]);
    FEEController* fee[4] = {NULL, NULL, NULL, NULL};
    for (int i = 0; i < 4; i++) {
      const std::string libname = config.get(StringUtil::form("fee.%c.lib", i + 'a'));
      const std::string funcname = config.get(StringUtil::form("fee.%c.function", i + 'a'));
      if (libname.size() > 0 && funcname.size() > 0) {
        try {
          LogFile::debug("dlopen(lib=%s, func=%s)", libname.c_str(), funcname.c_str());
          DynamicLoader dl(libname);
          fee[i] = (FEEController*)((getfee_t)dl.load(funcname))();
        } catch (const DynamicLoadException& e) {
          LogFile::fatal("failed to dlopen(lib=%s, func=%s): %s",
                         libname.c_str(), funcname.c_str(), e.what());
          return 1;
        }
      }
    }
    RCNodeDaemon(config, new COPPERCallback(fee)).run();
  }
  return 0;
}
