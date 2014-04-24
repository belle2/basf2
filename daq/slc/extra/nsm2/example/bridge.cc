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

#include "nsm2.h"
#include "belle2nsm.h"
#include "nsmlib2.h"
#include "bridge_data.h"

// -- global variables --------------------------------------------------
int local_nnode = 0;
char* local_nodes[256];
int  local_waiting[256];
int  local_nwaiting = 0;
int  local_lastreq = 0;
static NSMmsg lastmsg;
int  reqid_start;
int  reqid_stop;
int  reqid_ok;
int  reqid_error;

bridge_data* gdatap; // shared memory
NSMcontext* gnsm = 0;
NSMcontext* lnsm = 0;

// -- waiting_list ------------------------------------------------------
//    make a waiting list string
// ----------------------------------------------------------------------
const char*
waiting_list()
{
  static char buf[1024];
  int count = 0;
  buf[0] = 0;
  for (int i = 0; i < local_nnode; i++) {
    if (local_waiting[i]) {
      sprintf(buf + strlen(buf), " %s", local_nodes[i]);
      if (++count == 3) {
        strcat(buf, "...");
        break;
      }
    }
  }
  return buf;
}
// -- global_handler ----------------------------------------------------
//    callback function for the start or stop request
// ----------------------------------------------------------------------
void
global_handler(NSMmsg* msg, NSMcontext* nsmc)
{
  const char* reqname = msg->req == reqid_start ? "START" : "STOP";

  if (local_nwaiting) {
    b2nsm_error(msg, "%s message received, but%s %s still %s.",
                reqname, waiting_list(),
                local_nwaiting == 1 ? "is" : "are",
                lastmsg.req == reqid_start ? "starting" : "stopping");

  } else {
    printf("%s message received, distributing to %d local nodes...\n",
           reqname, local_nnode);
    b2nsm_context(lnsm);
    for (int i = 0; i < local_nnode; i++) {
      local_waiting[i] = 1;
      local_nwaiting++;
      printf("sending %s to %s...\n", reqname, local_nodes[i]);
      b2nsm_sendreq(local_nodes[i], reqname, msg->npar, msg->pars);
    }
    lastmsg = *msg;
    lastmsg.datap = 0; /* datap can't be used later anyway */
  }
}
// -- forget_handler ----------------------------------------------------
//    callback function for FORGET request
// ----------------------------------------------------------------------
void
forget_handler(NSMmsg* msg, NSMcontext* nsmc)
{
  const char* list = waiting_list();
  const char* req  = local_lastreq == reqid_start ? "START" : "STOP";
  memset(local_waiting, 0, sizeof(local_waiting));
  memset(&lastmsg, 0, sizeof(lastmsg));
  local_nwaiting = 0;

  if (! msg) {
    ;
  } else if (*list) {
    b2nsm_ok(msg, "READY", "nothing to forget");
  } else {
    b2nsm_ok(msg, "READY",
             "forgetting unanswered %s request from %s", req, list);
  }
}
// -- local_handler -----------------------------------------------------
//    callback function for the OK and ERROR request
// ----------------------------------------------------------------------
void
local_handler(NSMmsg* msg, NSMcontext* nsmc)
{
  const char* node = nsmlib_nodename(nsmc, msg->node);
  const char* req  = msg->req == reqid_ok ? "OK" : "ERROR";
  int found = -1;

  if (! lastmsg.req) {
    printf("%s received from %s (ignored)\n", req, node);
    return;
  }

  if (local_nwaiting) {
    for (int i = 0; i < local_nnode; i++) {
      if (strcasecmp(local_nodes[i], node) == 0) {
        if (local_waiting[i]) {
          local_waiting[i] = 0;
          local_nwaiting--;
          found = i;
        }
        break;
      }
    }
  }

  b2nsm_context(gnsm);

  if (msg->req != reqid_ok) {
    b2nsm_error(&lastmsg, "READY",
                "%s received from %s, no more waiting for %s",
                req, node, waiting_list());
    forget_handler(0, 0);
  } else if (found < 0) {
    printf("%s received from %s (ignored)\n", req, node);
  } else if (local_nwaiting) {
    printf("%s received from %s (still waiting for%s)\n",
           req, node, waiting_list());
  } else {
    printf("calling b2nsm_ok\n");
    b2nsm_ok(&lastmsg,
             lastmsg.req == reqid_start ? "RUNNING" : "READY",
             "everybody %s",
             lastmsg.req == reqid_start ? "started" : "stopped");
    forget_handler(0, 0);
  }
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
// -- register_callback -------------------------------------------------
//    set callback and error handling
// ----------------------------------------------------------------------
void
register_callback(const char* reqname, NSMcallback_t handler)
{
  if (b2nsm_callback(reqname, handler) < 0) {
    printf("callback(%s) %s", reqname, b2nsm_strerror());
    exit(1);
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
    for (int i = 0; i < local_nnode; i++)
      printf("node[%d] = %s\n", i, local_nodes[i]);
  }
  if (argc < 4 || global_port < 0 || local_port < 0) {
    printf("usage: %s <global-HNP> <local-HNP> %s\n",
           program, "<local-node1> [<local-node2> ...]");
    printf("(NHP: node-host-port in node@host:port format)\n");
    return 1;
  }

  // INIT with b2nsm_init2(node, usesig, host, port, shmkey)
  if (!(gnsm = b2nsm_init2(global_node, 0, global_host, global_port, 0))) {
    printf("%s: INIT-global %s", program, b2nsm_strerror());
    return 1;
  }
  if (!(lnsm = b2nsm_init2(local_node, 0, local_host, local_port, 0))) {
    printf("%s: INIT-local %s", program, b2nsm_strerror());
    return 1;
  }

  printf("global NSM2 context at %p, local NSM2 context at %p\n", gnsm, lnsm);

  // ALLOCATE shared memory (global)
  // (datap has to be allocated before callback registration
  const char* gdatname = global_node;
  const char* gfmtname = "bridge_data";
  b2nsm_context(gnsm);
  gdatap = (bridge_data*)b2nsm_allocmem(gdatname, gfmtname,
                                        bridge_data_revision, 3);
  if (! gdatap) {
    printf("%s: allocmem(%s) %s", program, gdatname, b2nsm_strerror());
    return 1;
  }

  // REGISTER callback functions (global)
  b2nsm_context(gnsm);
  register_callback("START",  global_handler);
  register_callback("STOP",   global_handler);
  register_callback("FORGET", global_handler);
  reqid_start = nsmlib_reqid(gnsm, "START");
  reqid_stop  = nsmlib_reqid(gnsm, "STOP");
  b2nsm_logging2(stdout, "G ");

  b2nsm_context(lnsm);
  register_callback("OK",    local_handler);
  register_callback("ERROR", local_handler);
  reqid_ok    = nsmlib_reqid(lnsm, "OK");
  reqid_error = nsmlib_reqid(lnsm, "ERROR");
  b2nsm_logging2(stdout, "L ");

  printf("initialized, entering an infinite loop...\n");

  // INFINITE-LOOP
  while (1) {
    int ret = b2nsm_wait(1);
  }

  // error if reached here
  printf("something wrong, exiting\n");

  return 0;
}
// ----------------------------------------------------------------------
// -- (emacs outline mode setup)
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^// --[+ ]" ***
// End: ***
