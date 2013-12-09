/* ---------------------------------------------------------------------- *\

   regft.c - FTSW register access

   Mikihiko Nakao, KEK IPNS

   2010100700  first version
   2011071100  fancy things
   2011072000  sclk
   
\* ---------------------------------------------------------------------- */

#define VERSION 2011072000

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
  int readonly = 0;
  int offset = -1;
  int value;
  char *p = getenv("FTSW_DEFAULT");
  int ftid = p ? atoi(p) : 0;
  ftsw_t *ftsw;
  int i, ret;

  static struct { char *name; int offset; } regs[] = {
    { "id",      FTSWREG_FTSWID },
    { "cpldver", FTSWREG_CPLDVER },
    { "conf",    FTSWREG_CONF },
    { "cclk",    FTSWREG_CCLK },
    { "fpgaid",  FTSWREG_FPGAID },
    { "fpgaver", FTSWREG_FPGAVER },
    { "jset",    FTSWREG_JCTL },
    { "jget",    FTSWREG_JREG },
    { "jpd",     FTSWREG_JRST },
    { "jsta",    FTSWREG_JSTA },
  };

  while (argc > 1 && argv[1][0] == '-') {
    if (argv[1][1] == 'v') {
      verbose = ! verbose;
    } else if (argv[1][1] == 'r') {
      readonly = ! readonly;
    } else if (isdigit(argv[1][1])) {
      ftid = atoi(&argv[1][1]);
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
  }
  if (argc < 2) {
    printf("usage: regft -<n> [-v] <reg] [<value> [<reg> <value>]...]\n");
    printf("       regft -<n> [-v] -r <reg> [<reg> ...]\n");
    printf("options:\n");
    printf(" -<n>    select <n>-th FTSW (<n>=1,2,3,4)\n");
    printf(" -v      verbose\n");
    printf(" -r      readonly\n");
    printf("reg can be a hex number, or one of:\n");
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
    ftsw = open_ftsw(ftid, FTSW_RDONLY);
  } else {
    ftsw = open_ftsw(ftid, FTSW_RDWR);
  }
  if (! ftsw) {
    perror("open");
    printf("FTSW is not found.\n");
    exit(1);
  }

  while (argc > 1) {
    char *reg = argv[1];
    if (isdigit(reg[0])) {
      offset = strtoul(reg, 0, 16) >> 2;
    } else if (strcmp(reg, "xclk") == 0) {
      if (write_ftsw(ftsw, FTSWREG_CPLDVER, 0x58434c4b) < 0) {
	perror("write.xclk");
	return 1;
      }
      argc--, argv++;
      continue;
    } else if (strcmp(reg, "stop") == 0) {
      if (write_ftsw(ftsw, FTSWREG_CPLDVER, 0x53544f50) < 0) {
	perror("write.stop");
	return 1;
      }
      argc--, argv++;
      continue;
    } else if (strcmp(reg, "sclk") == 0 && argc > 2) {
      value = strtoul(argv[2], 0, 16);
      if (write_ftsw(ftsw, FTSWREG_CPLDVER, 0x434c4b00 | value) < 0) {
	perror("write.sclk");
	return 1;
      }
      argc--, argv++;
      argc--, argv++;
      continue;
    } else if (strcmp(reg, "version") == 0) {
      int s3id  = read_ftsw(ftsw, FTSWREG_FTSWID);
      int s3ver = read_ftsw(ftsw, FTSWREG_CPLDVER);
      int v5id  = read_ftsw(ftsw, FTSWREG_FPGAID);
      int v5ver = read_ftsw(ftsw, FTSWREG_FPGAVER);
      char *ok = (s3id == 0x46545357 && v5id == 0x046545357) ? "ok" : "ng";
      printf("id %08x/%08x(%s) version s%03d/v%03d\n",
             s3id, v5id, ok, s3ver, v5ver);
      argc--, argv++;
      continue;
    } else {
      for (i = 0; i<sizeof(regs)/sizeof(regs[0]); i++) {
	if (strcmp(regs[i].name, reg) == 0) {
	  offset = regs[i].offset;
	  break;
	}
      }
      if (i == sizeof(regs)/sizeof(regs[0])) {
	offset = 0;
	printf("regft: unknown register %s\n", reg);
      }
    }

    if (offset < 0 || offset >= 0x10000) {
      printf("regft%d: %s bad address\n", ftid, reg);
    } else if (argc < 3 || readonly) {
      value = read_ftsw(ftsw, offset);
      printf("regft%d: %s %08x\n", ftid, reg, value);
    } else {
      value = strtoul(argv[2], 0, 16);
      if (write_ftsw(ftsw, offset, value) < 0) {
	perror("write");
	return 1;
      }
      argc--, argv++;
    }
    argc--, argv++;
  }
  return 0; 
}
