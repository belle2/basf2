#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "ftsw.h"

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
void ftsw_write_fpga(ftsw_t *ftsw, int m012, int ch, int n) {
  int i, mask, data;

static int xx=0,yy=0;
if (yy<20) {
  if (ch != 0 && ch != 0xff) {
    /*printf("xx=%d ch=%02x\n", xx, ch);*/
    yy++;
  }
}
xx++;

  if (m012 == M012_SERIAL) {
    for (mask = 0x80; mask > 0; mask >>= 1) {
      data = (ch & mask) ? -1 : 0; // bit-0 = DIN
      ftsw_write(ftsw, FTSW_CCLK, data);
/*printf("%d", data);*/
    }
/*if ((xx % 8) == 0) printf("\n");*/
  } else if (m012 == M012_SELECTMAP) {
    for (i=0; n == 0 && i<1000; i++) {
      mask = ftsw_read(ftsw, FTSW_CONF);
      /* data = ftsw_read(ftsw, FTSW_CCLK); */
      /* if (data == 1 && (mask & 0x0f) == 0x0e) break; */
      if ((mask & 0x4f) == 0x4e) break;
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
    ftsw_write(ftsw, FTSW_CCLK, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_fpga
\* ---------------------------------------------------------------------- */
void dump_fpga(int conf, char *str) {
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
int ftsw_boot_fpga(ftsw_t *ftsw, const char *file, int verbose, int forced, int m012) {
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
  conf = ftsw_read(ftsw, FTSW_CONF);
  if (verbose) dump_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  ftsw_write(ftsw, FTSW_CONF, 0x08 | m012);
  conf = ftsw_read(ftsw, FTSW_CONF) & 7;
  if (verbose || conf != m012) dump_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  ftsw_write(ftsw, FTSW_CONF, 0x41);
  ftsw_write(ftsw, FTSW_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = ftsw_read(ftsw, FTSW_CONF);
  if (verbose || (conf & 0x80)) dump_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  ftsw_write(ftsw, FTSW_CONF, 0x86);
  dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = ftsw_read(ftsw, FTSW_CONF);
  dump_fpga(conf, "(PRGM=0)");
  
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
    ftsw_write_fpga(ftsw, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 100000) == 0) {
      printf("%d bytes written (%d)\n", count, (int)time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    ftsw_write_fpga(ftsw, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) ftsw_write_fpga(ftsw, m012, 0xff, nbyte++);
  fclose(fp);

  conf = ftsw_read(ftsw, FTSW_CONF);
  dump_fpga(conf, "before");
  /* ftsw_write(ftsw, FTSW_CONF, 0x40);  clear ce_b */
  conf = ftsw_read(ftsw, FTSW_CONF);
  dump_fpga(conf, "clr ce_b");
  ftsw_write(ftsw, FTSW_CONF, 0x0f); /* clear m012 = 6 */
  conf = ftsw_read(ftsw, FTSW_CONF);
  dump_fpga(conf, "clr m012");
  conf = ftsw_read(ftsw, FTSW_CONF);
  if (verbose & ! (conf & 0x80)) dump_fpga(conf, "");
  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
}
/* ---------------------------------------------------------------------- *\
   ftsw_trigger_single
\* ---------------------------------------------------------------------- */
int ftsw_trigger_single(ftsw_t *ftsw) {
  int offset = strtoul("450", 0, 16) >> 2;
  int value = 1;
  return ftsw_write(ftsw, offset, value);
}
/* ---------------------------------------------------------------------- *\
   ftsw_stop_trigger
\* ---------------------------------------------------------------------- */
int ftsw_stop_trigger(ftsw_t *ftsw) {
  int offset = strtoul("190", 0, 16) >> 2;
  int value = 10000000;
  return ftsw_write(ftsw, offset, value);
}
