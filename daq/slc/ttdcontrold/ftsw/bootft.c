/* ---------------------------------------------------------------------- *\

   bootft.c - boot FTSW bitmap file

   Mikihiko Nakao, KEK IPNS

   2010091800  copied from bootio.c
   2011042200  -s/-p option
   
\* ---------------------------------------------------------------------- */

#define VERSION 2011042200

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
      write_ftsw(ftsw, FTSWREG_CCLK, data);
/*printf("%d", data);*/
    }
/*if ((xx % 8) == 0) printf("\n");*/
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
boot_fpga(ftsw_t *ftsw, char *file, int verbose, int forced, int m012)
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
  dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
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
  dump_fpga(conf, "before");
  /* write_ftsw(ftsw, FTSWREG_CONF, 0x40); /* clear ce_b */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_fpga(conf, "clr ce_b");
  write_ftsw(ftsw, FTSWREG_CONF, 0x0f); /* clear m012 = 6 */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_fpga(conf, "clr m012");
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
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
#ifdef VXWORKS
int
bootft(char *file, int id)
{
  int verbose  = 1;
  int forced   = 0;
  int m012 = M012_SERIAL;

  ftsw_t ftsw = open_ftsw(id, 0);

  boot_fpga((int *)ftsw, file, verbose, forced, m012);
}
#else
int
main(int argc, char **argv)
{
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
    int fpgaid;
    int fpgaver;
    
    if (! (ftsw = open_ftsw(id[i], oflag))) {
      perror("open");
      exit(1);
    }
    boot_fpga(ftsw, argv[1], verbose, forced, m012);

    /* utime setup */
    fpgaid  = read_ftsw(ftsw, FTSWREG_FPGAID);
    fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0xffff;
    if ((fpgaid == 0x46543255 && fpgaver >= 17) ||  /* FT2U */
	(fpgaid == 0x46543355 && fpgaver >= 17)) {  /* FT3U */
      struct timeval tv;
      int n;
      int sec;
      
      gettimeofday(&tv, 0);
      sec = tv.tv_sec;
      
      for (n=0; ; n++) {
	gettimeofday(&tv, 0);
	if (sec != tv.tv_sec) {
	  write_ftsw(ftsw, FTSWREG_SETUTIM, tv.tv_sec);
	  printf("utime is set, n=%d sec=%ld usec=%ld\n",
		 n, tv.tv_sec, tv.tv_usec);
	  break;
	}
      }
    }
  }
}
#endif
