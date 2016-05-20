/* rcbridgedMain.cpp */
/* Author:  Marty Kraimer Date:    17MAR2000 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "epicsExit.h"
#include "epicsThread.h"
#include "iocsh.h"

int main(int argc,char *argv[])
{
  /*
  if (Belle2::Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    init_pxdrc(argv[1]);
    while (1) {
      ca_pend_event(0.1);
    }
  }
  */

  if(argc >= 2) {    
    iocsh(argv[1]);
    epicsThreadSleep(.2);
  }
  iocsh(NULL);
  epicsExit(0);
  return(0);
}
