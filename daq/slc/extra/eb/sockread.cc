#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <netdb.h>

int
main(int argc, char** argv)
{
  addrinfo* res = NULL, hint;
  memset(&hint, 0, sizeof(hint));

  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  int ret;

  ret = getaddrinfo("localhost", "5001", &hint, &res);
  assert(ret == 0);

  int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  assert(s >= 0);

  ret = connect(s, res->ai_addr, res->ai_addrlen);
  assert(ret == 0);

#if 0
  ret = shutdown(s, SHUT_WR);
  assert(ret == 0);
#endif

  for (int i = 0; i < 5; i++) {
    sleep(1);
  }

  close(s);
}
