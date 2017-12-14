/* ---------------------------------------------------------------------- *\

   booths.c - boot HSLB FPGA bitmap file

   Mikihiko Nakao, KEK IPNS

   2014042600  0.01 initial version (copy from bootmgt.c 2009112501)
   2015033100  0.02 various improvements
   2015040100  0.03 a bit more improvements
   2015041600  0.04 libhslb version
   2015060200  0.05 "failed" message was missing
   2015060400  0.06 retry fix
   
\* ---------------------------------------------------------------------- */

#define VERSION 6
#define MOD_DATE 20150604

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>

#include "find_pci_path.h"
#include "hsreg.h"
#include "libhslb.h"

const char *PROGRAM = "booths";

volatile unsigned long *xmemp = 0;
int xslot = 0;

#define M012_SERIAL      7
#define M012_SELECTMAP   6

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
void
write_fpga(int fndev, int m012, int ch, int n, int verbose)
{
  int i, mask, data;

  if (m012 == M012_SERIAL) {
    for (mask = 0x80; mask > 0; mask >>= 1) {
      data = (ch & mask) ? 1 : 0; // bit-0 = DIN
      writefn(fndev, HSREG_CCLK, data);
    }
  } else if (m012 == M012_SELECTMAP) {
    for (i=0; n == 0 && i<1000; i++) {
      data = readfn(fndev, HSREG_CCLK);
      mask = readfn(fndev, HSREG_CONF);
      if (data == 0 && (mask & 0x0f) == 0x0e) break;
      usleep(1);
    }
    if (i == 1000) {
      printf("time out at byte %d\n", n);
      exit(1);
    }
    if (i > 0 && verbose > 0) printf("wait %d at byte %d\n", i, n);
    data = 0;
    for (i = 0, mask = 0x80; mask > 0; mask >>= 1, i++) {
      data |= (ch & mask) ? (1<<i) : 0;
    }
    writefn(fndev, HSREG_CCLK, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_fpga
\* ---------------------------------------------------------------------- */
void
dump_fpga(int conf, char *str)
{
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
int
boot_fpga(int fndev, char *file, int verbose, int forced, int m012)
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
  conf = readfn(fndev, HSREG_CONF);
  if (verbose > 1) dump_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  writefn(fndev, HSREG_CONF, 0x08 | m012);
  conf = readfn(fndev, HSREG_CONF) & 7;
  if (verbose > 1 || conf != m012) dump_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  writefn(fndev, HSREG_CONF, 0x41);
  writefn(fndev, HSREG_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = readfn(fndev, HSREG_CONF);
  if (verbose > 1 || (conf & 0x80)) dump_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  writefn(fndev, HSREG_CONF, 0x86);
  if (verbose > 0) dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = readfn(fndev, HSREG_CONF);
  if (verbose > 0) dump_fpga(conf, "(PRGM=0)");
  
  /* -- skip first 16 bytes -- */
  for (i = 0; i<16; i++) ch = getc(fp);
    
  /* -- get and print header -- */
  if (verbose > 1) printf("== file %s ==\n", file);
  while ((ch = getc(fp)) != 0xff && ch != EOF) {
    if (verbose > 1) putchar(isprint(ch) ? ch : ' ');
  }
  if (verbose > 1) putchar('\n');
  if (ch == EOF) {
    printf("immature EOF for %s\n", file);
    return -1;
  }

  /* -- main part -- */
  do {
    write_fpga(fndev, m012, ch, nbyte++, verbose);
    count++;
    if (verbose > 1 && (count % 10000) == 0) {
      printf("%d bytes written (%d)\n", count, time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose > 1) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_fpga(fndev, m012, 0xff, nbyte++, verbose);
  }
  
  for (i=0; i<100; i++) write_fpga(fndev, m012, 0xff, nbyte++, verbose);
  fclose(fp);

  writefn(fndev, HSREG_CONF, 0x40); /* clear ce_b */
  writefn(fndev, HSREG_CONF, 0x0f); /* clear m012 = 6 */

  conf = readfn(fndev, HSREG_CONF);
  if (verbose > 0 && ! (conf & 0x80)) dump_fpga(conf, "");
  if (conf & 0x80) {
    return 0;
  } else {
    return conf;
  }
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
  int usemmap = 0;
  int retry = 0;
  int i;
  int ch = 0; /* no default */
  int m012 = M012_SELECTMAP;
  int fndev;

  while (argc > 1 && argv[1][0] == '-') {
    for (i=1; argv[1][i]; i++) {
      switch (argv[1][i]) {
      case 'v': verbose++;   break;
      case 'm': usemmap = 1; break;
      case 's': m012 = M012_SERIAL; break;
      case 'p': m012 = M012_SELECTMAP; break;
      case 'r':
        if (isdigit(argv[1][i+1])) {
          retry = atoi(&argv[1][i+1]);
          i = strlen(argv[1]) - 1; /* to break from for loop */
        } else if ((! argv[1][i+1]) && argc > 2 && isdigit(argv[2][0])) {
          retry = atoi(argv[2]);
          argc--, argv++;
          i = strlen(argv[1]) - 1; /* to break from for loop */
        } else {
          retry = 10;
        }
        break;
      case 'a':
      case 'b':
      case 'c':
      case 'd': ch |= 1 << (argv[1][i] - 'a'); break;
      default:
        argc = 0;
      }
    }
    argc--, argv++;
  }
        
  if (argc < 2 || ch == 0) {
    printf("%s version %d.%02d date %d\n",
	   PROGRAM, VERSION/100, VERSION%100, MOD_DATE);
    printf("usage: %s [-abcdfpsv] <filename>\n", PROGRAM);
    printf("options:\n");
    printf(" -a    program HSLB A (at least one of a-d is needed)\n");
    printf(" -b    program HSLB B\n");
    printf(" -c    program HSLB C\n");
    printf(" -d    program HSLB D\n");
    printf(" -v    verbose mode\n");
    printf(" -r<n> retry <n> times\n");
    printf(" -s    serial mode   (Slave serial,    M[2:0]=7)\n");
    printf(" -p    parallel mode (Slave selectMAP, M[2:0]=6, default)\n");
    printf(" -f    forced (ignore errors)\n");
    return 255;
  }

  for (i=0; i<4; i++) {
    int iretry = 0;
    
    if ((ch & (1 << i)) == 0) {
      continue;
    }
    
    do {
      if ((fndev = openfn(i, O_RDWR, "booths")) < 0) {
	if (retry-- > 0) {
	  printf("retrying to open hslb-%d (%d)\n", 'a', retry);
	  sleep(1);
	} else {
	  perror("open");
	  exit(255);
	}
      }
    } while (fndev < 0);
    
    do {
      if ((ret = boot_fpga(fndev, argv[1], verbose, forced, m012)) == 0) {
        if (iretry) {
          printf("hslb-%c done (retry=%d).\n", 'a'+i, iretry);
        } else {
          printf("hslb-%c done.\n", 'a'+i);
        }
      } else {
	if (iretry < retry) {
	  if (verbose > 0) printf("retrying to boot (%d)\n", iretry);
	  sleep(1);
	} else if (ret == -1) {
          return 1;
        } else if (ret != 0) {
          printf("hslb-%c failed.\n", 'a'+i);
	  break;
	}
      }
    } while (ret != 0);

    close(fndev);
  }

  return -ret;
}
