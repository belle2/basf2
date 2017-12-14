/*
  libhslb.c

  HSLB and FEE register read/write functions
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#include "hslb.h"
#include "hsreg.h"

static char *PROGRAM = 0;

/*
  DEVICE contains the device name, but it breaks down if more than one
  devices are open....

  mixed devtype /dev/copper/hslb: and /dev/copper/fngeneric: is not allowed.
 */
static char DEVICE[256];
typedef enum { FIN_UNDEF, FIN_GENERIC, FIN_HSLB } devtype_t;
static devtype_t devtype = FIN_UNDEF;

static char errmsg[1024];

const char* hslberr() 
{
  return errmsg;
}

/* ---------------------------------------------------------------------- *\
   feename
\* ---------------------------------------------------------------------- */
const char *
feename(int hwtype, int fwtype)
{
  static const char *feetype[] = {
    "UNDEF", "SVD", "CDC", "TOP", "ARI", "ECL", "EECL", "KLM", "EKLM",
    "TRG", "FEE10", "FEE11", "FEE12", "FEE13", "DEMO", "TEST" };
  static const char *demotype[] = {
    "UNDEF", "HSLB-B2L", "SP605-B2L", "ML605-B2L", "AC701-B2L" };
  static const char *trgtype[] = {
    "TRGMERGER", /* = 0 */
    "TRGTSF",    /* = 1 */
    "TRG2D",     /* = 2 */
    "TRG3D",     /* = 3 */
    "TRGNN",     /* = 4 */
    "TRGEVTT",   /* = 5 */
    "TRGGRL",    /* = 6 */
    "TRGGDL",    /* = 7 */
    "TRGETM",    /* = 8 */
    "TRGTOP",    /* = 9 */
    "TRGKLM",    /* = 10 */
  };

  hwtype &= 0x0f;
  fwtype  %= 16;

  if (hwtype == 14 && fwtype > 0 && fwtype <= 4) {
    return demotype[fwtype];
  } else if (hwtype == 9 && fwtype >= 0 && fwtype <= 10) {
    return trgtype[fwtype];
  } else {
    int i;
    static int ringi = 0;
    static int ringval[32];
    static char ringbuf[32][20];
    for (i=0; i<32; i++) {
      if (ringval[i] == 0x100 + (hwtype<<4) + fwtype) break;
    }
    if (i == 32) {
      i = ringi;
      ringval[i] = 0x100 + (hwtype<<4) + fwtype;
      //sprintf(ringbuf[i], "%s firmware %d", feetype[hwtype], fwtype);
      sprintf(ringbuf[i], "%s", feetype[hwtype]);
      ringi = (ringi + 1) % 32;
    }
    return ringbuf[i];
  }
}

