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

int main(int argc, char **argv)
{
  if (argc > 1) {
    init_ca2nsm(argv[1]);
    SEVCHK(ca_pend_event(0.0),"ca_pend_event");
  } else {
    printf("Usage %s : <config>", argv[0]);
  }
  return 0;
}
