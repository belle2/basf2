#include "EventBuilderManager.h"

#include "base/Debugger.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

void EventBuilderManager::run() throw()
{
  if (_node != NULL) {
    char* argv[30];
    char path[128];
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    if (belle2_path == NULL) {
      Belle2::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    int i = 0;
    sprintf(path, "%s/daq/slc/bin/gen-eb0.sh", belle2_path);
    argv[i++] = path;

    char sender_c[256];
    sprintf(sender_c, "%s", _node->getData()->getTextValue("hosts").c_str());
    argv[i++] = sender_c;

    argv[i++] = NULL;
    i = 0;
    while (argv[i] != NULL) {
      printf("%s ", argv[i]);
      i++;
    }
    printf("\n");
    if (execvp(path, argv) == -1) {
      Belle2::debug("Faield to configure eb0");
    }
  }
}

