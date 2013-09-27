//+
// File : readraw.cc
// Description : Read raw data dump file
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Sep - 2013
//-

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

using namespace std;

#define MAXBUF 4096000

int main(int argc, char** argv)
{
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    perror("fopen");
    exit(-1);
  }
  char* buf = new char[MAXBUF];

  int nrec = 0;
  for (;;) {
    int sizebuf;
    int sstat = read(fd, &sizebuf, sizeof(int));
    if (sstat <= 0) break;
    int stat = read(fd, buf, (sizebuf - 1) * 4);
    if (stat <= 0) break;
    printf("record %d: size = %d\n", nrec, sizebuf);
    nrec++;
  }

  close(fd);
}

