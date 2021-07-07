/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <unistd.h>

#include <string>

extern "C" {
#include <nsm2/nsm2.h>
#include <nsm2/belle2nsm.h>
}

#include "daq/rfarm/manager/RFConf.h"

using namespace std;
using namespace Belle2;

static int done_flag;

void ok_handler(NSMmsg* msg, NSMcontext*)
{
  done_flag = 1;
  //  printf ( "OK received\n" );
}

void error_handler(NSMmsg* msg, NSMcontext*)
{
  done_flag = -1;
  printf("ERROR received\n");
}

int main(int argc, char** argv)
{
  // Usage : getconf conffile node

  if (argc < 4) {
    printf("Usage : rfcommand nodename command\n");
    exit(-1);
  }

  char* conffile = argv[1];
  char* nodename = argv[2];
  char* command = argv[3];

  char* confdir = getenv("RFARM_CONFDIR");
  string confpath = string(confdir) + "/" + string(conffile) + ".conf";

  RFConf conf(confpath.c_str());
  int port = conf.getconfi("system", "nsmport");

  // NSM initialization
  //  if (! b2nsm_init("COMMANDER")) {
  if (! b2nsm_init2("COMMANDER", 1, 0, port, 0)) {
    printf("rfcommand : initialization error %s\n",  b2nsm_strerror());
    return -1;
  }
  //  printf ( "initialization done\n" );

  // handlers
  if (b2nsm_callback("OK", ok_handler) < 0) {
    printf("rfcommand : error to hook CALLBACK(OK) %s", b2nsm_strerror());
    return -1;
  }
  if (b2nsm_callback("ERROR", ok_handler) < 0) {
    printf("rfcommand : error to hook CALLBACK(ERROR) %s", b2nsm_strerror());
    return -1;
  }


  // Send NSM request
  int* pars;
  done_flag = 0;
  b2nsm_sendreq(nodename, command, 0, pars);
  while (done_flag == 0) usleep(1000);
  return done_flag;
}

