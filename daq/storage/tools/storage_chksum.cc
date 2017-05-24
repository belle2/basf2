#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <fstream>

#include <zlib.h>
#include <iostream>

int main(int argc, const char** argv)
{
  int fd = ::open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  int* buf = new int[10000000];
  uLong adler = adler32(0L, Z_NULL, 0);
  while (true) {
    int sstat = read(fd, buf, sizeof(int));
    if (sstat <= 0) {
      break;
    }
    unsigned int nbyte = buf[0];
    int rstat = read(fd, (buf + 1), nbyte - sizeof(int));
    adler = adler32(adler, (Bytef*)buf, nbyte);
  }
  printf("%x\n", adler);

  return 0;
}
