// ----------------------------------------------------------------------
//  readdat.cc
//
//  A simple-minded NSM2 client program which does:
//  - open a shared memory data of client dat
//  - periodically printout updated info
// ----------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "belle2nsm.h"
#include "ca2nsm.h"

// -- global variables --------------------------------------------------
struct ca2nsm* datap = 0;  // shared memory
uint32 timestamp[100];

// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program  = argv[0];
  const char* nodename = argv[1]; // need to check before using
  const char* datname  = argv[2]; // need to check before using
  const char* fmtname = "ca2nsm";
  int ret;
  memset(timestamp, 0, sizeof(timestamp));

  // ARGV check
  if (argc < 3) {
    printf("usage: %s <nodename> <datanme>\n", program);
    return 1;
  }

  // INIT
  if (! b2nsm_init(nodename)) {
    printf("%s: INIT %s\n", program, b2nsm_strerror());
    return 1;
  }

  // OPEN shared memory
  // (datap has to be allocated before callback registration
  datap = (struct ca2nsm*) b2nsm_openmem(datname, fmtname, ca2nsm_revision);
  if (! datap) {
    printf("%s: openmem %s\n", program, b2nsm_strerror());
    return 1;
  }

  {
    int pars = 3;
    const char* pvname = "PXD:rc:state";
    int len = strlen(pvname);
    b2nsm_sendany("CA2NSM", "VGET", 1, &pars, len, pvname, NULL);
  }
  {
    int pars = 3;
    const char* pvname = "PXD:rc:request";
    int len = strlen(pvname);
    b2nsm_sendany("CA2NSM", "VGET", 1, &pars, len, pvname, NULL);
  }

  // INFINITE-LOOP
  while (1) {
    for (int i = 0; i < 100; i++) {
      ca2nsm::pv_info& info(datap->pv[i]);
      if (info.chid > 0 && info.timestamp > timestamp[i]) {
        timestamp[i] = info.timestamp;
        printf("%d %s = %s\n", info.chid, info.name, info.data);
      }
    }
    usleep(10000);
  }

  return 0; // never reached
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
