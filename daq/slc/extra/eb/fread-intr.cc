#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <poll.h>

#include "strferror.h"

FILE* fp;

void
usr1_handler(int arg)
{
  fprintf(stderr, "got signal\n");

  pollfd fds[1];

  int fd = fileno(fp);

  fds[0].fd = fd;
  fds[0].events = POLLIN;

  fprintf(stderr, "checking %d...", fd);
  while (1) {
    int ret = poll(fds, 1, 0);
    if (ret < 0) {
      fprintf(stderr, "poll error\n");
      break;
    }

    if (!(fds[0].revents & POLLIN)) {
      fprintf(stderr, "EMPTY\n");
      break;
    }
    char ch;
    ret = read(fd, &ch, sizeof(ch));
    if (ret == 0) {
      /* EOF */
      fprintf(stderr, "EOF\n");
      break;
    }
    fputc('o', stderr);
  }

  fprintf(stderr, "closing fp...");
  fclose(fp);
  fprintf(stderr, "closed\n");
}

int
main(int argc, char** argv)
{
  fprintf(stderr, "pid = %d\n", getpid());
  while (1) {
    fprintf(stderr, "dup 0 to new fd...");
    int fd = dup(0);
    fprintf(stderr, "%d\n", fd);
    fp = fdopen(fd, "r");

    signal(SIGUSR1, usr1_handler);

    fprintf(stderr, "send SIGUSR1 to my pid (%d)\n", getpid());

    while (1) {
      char buffer[4];
      int ret;
      ret = fread(buffer, sizeof(buffer), 1, fp);

      fprintf(stderr, "fread returned %d\n", ret);

      if (ret != 1) {
        if (feof(fp)) {
          fputs("EOF\n", stderr);
        } else {
          fprintf(stderr, "ferror: %s\n", strferror(fp, "r"));
        }
        break;
      }
    }
  }

  exit(0);
}
