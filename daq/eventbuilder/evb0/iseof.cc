#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "log.h"

int
main(int argc, char* argv[])
{
  int ret;
  char buffer[1];

  ret = recv(0, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT);
  switch (ret) {
    case 0: /* EOF */
      exit(0);
      break;
    case -1:
      if (errno == EAGAIN) {
        /* not EOF, no data in queue */
        exit(1);
      }
      log("recv: %s\n", strerror(errno));
      exit(1);
      break;
    default:
      exit(1);
      break;
  }
}
