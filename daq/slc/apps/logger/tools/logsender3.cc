#include <stdio.h>
#include <unistd.h>
#include <nsm2/nsm2.h>
#include <nsm2/belle2nsm.h>
#include <time.h>
#include <string.h>

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage : ./logsender3 <name> <priority> <message>\n");
    return 1;
  }
  const char* nodename = argv[1];
  const char* priority_s = argv[2];
  const char* message = argv[3];
  int priority = 1;// DEBUG
  if (strcmp(priority_s, "INFO") == 0) {
    priority = 2;// INFO
  } else if (strcmp(priority_s, "NOTICE") == 0) {
    priority = 3;// NOTICE
  } else if (strcmp(priority_s, "WARNING") == 0) {
    priority = 4;// WARNING
  } else if (strcmp(priority_s, "ERROR") == 0) {
    priority = 5;// ERROR
  } else if (strcmp(priority_s, "FATAL") == 0) {
    priority = 6;// FATAL
  }
  NSMcontext* nsmc = NULL;
  if ((nsmc = b2nsm_init(nodename)) <= 0) {
    printf("[FATAL] Failed to connect NSM : %s", b2nsm_strerror());
    return 1;
  }
  int date = time(NULL);
  int pars[3] = {priority, date, 0};
  int len = strlen(message);
  if (b2nsm_sendany("LOGGER", "LOG",
                    3, (int*)pars, len, message, NULL) <= 0) {
    printf("Failed\n");
  } else {
    printf("Succeded\n");
  }
  return 0;
}
