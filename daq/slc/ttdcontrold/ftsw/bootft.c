#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <ftsw.h>

#define VERSION 2011042200

int main(int argc, char **argv) {
  int verbose  = 0;
  int forced   = 0;
  int oflag    = FTSW_RDWR;
  int i, nftsw = 0;
  int id[16];
  ftsw_t *ftsw;
  int m012 = M012_SELECTMAP;

  while (argc > 3 || (argc > 1 && argv[1][0] == '-')) {
    if (strcmp(argv[1], "-v") == 0) {
      verbose = ! verbose;
    } else if (strcmp(argv[1], "-f") == 0) {
      forced = ! forced;
    } else if (strcmp(argv[1], "-s") == 0) {
      m012 = M012_SERIAL;
    } else if (strcmp(argv[1], "-p") == 0) {
      m012 = M012_SELECTMAP;
    } else if (argv[1][0] == '-' && isdigit(argv[1][1])) {
      if (nftsw == 16) {
        argc = 0;
        break; /* too many id's specified */
      }
      id[nftsw++] = atoi(&argv[1][1]);
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
 }
  if (argc < 2) {
    printf("bootft version %d\n", VERSION);
    printf("usage: bootft [-m] <filename>\n");
    printf("options:\n");
    printf(" -v      verbose mode\n");
    printf(" -s      serial mode\n");
    printf(" -p      parallel mode (default)\n");
    printf(" -f      forced (ignore errors)\n");
    printf(" -[<n>]  n-th FTSW\n");
    return 1;
  }
  if (nftsw == 0) {
    char *p = getenv("FTSW_DEFAULT");
    id[nftsw++] = p ? atoi(p) : 0;
  }

  for (i=0; i<nftsw; i++) {
    if (! (ftsw = ftsw_open(id[i], oflag))) {
      perror("open");
      return 1;
    }
    ftsw_boot_fpga(ftsw, argv[1], verbose, forced, m012);
  }
  return 0;
}
