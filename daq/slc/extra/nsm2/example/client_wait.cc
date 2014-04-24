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
#include <string.h>
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
  for (int i = 0; i < 4; i++) {
    datap->nodes[i].evt_number = 0;
  }
  b2nsm_ok(msg, "RUNNING", "run %d started as %dth run",
           datap->run_number, datap->run_count);
}
// -- client_stop -------------------------------------------------------
//    callback function for the stop request
// ----------------------------------------------------------------------
void
client_stop(NSMmsg* msg, NSMcontext* nsmc)
{
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
  if (b2nsm_init2(nodename, 0, 0, 0, 0) < 0) {
    printf("%s: INIT %s", program, b2nsm_strerror());
    return 1;
  }
  printf("init done\n");

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

  static const char* nodes[] = { "aaa", "bbb", "ccc", "ddd" };
  for (int i = 0; i < 4; i++) {
    strcpy(datap->nodes[i].nodename, nodes[i]);
  }

  // INFINITE-LOOP
  while (1) {
    int ret;

    // wait for a message for up to 1 second
    ret = b2nsm_wait(1);
    if (ret < 0) {
      printf("%s: wait failure\n", program);
      break;
    } else if (ret == 0) { // timeout
      // and update the shared memory when it is not busy
      if (datap->is_running) {
        datap->nodes[datap->evt_total % 4].evt_number++;
        datap->evt_total++;
      }
    }
  }

  return 0;
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
