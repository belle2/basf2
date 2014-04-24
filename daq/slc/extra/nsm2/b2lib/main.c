#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "nsm2.h"
#include "belle2nsm.h"
#include "client_data.h"

/* -- online --------------------------------------------------------- */
void
online(NSMmsg *msg, NSMcontext *nsmc)
{
  printf("[online] is called back\n");
}
/* -- main ----------------------------------------------------------- */
int
main(int argc, char **argv)
{
  const char *nodename = argc > 1 ? argv[1] : "client";
  struct client_data *datp;

  if (! b2nsm_init(nodename)) {
    printf("b2nsm_init: %s\n", b2nsm_strerror());
    return 1;
  }
  printf("b2nsm_init done\n");

  if (strcmp(nodename, "client") == 0) {
    datp = b2nsm_allocmem("client_data", 0, client_data_revision, 3);
    printf("allocmem at %p\n", datp);
  }
  
  b2nsm_callback("ONLINE", online);
  
  printf("callback registered\n");
  
  sleep(3);
  if (strcmp(nodename, "foo") == 0) {
    datp = b2nsm_openmem("client_data", 0, client_data_revision);
    if (! datp) {
      printf("b2nsm_openmem: %s\n", b2nsm_strerror());
      return 1;
    }
    printf("openmem at %p\n", datp);
    
    printf("sending ONLINE\n");
    if (b2nsm_sendreq("client", "ONLINE", 0, 0) < 0) {
      printf("b2nsm_sendreq(client, ONLINE): %s\n", b2nsm_strerror());
      return 1;
    }
    printf("loop start\n");
    while (1) {
      if (datp) printf("datp->a = %d\n", datp->a);
      sleep(1);
    }
  } else if (strcmp(nodename, "client") == 0) {
    printf("loop start\n");
    while (1) {
      if (datp) datp->a++;
      sleep(1);
    }
  } else {
    while (1) sleep(1);
  }
  return 0; /* unreachable */
}
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
