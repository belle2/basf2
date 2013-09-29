#include "RecieverManager.hh"

#include <node/DataSender.hh>
#include <node/HSLB.hh>

#include <util/Debugger.hh>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace B2DAQ;

void RecieverManager::run() throw()
{
  if (_node != NULL) {
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    if (belle2_path == NULL) {
      B2DAQ::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    char path[128];
    char script_c[128];
    sprintf(path, "/bin/sh");
    sprintf(script_c, "%s/daq/copper/daq_scripts/%s",
            belle2_path, _node->getScript().c_str());
    char* argv[3];
    argv[0] = path;
    argv[1] = script_c;
    argv[2] = NULL;
    B2DAQ::debug("%s %s", path, script_c);
    if (execvp(path, argv) == -1) {
      B2DAQ::debug("Faield to start receiver basf2 script");
    }
  }
}

