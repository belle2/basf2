/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

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

