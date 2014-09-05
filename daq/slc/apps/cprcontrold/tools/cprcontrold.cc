#include "daq/slc/apps/cprcontrold/COPPERCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/DynamicLoader.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

typedef void* (*getfee_t)(void);

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol", argv[1]);
  const std::string hostname = config.get("nsm.local.host");
  const int port = config.getInt("nsm.local.port");
  const std::string name = config.get("nsm.nodename");
  if (!Daemon::start(("cprcontrold." + name).c_str(), argc, argv)) {
    return 1;
  }
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
  NSMNode node(name);
  COPPERCallback* callback = new COPPERCallback(node, fee);
  callback->setFilePath("database/copper");
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();

  return 0;
}
