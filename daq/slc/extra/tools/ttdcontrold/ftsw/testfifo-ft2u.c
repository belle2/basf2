/* ---------------------------------------------------------------------- *\

   testfifo-ft2u.c

   Mikihiko Nakao, KEK IPNS

   2013090600  new
   
\* ---------------------------------------------------------------------- */

#define VERSION 2013090600

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "ftsw.h"

char *ARGV0  = 0;
char *DEVICE = 0;
int verbose = 0;

/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  char *p = getenv("FTSW_DEFAULT");
  int ftid = p ? atoi(p) : 0;
  ftsw_t *ftsw;
  int i;
  int ret;
  int val1;
  int val2;
  int nevent = 0;

  while (argc > 1 && argv[1][0] == '-') {
    if (isdigit(argv[1][1])) {
      ftid = atoi(&argv[1][1]);
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
  }
  if (argc > 1) nevent = atoi(argv[1]);

  if (argc < 2 || ftid == 0 || nevent == 0) {
    printf("usage: testhead-ft2u -<n> <num>\n");
    printf("options:\n");
    printf(" -<n>    select <n>-th FTSW (<n>=1,2,3,4)\n");
    printf(" <num>   number of events\n");
    return 1;
  }

  ftsw = open_ftsw(ftid, FTSW_RDONLY);
  
  if (! ftsw) {
    perror("open");
    printf("FTSW is not found.\n");
    exit(1);
  }

  for (i=0; i<nevent; i++) {
    while (1) {
      int stafifo = read_ftsw(ftsw, FTSWREG_STAFIFO);
      if ((stafifo & 0x10000000) == 0) break;
    }
    val1 = read_ftsw(ftsw, FTSWREG_FIFO);
    val2 = read_ftsw(ftsw, FTSWREG_FIFO);
    printf("%08x %08x\n", val1, val2);
  }
  
  return 0; 
}
