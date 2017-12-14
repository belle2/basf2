#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <assert.h>

int main(int arg, char** argv)
{
  addrinfo* res = NULL, hint;

  memset(&hint, 0, sizeof(hint));

  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;
  /* hint.ai_flags = AI_PASSIVE; */

  int ret;

  ret = getaddrinfo("127.0.0.1", "5001", &hint, &res);
  assert(ret == 0);

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  assert(s > 0);

  ret = bind(s, res->ai_addr, res->ai_addrlen);
  assert(ret == 0);

  ret = listen(s, 5);
  assert(ret == 0);

  int ss = accept(s, NULL, NULL);
  assert(ss >= 0);

  int one = 1;
#ifdef SOL_SOCKET
  ret = setsockopt(ss, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
  assert(ret == 0);
#endif

#ifdef SOL_TCP
  ret = setsockopt(ss, SOL_TCP, TCP_KEEPIDEL, &one, sizeof(one));
  assert(ret == 0);
  ret = setsockopt(ss, SOL_TCP, TCP_KEEPINTVL, &one, sizeof(one));
  assert(ret == 0);
  ret = setsockopt(ss, SOL_TCP, TCP_KEEPCNT, &one, sizeof(one));
  assert(ret == 0);
#endif

#if 0
  ret = shutdown(ss, SHUT_RD);
  assert(ret == 0);
#endif

  close(s);

  pollfd fds[1];
  memset(fds, 0, sizeof(fds));
  fds[0].fd = ss;
  fds[0].events = POLLIN | POLLOUT;
  fds[0].revents = 0;

  for (int i = 0; i < 30; i++) {
    ret = poll(fds, 1, 0);
    fprintf(stderr, "ret = %d events = %x revents = %x\n", i, fds[0].events, fds[0].revents);
    if (fds[0].revents & POLLIN) {
      char buffer[1];
      if (0 == recv(ss, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT)) {
        fprintf(stderr, "socket is closed!\n");
        break;
      }
    }
    sleep(1);
  }
  close(ss);
}
