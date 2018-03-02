#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include "nsm2.h"
#include "belle2nsm.h"

std::string g_state = "NOTREADY";

static void reply(NSMmsg* msg, NSMcontext* msnc, const char* state = NULL)
{
  if (state != NULL) {
    g_state = state;
    printf("set state : %s\n", g_state.c_str());
  }
  b2nsm_ok(msg, g_state.c_str(), NULL);
}

void client_start(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("%s\n", __FUNCTION__);
  printf("run # : %04d.%06d\n", msg->pars[0], msg->pars[1]);
  reply(msg, nsmc, "RUNNING");
}

void client_stop(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("%s\n", __FUNCTION__);
  if (g_state == "RUNNING") {
    reply(msg, nsmc, "READY");
  } else {
    reply(msg, nsmc);
  }
}

void client_trigft(NSMmsg* msg, NSMcontext* nsmc)
{
  printf("%s\n", __FUNCTION__);
  const char* cmd[] = {"none", "aux", "i", "tlu",
                       "pulse", "revo", "random",
                       "possion", "once", "stop"
                      };
  printf("trigger_type : %s\n", cmd[msg->pars[0]]);
  printf("dummy_rate %d\n", msg->pars[1]);
  printf("trigger_limit %d\n", msg->pars[2]);
  reply(msg, nsmc);
}

void client_statecheck(NSMmsg* msg, NSMcontext* nsmc)
{
  reply(msg, nsmc);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("usage: %s <nodename>\n", argv[0]);
    return 1;
  }
  const char* nodename = argv[1]; // need to check before using

  b2nsm_logging(stdout);
  if (b2nsm_init2(nodename, 1, 0, 0, 0) < 0) {
    printf("%s: INIT %s", argv[0], b2nsm_strerror());
    return 1;
  }

  if (b2nsm_callback("START", client_start) < 0) {
    return 1;
  }
  if (b2nsm_callback("STOP", client_stop) < 0) {
    return 1;
  }
  if (b2nsm_callback("STATECHECK", client_statecheck) < 0) {
    return 1;
  }
  if (b2nsm_callback("TRIGFT", client_trigft) < 0) {
    return 1;
  }

  while (1) sleep(1);

  return 0;
}
