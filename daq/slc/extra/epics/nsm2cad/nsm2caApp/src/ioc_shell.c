#include <epicsExport.h>
#include <iocsh.h>

#include "devNSM2CA.h"

/** Configuration for NSM IOC Shell.
 *  * Copied from
 *  * https://wiki-ext.aps.anl.gov/epics/index.php/How_to_make_your_EPICS_driver_operating_system_independent
 *  */

/** NSM Configuration **/
/* parameters */
static const iocshArg nsmConfigArg0 = {"node", iocshArgString}; /* node name */
static const iocshArg nsmConfigArg1 = {"host", iocshArgString}; /* host name */
static const iocshArg nsmConfigArg2 = {"port", iocshArgInt}; /* port number */
static const iocshArg *nsmConfigArgs[] = {&nsmConfigArg0, &nsmConfigArg1, &nsmConfigArg2};
/* command */
static const iocshFuncDef nsmConfigFuncDef = {"nsmConfig", 3 /* # parameters */, nsmConfigArgs};

static void nsmConfigCallFunc(const iocshArgBuf *args) {
  const char * const node = args[0].sval;
  const char * const host = args[1].sval;
  const int port = args[2].ival;
  init_nsm2(node, host, port);
}

void NSMRegisterCommands(void) {
  static int firstTime = 1;
  if (firstTime) {
    iocshRegister(&nsmConfigFuncDef, nsmConfigCallFunc);
    firstTime = 0;
  }
}

/** Register all Commands **/
epicsExportRegistrar(NSMRegisterCommands);

