#include "daq/slc/apps/rocontrold/EventBuilderManager.h"

#include "daq/slc/base/Debugger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

void EventBuilderManager::run() throw()
{
  if (_node != NULL) {
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    if (belle2_path == NULL) {
      Belle2::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    char path[64];
    char* argv[20];
    int i = 0;
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%s/daq/slc/bin/gen-eb0.sh", belle2_path);
    argv[i] = (char*)malloc(sizeof(char) * 64);
    sprintf(argv[i++], "%s", _node->getData()->getText("hosts").c_str());
    argv[i] = NULL;
    i = 0;
    while (argv[i] != NULL) {
      printf("%s ", argv[i++]);
    }
    printf("\n");
    if (execvp(path, argv) == -1) {
      Belle2::debug("Faield to configure eb0");
    }
    i = 0;
    while (argv[i] != NULL) {
      free(argv[i++]);
    }
  }
}

