#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

main()
{
  int ret;
  addrinfo* info, hint;

  bzero(&hint, sizeof(hint));

  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_DGRAM;
  //hint.ai_protocol = IPPROTO_TCP;
  hint.ai_flags = 0;

  ret = getaddrinfo("127.0.0.1", "5001", &hint, &info);
  printf("%d %s\n", ret, gai_strerror(ret));

  exit(0);
}
