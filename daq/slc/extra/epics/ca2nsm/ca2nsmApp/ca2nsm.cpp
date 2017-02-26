/*ca2nsm.c*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "ca2nsm.h"
#include "ca2nsm_callback.h"

#include "daq/slc/system/Daemon.h"

int main(int argc, char **argv)
{
  if (Belle2::Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    init_ca2nsm(argv[1]);
    while (1) {
      //SEVCHK(
      ca_pend_event(0.1);
      //,"ca_pend_event");
      add_PVs();
    }
  }
  return 0;
}