/* ---------------------------------------------------------------------- *\
   openfn
\* ---------------------------------------------------------------------- */
int
openfn(int ifn, int readonly, char *program)
{
  int fd;
  int o_mode = readonly ? O_RDONLY : O_RDWR;
  
  if (devtype == FIN_GENERIC) {
    sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + ifn);
  } else {
    sprintf(DEVICE, "/dev/copper/hslb:%c", 'a' + ifn);
  }
  PROGRAM = program;
  
  if ((fd = open(DEVICE, o_mode)) < 0) {
    if (errno == ENOENT && devtype == FIN_UNDEF) {
      sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + ifn);
      fd = open(DEVICE, o_mode);
      if (fd >= 0) {
        devtype = FIN_GENERIC;
      }
    }
    if (fd < 0 && program != 0) {
      sprintf(errmsg, "%s: cannot open %s: %s\n",
              program, DEVICE, strerror(errno));
    }
  } else if (devtype == FIN_UNDEF) {
    devtype = FIN_HSLB;
  }
  
  return fd;
}
/* ---------------------------------------------------------------------- *\
   readfn
\* ---------------------------------------------------------------------- */
int
readfn(int fd, int adr)
{
  int val = 0;
  if (ioctl(fd, HSLBIO_GET(adr), &val) < 0) {
    if (PROGRAM) {
      sprintf(errmsg, "%s: cannot read %s: %s\n",
              PROGRAM, DEVICE, strerror(errno));
      exit(1);
    } else {
      return -1;
    }
  }
  return val;
}
/* ---------------------------------------------------------------------- *\
   writefn
\* ---------------------------------------------------------------------- */
int
writefn(int fd, int adr, int val)
{
  if (ioctl(fd, HSLBIO_SET(adr), val) < 0) {
    if (PROGRAM) {
      sprintf(errmsg, "%s: cannot write %s: %s\n",
              PROGRAM, DEVICE, strerror(errno));
      exit(1);
    } else {
      return -1;
    }
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   readfn32
\* ---------------------------------------------------------------------- */
int
readfn32(int fd, int adr)
{
  if (devtype == FIN_HSLB) {
    return readfn(fd, adr);
  } else {
    int val = 0;
    writefn(fd, 0x6f, adr & 0x7f);
    val |= (readfn(fd, 0x6e) << 0);
    val |= (readfn(fd, 0x6e) << 8);
    val |= (readfn(fd, 0x6e) << 16);
    val |= (readfn(fd, 0x6e) << 24);
    return val;
  }
}
/* ---------------------------------------------------------------------- *\
   writefn32
\* ---------------------------------------------------------------------- */
int
writefn32(int fd, int adr, int val)
{
  if (devtype == FIN_HSLB) {
    return writefn(fd, adr, val);
  } else {
    int ret = 0;
    ret |= writefn(fd, 0x6e, (val >> 24) & 0xff);
    ret |= writefn(fd, 0x6e, (val >> 16) & 0xff);
    ret |= writefn(fd, 0x6e, (val >>  8) & 0xff);
    ret |= writefn(fd, 0x6e, (val >>  0) & 0xff);
    ret |= writefn(fd, 0x6f, adr & 0xff);
    return ret;
  }
}
/* ---------------------------------------------------------------------- *\
   hswait_quiet
\* ---------------------------------------------------------------------- */
int
hswait_quiet(int fd)
{
  int i;
  for (i=0; i<100; i++) { /* up to 100 ms */
    usleep(1000);
    if (readfn(fd, HSREG_STAT) == 0x11) break;
  }
  return (i == 100) ? -1 : 0;
}
/* ---------------------------------------------------------------------- *\
   hswait
\* ---------------------------------------------------------------------- */
int
hswait(int fin, int fd)
{
  int ret = hswait_quiet(fd);
  if (ret < 0) printf("no response from FEE at HSLB:%c\n", 'a'+fin);
  return ret;
}
/* ---------------------------------------------------------------------- *\
   readfee8
\* ---------------------------------------------------------------------- */
int
readfee8(int fd, int adr)
{
  if (devtype == FIN_HSLB) {
    int val = 0;
    int ret;
    if ((ret = ioctl(fd, HSLBFEE8_GET(adr), &val)) < 0) {
      sprintf(errmsg, "cannot read %s: %s\n",
	      DEVICE, strerror(errno));
      return -1;
    }
    return val;
  } else {
    writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
    writefn(fd, adr, 0);          /* dummy value write to pass address */
    writefn(fd, HSREG_CSR, 0x07); /* parameter read */
    if (hswait_quiet(fd) < 0) return -1;
    return readfn(fd, adr);
  }
}
/* ---------------------------------------------------------------------- *\
   writefee8
\* ---------------------------------------------------------------------- */
int
writefee8(int fd, int adr, int val)
{
  if (devtype == FIN_HSLB) {
    if (adr <= 0 || adr > 0x7f) return -1;
    if (ioctl(fd, HSLBFEE8_SET(adr), val) < 0) {
      sprintf(errmsg, "cannot write %s: %s\n",
	      DEVICE, strerror(errno));
      return -1;
    }
    return 0;
    
  } else {
    int ret = 0;
    ret |= writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
    ret |= writefn(fd, adr, val & 0xff);
    ret |= writefn(fd, HSREG_CSR, 0x0a); /* parameter write */
    return ret;
  }
}
/* ---------------------------------------------------------------------- *\
   readfee32
\* ---------------------------------------------------------------------- */
int
readfee32(int fd, int adr, int *valp)
{
  int ret = 0;
  
  if (devtype == FIN_HSLB) {
    int val = adr;
    if ((ret = ioctl(fd, HSLBFEE32_GET(0), &val)) < 0) {
      sprintf(errmsg, "cannot read %s: %s\n",
	      DEVICE, strerror(errno));
      return -1;
    }
    *valp = val;

  } else {
    ret |= writefn(fd, HSREG_CSR,    0x05); /* reset read fifo */
    ret |= writefn(fd, HSREG_CSR,    0x0c); /* 32-bit parameter read */
    ret |= writefn(fd, HSREG_SERIAL, (adr  >>  8) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (adr  >>  0) & 0xff);
    ret |= writefn(fd, HSREG_CSR,    0x08); /* end of stream */
    if (ret != 0) return ret;
    if (hswait_quiet(fd) < 0) return -1;
    
    *valp =
      ((readfn(fd, HSREG_D32D) & 0xff) << 24) |
      ((readfn(fd, HSREG_D32C) & 0xff) << 16) |
      ((readfn(fd, HSREG_D32B) & 0xff) <<  8) |
      ((readfn(fd, HSREG_D32A) & 0xff) <<  0);
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   writefn32
\* ---------------------------------------------------------------------- */
int
writefee32(int fd, int adr, int val)
{
  int ret = 0;

  if (devtype == FIN_HSLB) {
    int buf[2];
    buf[0] = adr;
    buf[1] = val;
    if (ioctl(fd, HSLBFEE32_SET(0), buf) < 0) {
      sprintf(errmsg, "cannot write %s: %s\n",
	      DEVICE, strerror(errno));
      return -1;
    }
    return 0;
  } else {
    ret |= writefn(fd, HSREG_CSR,    0x05); /* reset read fifo */
    ret |= writefn(fd, HSREG_CSR,    0x0b); /* 32-bit parameter write */
    ret |= writefn(fd, HSREG_SERIAL, (adr >>  8) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (adr >>  0) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (val >> 24) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (val >> 16) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (val >>  8) & 0xff);
    ret |= writefn(fd, HSREG_SERIAL, (val >>  0) & 0xff);
    ret |= writefn(fd, HSREG_CSR,    0x08); /* end of stream */
    return ret;
  }
}
/* ---------------------------------------------------------------------- *\
   writestream
\* ---------------------------------------------------------------------- */
int
writestream(int fd, char *filename)
{
  int ret = 0;
  int c = 0;
  FILE *fp = fopen(filename, "r");

  if (! fp) {
    if (PROGRAM) {
      sprintf(errmsg, "%s: cannot open %s: %s\n",
	      PROGRAM, filename, strerror(errno));
    }
    return -1;
  }
  
  ret |= writefn(fd, HSREG_CSR,    0x05); /* reset read fifo */
  ret |= writefn(fd, HSREG_CSR,    0x09); /* stream write begin */
  while ((c = getc(fp)) != EOF) {
    if (ret) return ret;
    ret |= writefn(fd, 0x00, c & 0xff);
  }  
  ret |= writefn(fd, HSREG_CSR,    0x08); /* stream write end */

  fclose(fp);
  
  return ret;
}

#define M012_SERIAL      7
#define M012_SELECTMAP   6

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
void write_fpga(int fndev, int m012, int ch, int n, int verbose)
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
void dump_fpga(int conf, char *str)
{
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
int boot_fpga(int fndev, char *file, int verbose, int forced, int m012)
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
