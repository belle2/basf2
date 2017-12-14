/* pxdpsMain.cpp */
/* Author:  Marty Kraimer Date:    17MAR2000 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "cadef.h"
#include "dbDefs.h"
#include "epicsString.h"
#include "cantProceed.h"

#include "pxdrc_callback.h"
#include <daq/slc/system/Daemon.h>

int main(int argc,char *argv[])
{
  if (Belle2::Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    init_pxdrc(argv[1]);
    while (1) {
      ca_pend_event(0.1);
    }
  }
  return(0);
}
