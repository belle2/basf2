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
#include <unistd.h>
#include "nsm2.h"
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

  if (! datap) {
    b2nsm_error(msg, "shared data is not allocated yet");
    return;
  }
  if (datap->is_running) {
    b2nsm_error(msg, "already running");
    return;
  }
  if (msg->npar < 1) {
    b2nsm_error(msg, "run number is missing");
    return;
  }
  datap->is_running = 1;
  datap->run_count++;
  datap->run_number = msg->pars[0];
  datap->evt_number = 0;
  b2nsm_ok(msg, "RUNNING\nRunning", "run %d started as %dth run\n test",
           datap->run_number, datap->run_count);
}
// -- client_stop -------------------------------------------------------
//    callback function for the stop request
// ----------------------------------------------------------------------
void
client_stop(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("STOP message received\n");

  if (! datap) {
    b2nsm_error(msg, "shared data is not allocated yet");
    return;
  }
  if (! datap->is_running) {
    b2nsm_error(msg, "not running");
    return;
  }
  b2nsm_ok(msg, "IDLE", "run %d started as %dth run",
           datap->run_number, datap->run_count);
  datap->is_running = 0;
  datap->run_number = -1;
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

  // INIT
  if (! b2nsm_init(nodename)) {
    printf("%s: b2nsm_init %s\n", program, b2nsm_strerror());
    return 1;
  }

  // log message to standard output
  b2nsm_logging(stdout);

  // ALLOCATE shared memory
  // (datap has to be allocated before callback registration
  datap = (struct client_data*)
          b2nsm_allocmem(datname, fmtname, client_data_revision, 3);
  if (! datap) {
    printf("%s: allocmem %s\n", program, b2nsm_strerror());
    return 1;
  }

  // REGISTER callback functions
  if (b2nsm_callback("START", client_start) < 0) {
    printf("%s: callback(START) %s\n", program, b2nsm_strerror());
    return 1;
  }
  if (b2nsm_callback("STOP", client_stop) < 0) {
    printf("%s: callback(START) %s\n", program, b2nsm_strerror());
    return 1;
  }

  // INFINITE-LOOP
  while (1) {
    if (datap->is_running) {
      datap->evt_number++;
      datap->evt_total++;
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
