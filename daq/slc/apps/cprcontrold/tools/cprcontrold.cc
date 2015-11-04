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
    const bool dummymode = config.getBool("dummymode");
    LogFile::info("dummymode is %s", ((dummymode) ? "on" : "off"));
    FEE* fee[4] = {NULL, NULL, NULL, NULL};
    for (int i = 0; i < 4; i++) {
      const std::string libname = config.get(StringUtil::form("fee.%c.lib", i + 'a'));
      std::string name = config.get(StringUtil::form("fee.%c.name", i + 'a'));
      std::string funcname = "get" + name + "FEE";
      if (libname.size() > 0 && funcname.size() > 0) {
        try {
          LogFile::debug("dlopen(lib=%s, func=%s)", libname.c_str(), funcname.c_str());
          DynamicLoader dl(libname);
          fee[i] = (FEE*)((getfee_t)dl.load(funcname))();
          fee[i]->setName(name);
        } catch (const DynamicLoadException& e) {
          LogFile::fatal("failed to dlopen(lib=%s, func=%s): %s",
                         libname.c_str(), funcname.c_str(), e.what());
          return 1;
        }
      } else {
        fee[i] = NULL;
      }
    }
    RCNodeDaemon(config, new COPPERCallback(fee, dummymode)).run();
  }
  return 0;
}
