#include "EventBuilderManager.hh"

#include <node/DataSender.hh>
#include <node/HSLB.hh>

#include <util/Debugger.hh>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace B2DAQ;

void EventBuilderManager::run() throw()
{
  if (_node != NULL) {
    char* argv[30];
    char path[128];
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    if (belle2_path == NULL) {
      B2DAQ::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    int i = 0;
    sprintf(path, "%s/daq/slc/bin/gen-eb0.sh", belle2_path);
    argv[i++] = path;

    char sender_c_v[20][128];
    for (int j = 0; j < _node->getNSenders(); j++) {
      sprintf(sender_c_v[j + 1], "%s", _node->getSender(j).c_str());
      argv[i++] = sender_c_v[j + 1];
    }

    argv[i++] = NULL;
    i = 0;
    while (argv[i] != NULL) {
      printf("%s ", argv[i]);
      i++;
    }
    printf("\n");
    if (execvp(path, argv) == -1) {
      B2DAQ::debug("Faield to configure eb0");
    }
  }
}

