/* ---------------------------------------------------------------------- *\

   bootmgt.c - boot MGT FINESSE prototype bitmap file

   Mikihiko Nakao, KEK IPNS

   2009042100  retry option
   2009101600  parallel version based on confmgt6.c
   2009112500  selectMAP fix (had to change M012 to 7 at the end)
   2009112501  merge serial and parallel modes

\* ---------------------------------------------------------------------- */

#define VERSION 2009112501
#define PROGRAM "bootmgt"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

#include "mgt.h"

/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int main(int argc, char** argv)
{
  int verbose  = 0;
  int forced   = 0;
  int ret;
  int mode = MGT_RDWR;
  int retry = 0;
  int i;
  int ch = -1; /* default: FINESSE A */
  int m012 = M012_SELECTMAP;
  Mgt_t* mgt;

  while (argc > 3 || (argc > 1 && argv[1][0] == '-')) {
    if (strcmp(argv[1], "-v") == 0) {
      verbose = ! verbose;
    } else if (strcmp(argv[1], "-m") == 0) {
      mode |= MGT_MMAP;
    } else if (strcmp(argv[1], "-f") == 0) {
      forced = ! forced;
    } else if (strcmp(argv[1], "-s") == 0) {
      m012 = M012_SERIAL;
    } else if (strcmp(argv[1], "-p") == 0) {
      m012 = M012_SELECTMAP;
    } else if (strncmp(argv[1], "-r", 2) == 0) {
      retry = atoi(&argv[1][2]);
    } else if (argv[1][0] == '-' && argv[1][1] >= 'a' && argv[1][1] <= 'd') {
      if (ch == -1) ch = 0;
      ch |= 1 << (argv[1][1] - 'a');
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
  }
  if (argc < 2) {
    printf("%s version %d\n", PROGRAM, VERSION);
    printf("usage: %s [-abcdfpsv] <filename>\n", PROGRAM);
    printf("options:\n");
    printf(" -a    config FINESSE A (default)\n");
    printf(" -b    config FINESSE B (more than one FINESSE)\n");
    printf(" -c    config FINESSE C (can be configured)\n");
    printf(" -d    config FINESSE D (at a time)\n");
    printf(" -v    verbose mode\n");
    printf(" -r<n> retry <n> times\n");
    printf(" -s    serial mode   (Slave serial,    M[2:0]=7)\n");
    printf(" -p    parallel mode (Slave selectMAP, M[2:0]=6, default)\n");
    printf(" -f    forced (ignore errors)\n");
    return 255;
  }

  if (ch < 0) ch = 1;

  for (i = 0; i < 4; i++) {
    if ((ch & (1 << i)) == 0) {
      continue;
    }

    do {
      if ((mgt = mgt_open(i, mode)) < 0) {
        if (retry-- > 0) {
          printf("retrying to open FINESSE-%d (%d)\n", 'a', retry);
          sleep(1);
        } else {
          perror("open");
          exit(255);
        }
      }
    } while (mgt < 0);

    do {
      ret = mgt_boot_fpga(mgt, argv[1], verbose, forced, m012);
      if (ret != 0) {
        if (retry-- > 0) {
          printf("retrying to boot (%d)\n", retry);
          sleep(1);
        } else {
          break;
        }
      }
    } while (ret != 0);
    mgt_close(mgt);
  }

  return -ret;
}
