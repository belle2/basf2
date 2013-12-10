#include "daq/slc/apps/rocontrold/RecieverManager.h"

#include "daq/slc/system/Executor.h"

#include "daq/slc/base/Debugger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

void RecieverManager::run() throw()
{
  if (_node != NULL) {
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    if (belle2_path == NULL) {
      Belle2::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    Executor executor;
    executor.setExecutable("/bin/sh");
    executor.addArg("%s/daq/copper/daq_scripts/%s",
                    belle2_path, _node->getData()->getText("script").c_str());
    executor.addArg("1");
    executor.addArg("5101");
    executor.addArg(_node->getName().c_str());
    executor.addArg(_buf_path);
    executor.addArg(_fifo_path);
    executor.execute();
  }
}

