/*caMonitor.c*/

/* This example accepts the name of a file containing a list of pvs to monitor.
 * It prints a message for all ca events: connection, access rights and monitor.
 */

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

int main(int argc, char **argv)
{
  init_ca2nsm(argv[1]);
  SEVCHK(ca_pend_event(0.0),"ca_pend_event");
  return 0;
}
