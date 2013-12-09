#include "daq/slc/apps/rocontrold/RecieverManager.h"

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
    char path[64];
    char buf[20][64];
    char* argv[20];
    int i = 0;
    sprintf(path, "/bin/sh");
    sprintf(buf[i], "%s/daq/copper/daq_scripts/%s",
            belle2_path, _node->getData()->getText("script").c_str());
    argv[i] = buf[i]; i++;
    sprintf(buf[i], "1");
    argv[i] = buf[i]; i++;
    sprintf(buf[i], "5101");
    argv[i] = buf[i]; i++;
    sprintf(buf[i], "%s", _node->getName().c_str());
    argv[i] = buf[i]; i++;
    sprintf(buf[i], "%s", _buf_path.c_str());
    argv[i] = buf[i]; i++;
    sprintf(buf[i], "%s", _fifo_path.c_str());
    argv[i] = buf[i]; i++;
    argv[i] = NULL;
    i = 0;
    while (argv[i] != NULL) {
      printf("%s ", argv[i++]);
    }
    printf("\n");
    if (execvp(path, argv) == -1) {
      Belle2::debug("Faield to start receiver basf2 script");
    }
  }
}

