/* ---------------------------------------------------------------------- *\
   regrx.c

   show / modify TT-RX registers

   Mikihiko Nakao, KEK IPNS

   2005021501  ttrxlib version
   2005072300  backport from tt-io
   2005081300  u* reorganization
\* ---------------------------------------------------------------------- */

static char VERSION[] = "2005081200";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#include "ttrx.h"

/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int readonly = 0;
  int mmapmode = 0;
  int verbose = 0;
  int rxid = 0;
  int offset = -1;
  int value;
  int i, ret;
  ttrx_t *ttrx = 0;
  static struct { char *name; int offset; } regs[] = {
    { "pid",   TTRX_PID },
    { "pver",  TTRX_PVER },
    { "prgm",  TTRX_PRGM },
    { "sclk",  TTRX_SCLK },
    { "id",    TTRX_ID },
    { "ver",   TTRX_VER },
    { "csr",   TTRX_CSR },
    { "clk",   TTRX_CLK },
    { "intr",  TTRX_INTR },
    { "trig",  TTRX_TRIG },
    { "tlast", TTRX_TLAST },
    { "rate",  TTRX_RATE },
    { "tag",   TTRX_TAG },
    { "tagr",  TTRX_TAGR },
    { "depth", TTRX_DEPTH },
    { "fifo1", TTRX_FIFO1 },
    { "fifo2", TTRX_FIFO2 },
    { "ustat", TTRX_USTAT },
    { "uerrs", TTRX_UERRS },
    { "ucmd",  TTRX_UCMD },
    { "uchk",  TTRX_UCHK },
    { "uraw",  TTRX_URAW },
    { "ext",   TTRX_EXT } };

  while (argc > 1 && argv[1][0] == '-') {
    if (argv[1][0] == '-' && argv[1][1] == 'v') {
      verbose = ! verbose;
    } else if (argv[1][0] == '-' && argv[1][1] == 'r') {
      readonly = ! readonly;
    } else if (argv[1][0] == '-' && argv[1][1] == 'm') {
      mmapmode = ! mmapmode;
    } else if (argv[1][0] == '-' && isdigit(argv[1][1])) {
      rxid = argv[1][1] - '0';
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
  }
  if (argc < 2) {
    printf("usage: regrx [-<n>] [-v] [-m] <reg> [<value> [<reg> <value>]...]\n");
    printf("usage: regrx [-<n>] [-v] [-m] -r <reg> [<reg> ...]\n");
    printf("options:\n");
    printf(" -<n>       select <n>-th TT-RX (<n>=0,1,2...9)\n");
    printf(" -v         verbose\n");
    printf(" -r         readonly\n");
    printf(" -m         mmap mode\n");
    printf(" reg can be a 4-digit hex number, or one of:\n");
    for (i = 0; i<sizeof(regs)/sizeof(regs[0]); i++) {
      printf(" %s", regs[i].name);
    }
    printf("\n");
    printf(" value is a 8-digit hex number\n");
    printf(" if no value is given, it shows the current value\n");
    return 1;
  }

  if (argc < 3 || readonly) {
    ttrx = open_ttrx(rxid, TTRX_RDONLY | (mmapmode ? TTRX_MMAPMODE : 0));
  } else {
    ttrx = open_ttrx(rxid, TTRX_RDWR   | (mmapmode ? TTRX_MMAPMODE : 0));
  }
  if (! ttrx) {
    perror("open");
    printf("TT-RX is not found.\n");
    exit(1);
  }

  while (argc > 1) {
    char *reg = argv[1];
    if (isdigit(reg[0])) {
      offset = strtoul(reg, 0, 16) >> 2;
    } else {
      for (i = 0; i<sizeof(regs)/sizeof(regs[0]); i++) {
	if (strcmp(regs[i].name, reg) == 0) {
	  offset = regs[i].offset;
	  break;
	}
      }
      if (i == sizeof(regs)/sizeof(regs[0])) {
	offset = 0;
	printf("regrx: unknown register %s\n", reg);
      }
    }
    if (offset < 0 || offset >= 0x1000) {
      printf("regrx%d: %s bad address\n", rxid, reg);
    } else if (argc < 3 || readonly) {
      value = read_ttrx(ttrx, offset);
      printf("regrx%d: %s %08x\n", rxid, reg, value);
    } else {
      value = strtoul(argv[2], 0, 16);
      if (write_ttrx(ttrx, offset, value) < 0) {
	perror("write");
	return 1;
      }
      argc--, argv++;
    }
    argc--, argv++;
  }
  return 0;
}
