/* ---------------------------------------------------------------------- *\

   bootft.c - boot FTSW bitmap file

   Mikihiko Nakao, KEK IPNS

   2010091800  copied from bootio.c
   2011042200  -s/-p option
   2013101800  cleanup, ft2u initial setup
   2014071700  -n (noinit) option
   2014081700  ft2p
   2014100700  clear ce_b after program
   2015030800  trgstop after boot
   2015052200  ft2/ft3 filename check
   2015073000  fto
   2016041300  ftproglib
   
\* ---------------------------------------------------------------------- */

#define VERSION 2016041300
const char *PROGRAM = "bootft";

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "ftsw.h"

#define M012_SERIAL      7
#define M012_SELECTMAP   6

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
static void
write_fpga(ftsw_t *ftsw, int m012, int ch, int n)
{
  int i, mask, data;

  if (m012 == M012_SERIAL) {
    for (mask = 0x80; mask > 0; mask >>= 1) {
      data = (ch & mask) ? -1 : 0; // bit-0 = DIN
      write_ftsw(ftsw, FTSWREG_CCLK, data);
    }
  } else if (m012 == M012_SELECTMAP) {
    for (i=0; n == 0 && i<1000; i++) {
      mask = read_ftsw(ftsw, FTSWREG_CONF);
      /* data = read_ftsw(ftsw, FTSWREG_CCLK); */
      /* if (data == 1 && (mask & 0x0f) == 0x0e) break; */
      if ((mask & 0x4f) == 0x4e) break;
      usleep(1);
    }
    if (i == 1000) {
      printf("time out at byte %d mask=%x\n", n, mask);
      exit(1);
    }
    if (i > 0) printf("wait %d at byte %d\n", i, n);
    data = 0;
    for (i = 0, mask = 0x80; mask > 0; mask >>= 1, i++) {
      data |= (ch & mask) ? (1<<i) : 0;
    }
    write_ftsw(ftsw, FTSWREG_CCLK, data);
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
boot_fpga(ftsw_t *ftsw, int id, char *file, int verbose, int forced, int m012)
{
  int i, ch, conf;
  int count = 1, length = 0;
  FILE *fp;
  int nbyte = 0;
  int ftswid = 0;
  char *p;

  /* -- check filename -- */
  p = strrchr(file, '/');
  p = p ? p + 1 : file;
  if (strncmp(p, "ft2", 3) == 0 && id >= 100 && ! forced) {
    printf("cannot program %s into FTSW3\n", p);
    return -1;
  } else if (strncmp(p, "ft3", 3) == 0 && id < 100 && ! forced) {
    printf("cannot program %s into FTSW2\n", p);
    return -1;
  }
  
  /* -- open the file -- */
  if (! (fp = fopen(file, "r"))) {
    printf("cannot open file: %s\n", file);
    return -1;
  }
  
  /* -- check FTSW -- */
  ftswid = read_ftsw(ftsw, FTSWREG_FTSWID);
  if (ftswid != 0x46545357) {
    printf("error: FTSW #%03d is not found\n", id);
    return -1;
  }
  
  /* -- initial condition -- */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose) dump_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  write_ftsw(ftsw, FTSWREG_CONF, 0x08 | m012);
  conf = read_ftsw(ftsw, FTSWREG_CONF) & 7;
  if (verbose || conf != m012) dump_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  write_ftsw(ftsw, FTSWREG_CONF, 0x41);
  write_ftsw(ftsw, FTSWREG_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose || (conf & 0x80)) dump_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  write_ftsw(ftsw, FTSWREG_CONF, 0x86);
  if (verbose) dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose) dump_fpga(conf, "(PRGM=0)");
  
  /* -- skip first 16 bytes -- */
  for (i = 0; i<16; i++) ch = getc(fp);
    
  /* -- get and print header -- */
  if (verbose) printf("== file %s ==\n", file);
  while ((ch = getc(fp)) != 0xff && ch != EOF) {
    if (verbose) putchar(isprint(ch) ? ch : ' ');
  }
  if (verbose) putchar('\n');
  if (ch == EOF) {
    printf("immature EOF for %s\n", file);
    return -1;
  }

  /* -- main part -- */
  do {
    write_fpga(ftsw, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 100000) == 0) {
      printf("%d bytes written (%d)\n", count, time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_fpga(ftsw, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) write_fpga(ftsw, m012, 0xff, nbyte++);
  fclose(fp);

  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose) dump_fpga(conf, "before");
  write_ftsw(ftsw, FTSWREG_CONF, 0x40); /* clear ce_b */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose) dump_fpga(conf, "clr ce_b");
  write_ftsw(ftsw, FTSWREG_CONF, 0x0f); /* clear m012 = 6 */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose) dump_fpga(conf, "clr m012");
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose & ! (conf & 0x80)) dump_fpga(conf, "");
  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
}
