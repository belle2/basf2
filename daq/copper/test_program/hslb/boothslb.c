/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* ---------------------------------------------------------------------- *\

   bootmgt.c - boot MGT FINESSE prototype bitmap file

   2009042100  retry option
   2009101600  parallel version based on confmgt6.c
   2009112500  selectMAP fix (had to change M012 to 7 at the end)
   2009112501  merge serial and parallel modes
   
\* ---------------------------------------------------------------------- */

#define VERSION 2009112501

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>

#include "find_pci_path.h"
#include "cprfin_fngeneric.h"
#include "find_pci_path.c"
const char *PROGRAM = "bootmgt";

typedef int mgt_t;
mgt_t mgt = -1;
volatile unsigned long *xmemp = 0;
int xslot = 0;

#define MGT_RDONLY  0
#define MGT_RDWR    1
#define MGT_MMAP    2

#define MGT_CCLK  0x74
#define MGT_CONF  0x75
#define MGT_CVER  0x76
#define MGT_TYP1  0x7d
#define MGT_TYP2  0x7e

#define M012_SERIAL      7
#define M012_SELECTMAP   6

/* ---------------------------------------------------------------------- *\
   open_mgt
\* ---------------------------------------------------------------------- */
static int
open_mgt(int ch, int mode)
{
  char *DEVICE = 0;
  int o_mode = (mode & 1) ? O_RDWR                 : O_RDONLY;
  int m_mode = (mode & 1) ? (PROT_READ|PROT_WRITE) : PROT_READ;

  printf("mode = %x\n", mode);
  
  if (mode & MGT_MMAP) {
    unsigned long base = 0;
    int fd;
    struct pcichip {
      unsigned long word[4];
      unsigned long bar[4];
    } chip;


    char *plx9054_path = get_copper_plx9054_path();
    
    if ((fd = open(plx9054_path, O_RDONLY)) < 0) {
      fprintf(stderr, "%s: cannot open %s: %s\n",
	      PROGRAM, plx9054_path, strerror(errno));
      return -1;
    }
    if (read(fd, &chip, sizeof(chip)) != sizeof(chip)) {
      fprintf(stderr, "%s: cannot read %s: %s\n",
	      PROGRAM, plx9054_path, strerror(errno));
      close(fd);
      return -1;
    }
    close(fd);
    base = chip.bar[2];
    
    if (ch >= 'a' && ch <= 'd') ch -= 'a';
    if (ch < 0 || ch > 4) {
      int mgt = open("/dev/mem", o_mode);
      if (mgt == -1) {
	fprintf(stderr, "%s: cannot open /dev/mem: %s\n",
		PROGRAM, strerror(errno));
	return -1;
      }

      xslot = ch;
      xmemp = mmap(0, 0x1000000, m_mode, MAP_SHARED, mgt, base);
      if (xmemp == (unsigned long *)0xffffffff) {
	fprintf(stderr, "%s: cannot mmap /dev/mem: %s\n",
		PROGRAM, strerror(errno));
	return -1;
      }
      
      return mgt;
    }
  } else {
    switch (ch) {
    case  0:
    case 'a': DEVICE = "/dev/copper/fngeneric:a"; break;
    case  1:
    case 'b': DEVICE = "/dev/copper/fngeneric:b"; break;
    case  2:
    case 'c': DEVICE = "/dev/copper/fngeneric:c"; break;
    case  3:
    case 'd': DEVICE = "/dev/copper/fngeneric:d"; break;
    default: errno = ENODEV; return -1;
    }
    int mgt = open(DEVICE, o_mode);
    if (mgt < 0) {
      fprintf(stderr, "%s: cannot open %s: %s\n",
	      PROGRAM, DEVICE, strerror(errno));
      exit(1);
    }
    return mgt;
  }
}
/* ---------------------------------------------------------------------- *\
   close_mgt
\* ---------------------------------------------------------------------- */
static int
close_mgt(int mgt)
{
  close(mgt);
}
/* ---------------------------------------------------------------------- *\
   read_mgt
\* ---------------------------------------------------------------------- */
static int
read_mgt(int mgt, int reg)
{
  if (xmemp) {
    unsigned long csr[4] = { 0x00100000, 0x00100200,
			     0x00100400, 0x00100600 };
    volatile unsigned long *regp = xmemp + csr[xslot] + reg*4;

    static int first = 1;
    if (first) {
      first = 0;
      printf("read_mgt %02x %p\n", reg, regp);
    }
    
    return (*regp >> 24) & 0xff;
  } else {
    int val;
    int ret = ioctl(mgt, FNGENERICIO_GET(reg), &val);
    if (ret < 0) return ret;
    return val;
  }
}
/* ---------------------------------------------------------------------- *\
   write_mgt
\* ---------------------------------------------------------------------- */
static int
write_mgt(int mgt, int reg, int val)
{
  if (xmemp) {
    unsigned long csr[4] = { 0x00100000, 0x00100200,
			     0x00100400, 0x00100600 };
    volatile unsigned long *regp = xmemp + csr[xslot] + reg*4;
    
    *regp = (val & 0xff) << 24;
    static int first = 1;
    if (first) {
      first = 0;
      printf("write_mgt %02x %p %d\n", reg, regp, val);
    }
    return 0;
  } else {
    return ioctl(mgt, FNGENERICIO_SET(reg), val);
  }
}
/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
static void
write_fpga(mgt_t mgt, int m012, int ch, int n)
{
  int i, mask, data;

  if (m012 == M012_SERIAL) {
    for (mask = 0x80; mask > 0; mask >>= 1) {
      data = (ch & mask) ? 1 : 0; // bit-0 = DIN
      write_mgt(mgt, MGT_CCLK, data);
    }
  } else if (m012 == M012_SELECTMAP) {
    for (i=0; n == 0 && i<1000; i++) {
      data = read_mgt(mgt, MGT_CCLK);
      mask = read_mgt(mgt, MGT_CONF);
      if (data == 0 && (mask & 0x0f) == 0x0e) break;
      usleep(1);
    }
    if (i == 1000) {
      printf("time out at byte %d\n", n);
      exit(1);
    }
    if (i > 0) printf("wait %d at byte %d\n", i, n);
    data = 0;
    for (i = 0, mask = 0x80; mask > 0; mask >>= 1, i++) {
      data |= (ch & mask) ? (1<<i) : 0;
    }
    write_mgt(mgt, MGT_CCLK, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_fpga
\* ---------------------------------------------------------------------- */
static void
dump_fpga(int conf, char *str)
{
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
static int
boot_fpga(mgt_t mgt, char *file, int verbose, int forced, int m012)
{
  int i, ch, conf;
  int count = 1, length = 0;
  FILE *fp;
  int nbyte = 0;

  /* -- open the file -- */
  if (! (fp = fopen(file, "r"))) {
    printf("cannot open file: %s\n", file);
    return -1;
  }
  
  /* -- initial condition -- */
  conf = read_mgt(mgt, MGT_CONF);
  if (verbose) dump_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  write_mgt(mgt, MGT_CONF, 0x08 | m012);
  conf = read_mgt(mgt, MGT_CONF) & 7;
  if (verbose || conf != m012) dump_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d).%d\n", conf,m012);
    if (! forced) {
      fclose(fp);
      return -1;
    }
  }
  
  /* -- programming mode (CONF<=1) -- */
  write_mgt(mgt, MGT_CONF, 0x41);
  write_mgt(mgt, MGT_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = read_mgt(mgt, MGT_CONF);
  if (verbose || (conf & 0x80)) dump_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced){
      fclose(fp);
      return -1;
    }
  }

  write_mgt(mgt, MGT_CONF, 0x86);
  dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  /* -- skip first 16 bytes -- */
  for (i = 0; i<16; i++) ch = getc(fp);
    
  /* -- get and print header -- */
  if (verbose) printf("== file %s ==\n", file);
  while ((ch = getc(fp)) != 0xff && ch != EOF) {
    if (verbose) putchar(isprint(ch) ? ch : '.');
  }
  if (verbose) putchar('\n');
  if (ch == EOF) {
    printf("immature EOF for %s\n", file);
      fclose(fp);    return -1;
  }

  /* -- main part -- */
  do {
    write_fpga(mgt, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 10000) == 0) {
      printf("%d bytes written (%d)\n", count, time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_fpga(mgt, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) write_fpga(mgt, m012, 0xff, nbyte++);
  fclose(fp);
  if (verbose ) dump_fpga(conf, "");
  write_mgt(mgt, MGT_CONF, 0x40); /* clear ce_b */
  conf = read_mgt(mgt, MGT_CONF);
  if (verbose) dump_fpga(conf, "");
  write_mgt(mgt, MGT_CONF, 0x0f); /* clear m012 = 6 */
  conf = read_mgt(mgt, MGT_CONF);
  if (verbose) dump_fpga(conf, "");
  conf = read_mgt(mgt, MGT_CONF);
  if (verbose & ! (conf & 0x80)) dump_fpga(conf, "");
  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
  fclose(fp);
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int verbose  = 0;
  int forced   = 0;
  int ret;
  int mode = MGT_RDWR;
  int retry = 0;
  int i;
  int ch = -1; /* default: FINESSE A */
  int m012 = M012_SELECTMAP;
  mgt_t mgt;

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
    } else if (argv[1][0]=='-' && argv[1][1]>='a' && argv[1][1]<='d') {
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

  for (i=0; i<4; i++) {
    if ((ch & (1 << i)) == 0) {
      continue;
    }
    
    do {
      if ((mgt = open_mgt('a'+i, mode)) < 0) {
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
      ret = boot_fpga(mgt, argv[1], verbose, forced, m012);
      if (ret != 0) {
	if (retry-- > 0) {
	  printf("retrying to boot (%d)\n", retry);
	  sleep(1);
	} else {
	  break;
	}
      }
    } while (ret != 0);

    close_mgt(mgt);
  }

  return -ret;
}
