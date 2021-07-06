/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* ---------------------------------------------------------------------- *\

   regmgt.c - MGT FINESSE prototype register access

   2009042100  first version
   2009110400  32-bit registers
   2009120401  multiple finesse, multiple write
   2009121500  register by name
   2009121600  small fix
   
\* ---------------------------------------------------------------------- */

#define VERSION 2009121600

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "cprfin_fngeneric.h"
#include "mgtf.h"

char *ARGV0  = 0;
char *DEVICE = 0;

/* ---------------------------------------------------------------------- *\
   rdmgt
\* ---------------------------------------------------------------------- */
int
rdmgt(int fd, int adr)
{
  int val = 0;
  if (ioctl(fd, FNGENERICIO_GET(adr), &val) < 0) {
    fprintf(stderr, "%s: cannot read %s: %s\n",
	    ARGV0, DEVICE, strerror(errno));
    exit(1);
  }
  return val;
}
/* ---------------------------------------------------------------------- *\
   wrmgt
\* ---------------------------------------------------------------------- */
void
wrmgt(int fd, int adr, int val)
{
  if (ioctl(fd, FNGENERICIO_SET(adr), val) < 0) {
    fprintf(stderr, "%s: cannot write %s: %s\n",
	    ARGV0, DEVICE, strerror(errno));
    exit(1);
  }
}
/* ---------------------------------------------------------------------- *\
   rdmgt32
\* ---------------------------------------------------------------------- */
int
rdmgt32(int fd, int adr)
{
  int val = 0;
  wrmgt(fd, 0x6f, adr & 0x7f);
  val |= (rdmgt(fd, 0x6e) << 0);
  val |= (rdmgt(fd, 0x6e) << 8);
  val |= (rdmgt(fd, 0x6e) << 16);
  val |= (rdmgt(fd, 0x6e) << 24);
  return val;
}
/* ---------------------------------------------------------------------- *\
   wrmgt32
\* ---------------------------------------------------------------------- */
int
wrmgt32(int fd, int adr, int val)
{
  wrmgt(fd, 0x6e, (val >> 24) & 0xff);
  wrmgt(fd, 0x6e, (val >> 16) & 0xff);
  wrmgt(fd, 0x6e, (val >>  8) & 0xff);
  wrmgt(fd, 0x6e, (val >>  0) & 0xff);
  wrmgt(fd, 0x6f, adr & 0xff);
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int fd[4];
  int use[4];
  int i, j;

  static struct { char *name; int adrs; } regs[] = {
    { "gtprst",   MGTF_GTPRST },
    { "gtpstat",  MGTF_GTPSTAT },
    { "window",	  WINDOW},
    { "datalo",   MGTF_DATALO },
    { "datahi",   MGTF_DATAHI },
    { "prbs",     MGTF_PRBS },
    { "ramdat",   MGTF_RAMDAT },
    { "ramcsr",   MGTF_RAMCSR },
    { "ramahi",   MGTF_RAMAHI },
    { "ramalo",   MGTF_RAMALO },
    { "d32a",     MGTF_D32A },
    { "d32b",     MGTF_D32B },
    { "d32c",     MGTF_D32C },
    { "d32d",     MGTF_D32D },
    { "d32",      MGTF_D32 },
    { "a32",      MGTF_A32 },
    { "cclk",     MGTF_CCLK },
    { "conf",     MGTF_CONF },
    { "cpldver",  MGTF_CPLDVER },
    { "finrev",   MGTF_FINREV },
    { "finver",   MGTF_FINVER },
    { "serial1",  MGTF_SERIAL1 },
    { "serial2",  MGTF_SERIAL2 },
    { "fintype1", MGTF_FINTYPE1 },
    { "fintype2", MGTF_FINTYPE2 },
    { "id",       MGTF_ID },
    { "ext",      MGTF_EXT },
    { "pin",      MGTF_PIN },
    { "scal",     MGTF_SCAL },
    { "sca01",    MGTF_SCA01 },
    { "sca23",    MGTF_SCA23 },
    { "rxdata",   MGTF_RXDATA },
  };

  ARGV0 = argv[0];

  use[0] = use[1] = use[2] = use[3] = 0;
  fd[0]  = fd[1]  = fd[2]  = fd[3]  = -1;
  
  while (argc >= 2 && argv[1][0] == '-' && argv[1][1] != 0) {
    for (i = 1; argv[1][i]; i++) {
      DEVICE = 0;
      if (! strchr("abcd", argv[1][i])) {
	argc = -1;
	break;
      }
      
      j = argv[1][i] - 'a';
      if (use[j]) {
	fprintf(stderr, "FINESSE %c is specified twice\n", 'a'+j);
	exit(1);
      }
      use[j] = 1;
    }
    argv++, argc--;
  }

  if (argc < 2 || (use[0]+use[1]+use[2]+use[3]) == 0) {
    fprintf(stderr, "%s version %d\n", ARGV0, VERSION);
    fprintf(stderr, "usage: %s -{a,b,c,d} <adr(hex)> [<val(hex)>]\n", ARGV0);
    exit(1);
  }

  while (argc >= 2) {
    int o_mode = O_RDONLY;
    unsigned int adr = 0;
    
    for (i = 0; i<sizeof(regs)/sizeof(regs[0]); i++) {
      if (strcmp(regs[i].name, argv[1]) == 0) {
	adr = regs[i].adrs;
	break;
      }
    }
    
    if (i == sizeof(regs)/sizeof(regs[0])) {
      adr = strtoul(argv[1], 0, 16);
    }
    
    unsigned int val = 0;
    
    if (argc >= 3 && strcmp(argv[2], "-") != 0) {
      o_mode = O_RDWR;
      val = strtoul(argv[2], 0, 16);
    }
    if (adr > 0xff) {
      fprintf(stderr, "%s: invalid address 0x%x\n", ARGV0, adr);
      exit(1);
    }
    if (adr < 0x80 && val > 0xff) {
      fprintf(stderr, "%s: invalid value 0x%x for address 0x%x\n",
	      ARGV0, val, adr);
      exit(1);
    }
    printf("address is %u\n",adr);
    for (i=0; i<4; i++) {
      if (! use[i]) continue;
      if (fd[i] < 0) {
	char DEVICE[256];
	sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + i);
	if ((fd[i] = open(DEVICE, o_mode)) < 0) {
	  fprintf(stderr, "%s: cannot open %s: %s\n",
		  ARGV0, DEVICE, strerror(errno));
	  exit(1);
	}
      }

      if (o_mode != O_RDWR) {
	if (adr >= 0x80) {
	  val = rdmgt32(fd[i], adr);
	  printf("%c: %02x val %08x\n", 'a' + i, adr, val);
	} else {
	  val = rdmgt(fd[i], adr);
	  printf("%c: %02x val %02x\n", 'a' + i, adr, val);
	}
      } else {
	if (adr >= 0x80) {
	  wrmgt32(fd[i], adr, val);
	} else {
	  wrmgt(fd[i], adr, val);
	}
      }
    }
    argc -= 2;
    argv += 2;
  }

  close(fd);
}
