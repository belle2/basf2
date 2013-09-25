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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "nsmlib2.h"
#include "belle2nsm.h"
#include "client_data.h"
#include "bridge_data.h"
}

// -- global variables --------------------------------------------------
int is_running = 0;
client_data* datap;  // shared memory

// -- client_start ------------------------------------------------------
//    callback function for the start request
// ----------------------------------------------------------------------
int
client_start(NSMmsg* msg, NSMcontext* nsmc)
{
  if (datap->is_running) {
    b2nsm_error(msg, "already running");
    return 1;
  }
  if (msg->npar < 1) {
    b2nsm_error(msg, "run number is missing");
    return 1;
  }
  datap->is_running = 1;
  datap->run_count++;
  datap->run_number = msg->pars[0];
  datap->evt_number = 0;
  b2nsm_ok(msg, "RUNNING", "run %d started as %dth run",
           datap->run_number, datap->run_count);
  return 0;
}
// -- client_stop -------------------------------------------------------
//    callback function for the stop request
// ----------------------------------------------------------------------
int
client_stop(NSMmsg* msg, NSMcontext* nsmc)
{
  if (! is_running) {
    b2nsm_error(msg, "not running");
    return 1;
  }
  b2nsm_ok(msg, "IDLE", "run %d started as %dth run",
           datap->run_number, datap->run_count);
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
    if (! isdigit(*q)) return -1;
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
  NSMcontext* gnsm = 0;
  NSMcontext* lnsm = 0;
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
  if (argc < 5 || global_port < 0 || local_port < 0) {
    printf("usage: %s <global-HNP> <local-HNP> <node1> <node2> ...\n",
           program);
    printf("(NHP: node-host-port in node@host:port format)\n");
    return 1;
  }

  printf("global: (%s, %s, %d)\n", global_node, global_host, global_port);
  // INIT
  if (!(gnsm = nsmlib_init(global_node, global_host, global_port, 8122))) {
    printf("%s: INIT-global %s\n", program, nsmlib_strerror(gnsm));
    return 1;
  }
  nsmlib_usesig(gnsm, 0);
  printf("global: (%s, %s, %d)\n", local_node, local_host, local_port);
  if (!(lnsm = nsmlib_init(local_node, local_host, local_port, 7777))) {
    printf("%s: INIT-local %s\n", program, nsmlib_strerror(lnsm));
    return 1;
  }
  nsmlib_usesig(lnsm, 0);

  // ALLOCATE shared memory (global)
  // (datap has to be allocated before callback registration
  const char* gdatname = global_node;
  const char* gfmtname = "bridge_data";
  b2nsm_context(gnsm);
  datap = (client_data*)b2nsm_allocmem(gdatname, gfmtname, bridge_data_revision, 3);
  if (! datap) {
    printf("%s: allocmem(%s) %s", program, gdatname, b2nsm_strerror());
    return 1;
  }

  // REGISTER callback functions (global)
  b2nsm_context(gnsm);
  if (b2nsm_callback("START", 0) < 0) {
    printf("%s: callback(START) %s", program, b2nsm_strerror());
    return 1;
  }
  if (b2nsm_callback("STOP", 0) < 0) {
    printf("%s: callback(STOP) %s", program, b2nsm_strerror());
    return 1;
  }
  b2nsm_context(gnsm);

  // REGISTER callback functions (global)
  b2nsm_context(lnsm);
  if (b2nsm_callback("START", 0) < 0) {
    printf("%s: callback(START) %s", program, b2nsm_strerror());
    return 1;
  }
  if (b2nsm_callback("STOP", 0) < 0) {
    printf("%s: callback(STOP) %s", program, b2nsm_strerror());
    return 1;
  }
  b2nsm_context(lnsm);

  // INFINITE-LOOP
  NSMcontext* nsmlist[3];
  nsmlist[0] = gnsm;
  nsmlist[1] = lnsm;
  nsmlist[2] = 0;

  char buf[NSM_TCPMSGSIZ];
  while (1) {
    NSMcontext* nsm = nsmlib_selectc(0, 5);//2, nsmlist, 0.1);
    if (nsm == (NSMcontext*) - 1) {
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
