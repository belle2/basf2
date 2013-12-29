// ----------------------------------------------------------------------
//  bridge.cc
//
//  A simple-minded NSM2 bridge program to connect "global-net"
//  and "local-net".
//
//  - receive start/stop message from global-master
//  - distribute start/stop message to local-clients
//  - collect ok/error message from nodes in local-clients
//  - send back ok/error message to global-master
//  - collect log messages and send it to the global-master
//    (no screen output as it conflicts with readline)
// ----------------------------------------------------------------------
#include <stdio.h>
#include "nsm2.h"

// -- global variables --------------------------------------------------
int is_running = 0;
client_data* datap;  // shared memory

// -- client_start ------------------------------------------------------
//    callback function for the start request
// ----------------------------------------------------------------------
int
client_start(NSMmsg* msg, NSMcontext2* nsmc)
{
  if (datap->is_running) {
    nsm_error(msg.src, "already running");
    return;
  }
  if (msg.npar < 1) {
    nsm_error(msg.src, "run number is missing");
    return;
  }
  datap->is_running = 1;
  datap->run_count++;
  datap->run_number = action.par[0];
  datap->evt_number = 0;
  nsm_ok(msg.src, "run %d started as %dth run", run_number, run_count);
  return 0;
}
// -- client_stop -------------------------------------------------------
//    callback function for the stop request
// ----------------------------------------------------------------------
int
client_stop(NSMmsg* msg, NSMcontext2* nsmc)
{
  if (! is_running) {
    nsm_error(msg.src, "not running");
    return;
  }
  nsm_ok(msg.src, "run %d started as %dth run", run_number, run_count);
  datap->is_running = 0;
  datap->run_number = -1;
  return 0;

}
// -- nodehostport ------------------------------------------------------
//
// - limiting to strlen(node) < 32, strlen(host) < 256
// - node may be a null pointer if not needed.
// - port may be missing for a default port
// - return -1 if bad format
// ----------------------------------------------------------------------
int
nodehostport(const char* str, char* node, char* host)
{
  const char* p;
  const char* q;

  if (! str || ! host) return -1;

  if (node) {
    *node = 0;
    if (!(p = strchr(str, '@'))) return -1;
    if (p - str >= 32) return -1;
    strncpy(node, str, p - str);
    node[p - str] = 0;
    p++;
  } else {
    *node = 0;
    p = str;
  }

  if (q = strchr(p, ':')) {
    if (q - p >= 256) return -1;
    strncpy(host, p, q - p);
    host[q - p] = 0;
    q++;
    if (! isdigit(q)) return -1;
    return atoi(q);
  } else if (strlen(p) >= 256) {
    return -1;
  } else {
    strcpy(host, p);
    return 0;
  }
}
// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program = argv[0];
  char global_node[32];
  char global_host[256];
  int  global_port;
  char local_node[32];
  char local_host[256];
  int  local_port;
  int  local_nnode;
  char* local_nodes[256];
  NSMcontext2* gnsm = 0;
  NSMcontext2* lnsm = 0;
  int ret;

  // ARGV check
  // at least 4 args
  // global-NHP local-NHP local-node1 local-node2 ...
  if (argc > 4) {
    global_port = nodehostport(argv[1], global_node, global_host);
    local_port  = nodehostport(argv[2], local_node,  local_host);
    local_nnode = argc - 3;
    for (int i = 0; i < local_nnode; i++) local_nodes[i] = argv[3 + i];
    local_nodes[local_nnode] = 0;
  }
  if (argc < 4 || global_port < 0 || local_port < 0) {
    printf("usage: %s <global-HNP> <local-HNP> <local-node1> [<local-node2> ...]\n",
           program);
    printf("(NHP: node-host-port in node@host:port format)\n");
    return 1;
  }

  // INIT
  if (!(gnsm = nsm_init2(global_node, global_host, global_port, 0))) {
    printf("%s: INIT-global %s", program, nsm_strerror());
    return 1;
  }
  if (!(lnsm = nsm_init2(local_node, local_host, local_port, 0))) {
    printf("%s: INIT-local %s", program, nsm_strerror());
    return 1;
  }

  // ALLOCATE shared memory (global)
  // (datap has to be allocated before callback registration
  const char* gdatname = global_node;
  const char* gfmtname = "bridge_data";
  nsm_context(gnsm);
  gdatap = nsm_allocmem(gdatname, gfmtname, bridge_data_revision, 3);
  if (! gdatap) {
    printf("%s: allocmem(%s) %s", program, gdatname, nsm_strerror());
    return 1;
  }

  // REGISTER callback functions (global)
  nsm_context(gnsm);
  if (nsm_callback("START", 0) < 0) {
    printf("%s: callback(START) %s", program, nsm_strerror());
    return 1;
  }
  if (nsm_callback("STOP", 0) < 0) {
    printf("%s: callback(STOP) %s", program, nsm_strerror());
    return 1;
  }
  nsm_context(gnsm);

  // INFINITE-LOOP
  NSMcontext2* nsmlist[3];
  nsmlist[0] = gnsm;
  nsmlist[1] = lnsm;
  nsmlist[2] = 0;

  while (1) {
    NSMcontext2* nsm = nsm_select(2, nsmlist, 0.1);
    if (nsm == (NSMcontext2*) - 1) {
    } else if (nsm == gnsm) {

    } else if (nsm == lnsm) {

    } else { // time-out
    }
  }

  return 0; // never reached
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
