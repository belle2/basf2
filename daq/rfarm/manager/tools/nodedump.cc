/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//#define DESY

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "daq/rfarm/manager/RFConf.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RFFlowStat.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{

  if (argc < 2) {
    printf("Usage : nodedump unitname nodename\n");
    return -1;
  }

  char host[256];
  gethostname(host, sizeof(host));

  char* confdir = getenv("RFARM_CONFDIR");
  string confpath = string(confdir) + "/" + string(argv[1]) + ".conf";

  RFConf conf(confpath.c_str());

  //  char* conffile = getenv("RFARM_CONFFILE");
  //  RFConf conf(conffile);

  char nodename[256];
  int nid = 0;
  int idlist[256];
  char item[256][7];

  if (argc >= 3) {
    if (strstr(argv[2], "dist") != NULL) {      // distributor
      strcpy(nodename, "distributor");
      int nnodes = conf.getconfi("processor", "nnodes");
      idlist[0] = RF_INPUT_ID;
      strcpy(item[0], "IN    ");
      for (int i = 0; i < nnodes; i++) {
        idlist[i + 1] = i;
        sprintf(item[i + 1], "OUT%2.2d ", i);
      }
      nid = nnodes + 1;
    } else if (strstr(argv[2], "col") != NULL) {    // collector
      strcpy(nodename, "collector");
      int nnodes = conf.getconfi("processor", "nnodes");
      for (int i = 0; i < nnodes; i++) {
        idlist[i] = i;
        sprintf(item[i], "IN%2.2d  ", i);
      }
      idlist[nnodes] = RF_OUTPUT_ID;
      strcpy(item[nnodes], "OUT   ");
      nid = nnodes + 1;
    } else if (strstr(argv[2], "roi") != NULL) {    // collector
      strcpy(nodename, "roisender");
      idlist[0] = RF_ROI_ID;
      strcpy(item[0], "OUT   ");
      nid = 1;
    }
  } else {
#ifdef DESY
    // Special treatment for DESY test nodes!!
    strcpy(nodename, "evp_");
    char hostnamebuf[256];
    gethostname(hostnamebuf, sizeof(hostnamebuf));
    strcat(&nodename[4], &hostnamebuf[6]);
    int lend = strlen(nodename);
    nodename[lend + 1] = (char)0;
    nodename[lend] = nodename[lend - 1];
    strncpy(&nodename[lend - 1], "0", 1);
    // End of DESY special treatment
#else
    sprintf(nodename, "evp_%s", host);
#endif
    idlist[0] = 0;
    strcpy(item[0], "IN    ");
    idlist[1] = 1;
    strcpy(item[1], "OUT   ");
    nid = 2;
  }
  string shmname = string(conf.getconf("system", "unitname")) + ":" +
                   string(nodename);
  RFFlowStat flow((char*)shmname.c_str());

  printf("nodename = %s, nid = %d\n", nodename, nid);
  printf("id range : %d to %d\n", idlist[0], idlist[nid - 1]);
  //  printf ( "cell      nevt nque flowrate avesize evtrate elapsed\n" );
  //        123456789012345678901234567890123456789012345678901234567890
  printf("  At   :       Nevent     Nqueue  Flow(MB/s)  Size(KB)   Rate(Hz)\n");
  for (int i = 0; i < nid; i++) {
    RfShm_Cell& cell = flow.getinfo(idlist[i]);
    //    printf ( "cell %2.2d : %8d %4d %7.2f %7.2f %7.2f\n",
    //       idlist[i],
    //       cell.nevent, cell.nqueue, cell.flowrate, cell.avesize,
    //       cell.evtrate );
    printf("%s : %10d %10d %10.2f %10.2f %10.2f\n",
           item[i],
           cell.nevent, cell.nqueue, cell.flowrate, cell.avesize,
           cell.evtrate);
  }
}








