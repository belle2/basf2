// ----------------------------------------------------------------------
//  client.cc
//
//  A simple-minded NSM2 client program which does:
//  - accept START and STOP request
//  - return OK request
//  - allocate a shared memory
//  - update run number and time on a shared memory
//  - periodically send INFO message
// ----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "nsm2.h"
#include "nsmlib2.h"
#include "belle2nsm.h"
#include "client_data.h"

// -- global variables --------------------------------------------------
struct client_data* datap = 0;  // shared memory

// -- client_start ------------------------------------------------------
//    callback function for the start request
// ----------------------------------------------------------------------
void
client_start(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("START message received\n");
  sleep(2);
  b2nsm_ok(msg, "RUNNING", "started");
}
// -- client_stop -------------------------------------------------------
//    callback function for the stop request
// ----------------------------------------------------------------------
void
client_stop(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("STOP message received\n");
  sleep(2);
  b2nsm_ok(msg, "READY", "stopped");
}
// -- client_start ------------------------------------------------------
//    callback function for the start request
// ----------------------------------------------------------------------
void
client_load(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("LOAD message received\n");
  sleep(2);
  b2nsm_ok(msg, "READY", "loaded");
}
// -- client_abort -------------------------------------------------------
//    callback function for the abort request
// ----------------------------------------------------------------------
void
client_abort(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("ABORT message received\n");
  sleep(2);
  b2nsm_ok(msg, "NOTREADY", "abortped");
}
// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program  = argv[0];
  const char* nodename = argv[1]; // need to check before using
  const char* datname = nodename;
  const char* fmtname = "client_data";
  int ret;

  // ARGV check
  if (argc < 2) {
    printf("usage: %s <nodename>\n", program);
    return 1;
  }

  // debug level
  if (argc > 2 && strncmp(argv[2], "-d", 2) == 0) {
    b2nsm_debuglevel(isdigit(argv[2][2]) ? atoi(&argv[2][2]) : 1);
  }

  // INIT
  if (! b2nsm_init(nodename)) {
    printf("%s(b2nsm_init): %s\n", program, b2nsm_strerror());
    return 1;
  }

  datap = (struct client_data*)
          b2nsm_allocmem(datname, fmtname, client_data_revision, 3);
  if (! datap) {
    printf("%s: allocmem %s\n", program, b2nsm_strerror());
    return 1;
  }

  // log message to standard output
  b2nsm_logging(stdout);

  // REGISTER callback functions
  if (b2nsm_callback("START", client_start) < 0) {
    return 1; /* error message is already printed by b2nsm_logging */
  }
  if (b2nsm_callback("STOP", client_stop) < 0) {
    return 1; /* error message is already printed by b2nsm_logging */
  }
  if (b2nsm_callback("LOAD", client_load) < 0) {
    return 1; /* error message is already printed by b2nsm_logging */
  }
  if (b2nsm_callback("ABORT", client_abort) < 0) {
    return 1; /* error message is already printed by b2nsm_logging */
  }

  // INFINITE-LOOP
  while (1) {
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
