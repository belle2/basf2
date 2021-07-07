/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cstdlib>
#include <cstring>
#include <stdio.h>

#include "nsm2/belle2nsm.h"

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

#define MONITOR_NODE "rf_mon"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  // 0. Conf file
  char* conffile = getenv("RFARM_CONFFILE");
  RFConf conf(conffile);

  char* format = conf.getconf("system", "nsmdata");

  // 1. Initialize NSM
  if (!b2nsm_init(MONITOR_NODE)) {
    printf("Error in initializing NSM : %s\n", b2nsm_strerror());
    exit(-1);
  }

  printf("----- Control Nodes\n");

  // 1. Dump distributor node
  char* node = conf.getconf("distributor", "nodename");
  RfNodeInfo* nshm = (struct RfNodeInfo*) b2nsm_openmem(node, format, 1);
  printf(" RFIN    : %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
         nshm->nevent_in, nshm->nqueue_in, nshm->avesize_in,
         nshm->flowrate_in, nshm->evtrate_in, nshm->loadave);
  // 2. Dump collector node
  node = conf.getconf("collector", "nodename");
  nshm = (struct RfNodeInfo*) b2nsm_openmem(node, format, 1);
  printf(" RFOUT    : %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
         nshm->nevent_out, nshm->nqueue_out, nshm->avesize_out,
         nshm->flowrate_out, nshm->evtrate_out, nshm->loadave);

  printf("----- Processing Nodes\n");

  // 3. Dump processing nodes
  int maxnodes = conf.getconfi("processor", "nnodes");
  int idbase = conf.getconfi("processor", "idbase");
  char* hostbase = conf.getconf("processor", "nodebase");
  char* badlist = conf.getconf("processor", "badlist");

  char hostnode[512], idname[3];
  int nnodes = 0;
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostnode, "evp_%s%2.2d", hostbase, idbase + i);
      nshm = (struct RfNodeInfo*) b2nsm_openmem(hostnode, format, 1);
      printf(" %s(in) : %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
             hostnode,
             nshm->nevent_in, nshm->nqueue_in, nshm->avesize_in,
             nshm->flowrate_in, nshm->evtrate_in, nshm->loadave);
      printf(" %s(out): %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
             hostnode,
             nshm->nevent_out, nshm->nqueue_out, nshm->avesize_out,
             nshm->flowrate_out, nshm->evtrate_out, nshm->loadave);
      nnodes++;
    }
  }
  /*
  // Test

  printf ( "----- Test\n" );
  strcpy ( hostnode, "evp_hltwk02" );
  nshm = (struct RfNodeInfo*) b2nsm_openmem(hostnode, format, 1);
  printf ( " %s(in) : %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
     hostnode,
           nshm->nevent_in, nshm->nqueue_in, nshm->avesize_in,
           nshm->flowrate_in, nshm->evtrate_in, nshm->loadave );
  printf ( " %s(out): %8d %4d %5.2f %5.2f           ; %6.2f; %5.2f\n",
     hostnode,
           nshm->nevent_out, nshm->nqueue_out, nshm->avesize_out,
           nshm->flowrate_out, nshm->evtrate_out, nshm->loadave );
  */

}
