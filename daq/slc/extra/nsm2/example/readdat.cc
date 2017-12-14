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
#include "client_data.h"

// -- global variables --------------------------------------------------
struct client_data* datap = 0;  // shared memory

// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program  = argv[0];
  const char* nodename = argv[1]; // need to check before using
  const char* datname  = argv[2]; // need to check before using
  const char* fmtname = "client_data";
  int ret;

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
  datap = (struct client_data*)
          //b2nsm_openmem(datname, fmtname, client_data_revision);
          b2nsm_openmem(datname, fmtname, -1);
  if (! datap) {
    printf("%s: openmem %s\n", program, b2nsm_strerror());
    return 1;
  }

  // INFINITE-LOOP
  while (1) {
    static struct client_data databuf;
    if (memcmp(&databuf, datap, sizeof(databuf)) != 0) {
      printf("%s run=%d (total %d) evt=%s:%d/%s:%d/%s:%d/%s:%d (total %d)\n",
             datap->is_running ? "RUNNING" : "IDLE",
             datap->run_number,
             datap->run_count,
             datap->nodes[0].nodename,
             datap->nodes[0].evt_number,
             datap->nodes[1].nodename,
             datap->nodes[1].evt_number,
             datap->nodes[2].nodename,
             datap->nodes[2].evt_number,
             datap->nodes[3].nodename,
             datap->nodes[3].evt_number,
             datap->evt_total);

      databuf = *datap; // update the local copy
    }
    sleep(1);
  }

  return 0; // never reached
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
