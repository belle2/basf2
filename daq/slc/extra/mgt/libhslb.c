/*
  libhslb.c

  HSLB and FEE register read/write functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "cprfin_fngeneric.h"
#include "hsreg.h"
#include "hslb_info.h"

static char DEVICE[256];

#define M012_SERIAL      7
#define M012_SELECTMAP   6

/* ---------------------------------------------------------------------- *\
   openfn
\* ---------------------------------------------------------------------- */
int
openfn(int fin, int readonly)
{
  int fd;
  int o_mode = readonly ? O_RDONLY : O_RDWR;
  sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + fin);
  
  if ((fd = open(DEVICE, o_mode)) < 0) {
    fprintf(stderr, "cannot open %s: %s\n",
            DEVICE, strerror(errno));
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
  if (ioctl(fd, FNGENERICIO_GET(adr), &val) < 0) {
    fprintf(stderr, "cannot read %s: %s\n",
	    DEVICE, strerror(errno));
    return -1;
  }
  return val;
}
/* ---------------------------------------------------------------------- *\
   writefn
\* ---------------------------------------------------------------------- */
int
writefn(int fd, int adr, int val)
{
  if (ioctl(fd, FNGENERICIO_SET(adr), val) < 0) {
    fprintf(stderr, "cannot write %s: %s\n",
	    DEVICE, strerror(errno));
      return -1;
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   readfn32
\* ---------------------------------------------------------------------- */
int
readfn32(int fd, int adr)
{
  int val = 0;
  writefn(fd, 0x6f, adr & 0x7f);
  val |= (readfn(fd, 0x6e) << 0);
  val |= (readfn(fd, 0x6e) << 8);
  val |= (readfn(fd, 0x6e) << 16);
  val |= (readfn(fd, 0x6e) << 24);
  return val;
}
/* ---------------------------------------------------------------------- *\
   writefn32
\* ---------------------------------------------------------------------- */
int
writefn32(int fd, int adr, int val)
{
  int ret = 0;
  ret |= writefn(fd, 0x6e, (val >> 24) & 0xff);
  ret |= writefn(fd, 0x6e, (val >> 16) & 0xff);
  ret |= writefn(fd, 0x6e, (val >>  8) & 0xff);
  ret |= writefn(fd, 0x6e, (val >>  0) & 0xff);
  ret |= writefn(fd, 0x6f, adr & 0xff);
  return ret;
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
hswait(int fd)
{
  int ret = hswait_quiet(fd);
  //if (ret < 0) printf("no response from FEE at HSLB:%c\n", 'a'+fin);
  return ret;
}
/* ---------------------------------------------------------------------- *\
   readfee8
\* ---------------------------------------------------------------------- */
int
readfee8(int fd, int adr)
{
  writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
  writefn(fd, adr, 0);          /* dummy value write to pass address */
  writefn(fd, HSREG_CSR, 0x07); /* parameter read */
  if (hswait_quiet(fd) < 0) return -1;
  return readfn(fd, adr);
}
/* ---------------------------------------------------------------------- *\
   writefee8
\* ---------------------------------------------------------------------- */
int
writefee8(int fd, int adr, int val)
{
  int ret = 0;
  ret |= writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
  ret |= writefn(fd, adr, val & 0xff);
  ret |= writefn(fd, HSREG_CSR, 0x0a); /* parameter write */
  return ret;
}
/* ---------------------------------------------------------------------- *\
   readfee32
\* ---------------------------------------------------------------------- */
int
readfee32(int fd, int adr, int *valp)
{
  int ret = 0;
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
  
  return 0;
}
/* ---------------------------------------------------------------------- *\
   writefn32
\* ---------------------------------------------------------------------- */
int
writefee32(int fd, int adr, int val)
{
  int ret = 0;
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
/* ---------------------------------------------------------------------- *\
   writestream
\* ---------------------------------------------------------------------- */
int
writestream(int fd, char *filename)
{
  int ret = 0;
  int c = 0;
  FILE *fp = fopen(filename, "r");
  int count = 0;

  if (! fp) {
    fprintf(stderr, "cannot open %s: %s\n",
	    filename, strerror(errno));
    return -1;
  }
  
  ret |= writefn(fd, HSREG_CSR,    0x05); /* reset read fifo */
  ret |= writefn(fd, HSREG_CSR,    0x09); /* stream write begin */
  while ((c = getc(fp)) != EOF) {
    if (ret) return ret;
    ret |= writefn(fd, 0x00, c & 0xff);
    count++;
  }  
  ret |= writefn(fd, HSREG_CSR,    0x08); /* stream write end */

  fclose(fp);
  printf("%d bytes written.\n", count);
  
  return ret;
}

void
writefpga(int fd, int m012, int ch, int n)
{
  int i, mask, data;

  if (m012 == M012_SERIAL) {
    for (mask = 0x80; mask > 0; mask >>= 1) {
      data = (ch & mask) ? 1 : 0; // bit-0 = DIN
      writefn(fd, HSREG_CCLK, data);
    }
  } else if (m012 == M012_SELECTMAP) {
    for (i=0; n == 0 && i<1000; i++) {
      data = readfn(fd, HSREG_CCLK);
      mask = readfn(fd, HSREG_CONF);
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
    writefn(fd, HSREG_CCLK, data);
  }
}

void
dumpfpga(int conf, char *str)
{
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}

int
bootfpga(int fd, char *file, int verbose, int forced, int m012)
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
  conf = readfn(fd, HSREG_CONF);
  if (verbose) dumpfpga(conf, 0);

  /* -- download mode (set M012) -- */
  writefn(fd, HSREG_CONF, 0x08 | m012);
  conf = readfn(fd, HSREG_CONF) & 7;
  if (verbose || conf != m012) dumpfpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  writefn(fd, HSREG_CONF, 0x41);
  writefn(fd, HSREG_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = readfn(fd, HSREG_CONF);
  if (verbose || (conf & 0x80)) dumpfpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  writefn(fd, HSREG_CONF, 0x86);
  dumpfpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = readfn(fd, HSREG_CONF);
  dumpfpga(conf, "(PRGM=0)");
  
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
    writefpga(fd, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 10000) == 0) {
      printf("%d bytes written (%ld)\n", count, time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    writefpga(fd, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) writefpga(fd, m012, 0xff, nbyte++);
  fclose(fp);

  writefn(fd, HSREG_CONF, 0x40); /* clear ce_b */
  writefn(fd, HSREG_CONF, 0x0f); /* clear m012 = 6 */
  conf = readfn(fd, HSREG_CONF);
  if (verbose & ! (conf & 0x80)) dumpfpga(conf, "");
  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
}

int writefee(int fd, int addr, int val)
{
  writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
  writefn(fd, HSREG_CSR,     0x06); /* reset read ack */
  writefn(fd, addr, val);    //HSREG_FEECONT
  writefn(fd, HSREG_CSR,     0x0a); /* parameter write */
  return 1;
}

int readfee(int fd, int addr)
{
  writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
  writefn(fd, HSREG_CSR,     0x06); /* reset read ack */
  writefn(fd, addr, 0x02);    //HSREG_FEECONT
  writefn(fd, HSREG_CSR,     0x07); /* parameter write */
  if (hswait(fd) < 0) return -1;
  return readfn(fd, addr);
}

int linkfee(int fd)
{
  return writefee(fd, HSREG_FEECONT, 0x01);
}

int unlinkfee(int fd)
{
  return writefee(fd, HSREG_FEECONT, 0x02);
}

int trgofffee(int fd)
{
  return writefee(fd, HSREG_FEECONT, 0x03);
}

int trgonfee(int fd)
{
  return writefee(fd, HSREG_FEECONT, 0x04);
}

int writefee16a(int fd, int addr, int nvals, 
		const int* wval, int* rval)
{
  writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
  int i = 0;
  for (; i < nvals; i++) {
    writefn(fd, addr + i * 2, wval[i] & 0xff);
    writefn(fd, addr + i * 2 + 1, (wval[i]>>8) & 0xff);
  }
  if (rval != NULL) {
    writefn(fd, HSREG_CSR,   0x07); /* parameter read */
    if (hswait(fd) < 0) return 0;
    i = 0;
    for (; i < nvals; i++) {
      rval[i] = readfn(fd, addr + i * 2) & 0xff;
      rval[i] |= ((readfn(fd, addr + i * 2 + 1) & 0xff) << 8);
    }
  } else {
    writefn(fd, HSREG_CSR, 0x0a); /* parameter write */
  }
  return 1;
}

int writefee16(int fd, int addr, int wval, int* rval)
{
  return writefee16a(fd, addr, 1, &wval, rval);
}

int writefee8a(int fd, int addr, int nvals, 
	       const int* wval, int* rval)
{
  writefn(fd, HSREG_CSR, 0x05); /* reset read fifo */
  int i = 0;
  for (; i < nvals; i++) {
    writefn(fd, addr + i, wval[i] & 0xff);
  }
  if (rval != NULL) {
    writefn(fd, HSREG_CSR,   0x07); /* parameter read */
    if (hswait(fd) < 0) return 0;
    i = 0;
    for (; i < nvals; i++) {
      rval[i] = readfn(fd, addr + i) & 0xff;
    }
  } else {
    writefn(fd, HSREG_CSR, 0x0a); /* parameter write */
  }
  return 1;
}

int checkfee(struct hslb_info* hslb)
{
  int fd;
  int ret;
  fd = hslb->fd;
 
  writefn(fd, HSREG_CSR, 0x05); /* reset address fifo */
  writefn(fd, HSREG_CSR, 0x06); /* reset status register */
  if ((ret = readfn(fd, HSREG_STAT))) {
    printf("checkfee: cannot clear HSREG_STAT=%02x\n", ret);
    return 0;
  }
  writefn(fd, HSREG_FEEHWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEESERIAL, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_HWVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_FWVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_CPLDVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_CSR, 0x07);
  
  if (hswait(fd) < 0) {
    return 0;
  }
  
  hslb->feehw = readfn(hslb->fd, HSREG_FEEHWTYPE);
  hslb->feeserial = readfn(hslb->fd, HSREG_FEESERIAL);
  hslb->feetype = readfn(hslb->fd, HSREG_FEEFWTYPE);
  hslb->feever  = readfn(hslb->fd, HSREG_FEEFWVER);
  hslb->hslbhw  = readfn(hslb->fd, HSREG_HWVER) & 0xf;
  hslb->hslbfw  = readfn(hslb->fd, HSREG_FWVER) & 0xf;
  hslb->cpldver  = readfn(hslb->fd, HSREG_FWVER) & 0xf;
  
  hslb->feeserial |= (hslb->feehw & 0xf) << 8;
  hslb->feever  |= (hslb->feetype & 0xf) << 8;
  hslb->feehw = (hslb->feehw >> 4) & 0xf;
  hslb->feetype = (hslb->feetype >> 4) & 0xf;

  hslb->hslbid = readfn32(hslb->fd, HSREGL_ID);
  hslb->hslbver = readfn32(hslb->fd, HSREGL_VER);

  return 1;
}

