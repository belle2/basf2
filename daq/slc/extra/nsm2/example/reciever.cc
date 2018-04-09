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
#include <unistd.h>
#include <errno.h>

extern "C" {
#include "nsmlib2.h"
#include "belle2nsm.h"
#include "client_data.h"
#include "bridge_data.h"
}

int try_read(int sock, char* buf, int datalen)
{
  int recvlen = 0;
  while (recvlen < datalen) {
    int ret;
    if ((ret = read(sock, buf + recvlen, datalen)) <= 0) {
      if (ret < 0) {
        perror("EINTER");
        return -1;
      }
    }
    datalen -= ret;
    recvlen += ret;
  }
  return recvlen;
}

// -- main --------------------------------------------------------------
//    main does everything except callback functions
// ----------------------------------------------------------------------
int
main(int argc, char** argv)
{
  const char* program = argv[0];
  const char nodename[32] = "R";
  char local_host[256];
  int  local_port;
  int  local_nnode;
  char* local_nodes[256];
  NSMcontext* lnsm = 0;
  int ret;

  if (!(lnsm = b2nsm_init(nodename))) {
    printf("%s: INIT-local %s\n", program, nsmlib_strerror(lnsm));
    return 1;
  }
  nsmlib_usesig(lnsm, 0);

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
  //b2nsm_context(lnsm);

  // INFINITE-LOOP
  NSMcontext* nsmlist[3];
  nsmlist[0] = lnsm;
  nsmlist[1] = 0;

  char buf[NSM_TCPMSGSIZ];
  int pars[256];
  while (1) {
    NSMcontext* nsmc = nsmlib_selectc(0, 5);//2, nsmlist, 0.1);
    if (nsmc == (NSMcontext*) - 1) {
    } else if (nsmc == lnsm) {
      NSMtcphead hp;
      int sock = nsmc->sock;
      int datalen = sizeof(NSMtcphead);
      if (try_read(sock, (char*)&hp, datalen) < 0) {
        return 1;
      }
      NSMmsg msg;
      msg.req  = ntohs(hp.req);
      msg.seq  = ntohs(hp.seq);
      msg.node = ntohs(hp.src);
      msg.npar = hp.npar;
      msg.len  = ntohs(hp.len);
      msg.pars[0] = msg.pars[1] = 0; /* to be compatible with NSM1 */
      datalen = sizeof(int32) * msg.npar;
      if (try_read(sock, (char*)(msg.pars), datalen) < 0) {
        return 1;
      }
      for (int i = 0; i < msg.npar; i++) {
        msg.pars[i] = ntohl(msg.pars[i]);
      }
      datalen = msg.len;
      memset(buf, 0, sizeof(buf));
      if (try_read(sock, buf, datalen) < 0) {
        return 1;
      }
      msg.datap = msg.len > 0 ? buf : 0;
      printf("msg.npar=%d\n", msg.npar);
      printf("msg.pars[0]=%d\n", msg.pars[0]);
      printf("msg.len=%d\n", msg.len);
      printf("msg.datap=%s\n", (const char*)msg.datap);
    } else { // time-out
      printf("time-out\n");
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
