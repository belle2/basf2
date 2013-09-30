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
    //sprintf(path, "%s/daq/eventbuilder/evb0/eb0", belle2_path);
    sprintf(path, "/home/usr/eb2daq/eb/gen-eb0.sh");//, belle2_path);
    argv[i++] = path;

    char sender_c_v[20][128];
    for (int j = 0; j < _node->getNSenders(); j++) {
      sprintf(sender_c_v[j], "%s", _node->getSender(j)->getHost().c_str());
      argv[j + 1] = sender_c_v[j];
      i++;
    }

    /*
    char arg_n_c[10];
    sprintf(arg_n_c, "-n");
    argv[1] = arg_n_c;

    char sender_c_v[20][128];
    char nsenders_c[16];
    sprintf(nsenders_c, "%d", _node->getNSenders());
    argv[2] = nsenders_c;
    int i = 0;
    for (; i < _node->getNSenders(); i++) {
      sprintf(sender_c_v[i], "%s", _node->getSender(i)->getHost().c_str());
      argv[i + 3] = sender_c_v[i];
    }
    i += 3;

    char arg_D_c[10];
    sprintf(arg_D_c, "-D");
    argv[i++] = arg_D_c;

    char arg_b_c[10];
    sprintf(arg_b_c, "-b");
    argv[i++] = arg_b_c;
    */
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

