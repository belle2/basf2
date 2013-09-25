/* ---------------------------------------------------------------------- *\

   regft.c - FTSW register access

   Mikihiko Nakao, KEK IPNS

   2010100700  first version
   2011071100  fancy things
   2011072000  sclk
   
\* ---------------------------------------------------------------------- */

#define VERSION 2011072000

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "ftsw.h"

char *ARGV0  = 0;
char *DEVICE = 0;
int verbose = 0;

/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int main(int argc, char **argv) {
  int offset = strtoul("450", 0, 16) >> 2;
  int value = strtoul("1", 0, 16);
  ftsw_t *ftsw;
  int i;//, ret;
  ftsw = ftsw_open(11, FTSW_RDWR);

  for (i = 0; 1/*i < 100000*/; i++ ) {
    usleep(10000);
    if (ftsw_write(ftsw, offset, value) < 0) {
      perror("write");
      return 1;
    }
  }
  return 0; 
}
