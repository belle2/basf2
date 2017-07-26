#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#ifdef BELLE2_DAQ_PYFEE
#include <daq/slc/pyb2daq/PyFEE.h>
#endif

#include <daq/slc/system/DynamicLoader.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

typedef void* (*getfee_t)(void);

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", "copper", argv[1]);
    FEE* fee[4] = {NULL, NULL, NULL, NULL};
    const std::string libname = config.get("fee.lib");
    std::string name = config.get("fee.name");
    const std::string script = config.get("fee.script");
    if (libname.size() > 0) {
      std::string funcname = "get" + name + "FEE";
      LogFile::debug("dlopen(lib=%s, func=%s)", libname.c_str(), funcname.c_str());
      DynamicLoader dl(libname);
      for (int i = 0; i < 4; i++) {
        LogFile::debug("DEBUG lib %d", i);
        if (config.getBool(StringUtil::form("fee.%c.used", i + 'a'))) {
          try {
            fee[i] = (FEE*)((getfee_t)dl.load(funcname))();
            LogFile::debug("loaded lib %d", i);
            //fee[i]->setName(name);
          } catch (const DynamicLoadException& e) {
            LogFile::fatal("failed to dlopen(lib=%s, func=%s): %s",
                           libname.c_str(), funcname.c_str(), e.what());
            return 1;
          }
        } else {
          fee[i] = NULL;
        }
      }
    }
    RCNodeDaemon(config, new COPPERCallback(fee)).run();
  }
  return 0;
}
