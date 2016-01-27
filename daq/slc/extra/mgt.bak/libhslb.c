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

static const char *feetype[] = {
  "UNDEF", "SVD", "CDC", "BPID", "EPID", "ECL", "KLM", "TRG",
  "UNKNOWN-8", "UNKNOWN-9", "UNKNOWN-10", "UNKNOWN-11",
  "UNKNOWN-12", "UNKNOWN-13", "DEMO", "TEST" };

/*
static const char *demotype[] = {
  "UNDEF", "HSLB-B2L", "SP605-B2L", "ML605-B2L", "AC701-B2L" };
*/

static char errmsg[1024];

const char* hslberr() 
{
  return errmsg;
}

/* ---------------------------------------------------------------------- *\
   openfn
\* ---------------------------------------------------------------------- */
int
openfn(int fin, int readonly)
{
  int fd;
  int o_mode = readonly ? O_RDONLY : O_RDWR;
  //sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + fin);
  sprintf(DEVICE, "/dev/copper/hslb:%c", 'a' + fin);
  
  if ((fd = open(DEVICE, o_mode)) < 0) {
    sprintf(errmsg, "cannot open %s: %s\n",
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
    sprintf(errmsg, "cannot read %s: %s\n",
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
    sprintf(errmsg, "cannot write %s: %s\n",
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
bootfpga(int fd, const char *file, int verbose, int forced, int m012)
{
  int i, ch, conf;
  int count = 1, length = 0;
  FILE *fp;
  int nbyte = 0;

  /* -- open the file -- */
  if (! (fp = fopen(file, "r"))) {
    sprintf(errmsg, "cannot open file: %s\n", file);
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
    sprintf(errmsg, "cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  writefn(fd, HSREG_CONF, 0x41);
  writefn(fd, HSREG_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = readfn(fd, HSREG_CONF);
  if (verbose || (conf & 0x80)) dumpfpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    sprintf(errmsg, "cannot set FPGA to the programming mode.\n");
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
    sprintf(errmsg, "immature EOF for %s\n", file);
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

int checkfee(struct hslb_info* hslb)
{
  int ret;
  int fd = hslb->fd;
  int fin = hslb->fin;
  
  writefn(fd, HSREG_CSR, 0x05); /* reset address fifo */
  writefn(fd, HSREG_CSR, 0x06); /* reset status register */
  if ((ret = readfn(fd, HSREG_STAT))) {
    sprintf(errmsg, "checkfee: cannot clear HSREG_STAT=%02x\n", ret);
    return 0;
  }
  writefn(fd, HSREG_FEEHWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEESERIAL, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_CSR, 0x07);
  
  if (hswait(fd) < 0) {
    return 0;
  }
  
  hslb->feehw = readfn(fd, HSREG_FEEHWTYPE);
  hslb->feeserial = readfn(fd, HSREG_FEESERIAL);
  hslb->feetype = readfn(fd, HSREG_FEEFWTYPE);
  hslb->feever = readfn(fd, HSREG_FEEFWVER);
  hslb->feeserial |= (hslb->feehw & 0xf) << 8;
  hslb->feetype |= (hslb->feetype & 0xf) << 8;
  hslb->feehw = (hslb->feehw >> 4) & 0xf;
  hslb->feetype = (hslb->feetype >> 4) & 0xf;
  hslb->hslbid = readfn32(hslb->fd, HSREGL_ID);
  hslb->hslbver = readfn32(hslb->fd, HSREGL_VER);
  if (hslb->feehw == 14 && hslb->feetype > 0 && hslb->feetype <= 4) {
    printf("FEE type %s serial %d version %d at HSLB-%c\n",
	   feetype[hslb->feehw], hslb->feeserial, hslb->feever, (char)('a'+fin));
    return 1;
  } else {
    printf("FEE type %s serial %d firmware %d version %d at HSLB-%c\n",
	   feetype[hslb->feehw], hslb->feeserial, hslb->feetype, hslb->feever, (char)('a'+fin));
    return 1;
  }
  return 0;
}

#define EHSLB_NOTFOUND   (-1001)
#define EHSLB_TOOOLD     (-1002)
#define EHSLB7_TOOOLD    (-1003)
#define EHSLB_CLOCKNONE  (-1004)
#define EHSLB_CLOCKLOST  (-1005)
#define EHSLB_CLOCKFAST  (-1006)
#define EHSLB_PLLLOST    (-1007)
#define EHSLB_BADSTATE   (-1008)
#define EHSLB_DISABLED   (-1009)
#define EHSLB_INTERNAL   (-1010)

#define EHSLB_B2LDOWN    (-1011)
#define EHSLB_CLOCKBAD   (-1012)
#define EHSLB_PLL2LOST   (-1013)
#define EHSLB_GTPPLL     (-1014)
#define EHSLB_FFCLOCK    (-1015)

#define WHSLB_PLLLOST    (1)
#define WHSLB_B2LDOWN    (2)
#define WHSLB_CLOCKBAD   (3)
#define WHSLB_PLL2LOST   (4)
#define WHSLB_GTPPLL     (5)
#define WHSLB_FFCLOCK    (6)

int reset_b2l(int fd, int *csrp)
{
  int i = 0;
  int csr = 0;
  int force = 1;
  for (i=0; i<100; i++) {
    int j = 0;
    csr = readfn32(fd, HSREGL_STAT); /* 0x83 */

    if (csr & 0x80) {
      writefn32(fd, HSREGL_RESET, 0x100000);
      usleep(100000);
    } else if (csr & 0x20000000) {
      writefn32(fd, HSREGL_RESET, 0x10000);
      usleep(100000);
    }

    for (j=0; j<100; j++) {
      int csr2 = readfn32(fd, HSREGL_STAT); /* 0x83 */
      if ((csr ^ csr2) & 0x100) break;
    }
    if (j == 100) csr |= 0x20000000;
    
    if ((csr & 0x200000e1) == 0 && force == 0) break;
    force = 0;
    writefn32(fd, HSREGL_RESET, 0x1000);
    usleep(100000);
    writefn32(fd, HSREGL_RESET, 0x10);
    usleep(100000);
  }
  if (csrp) *csrp = csr;
  return i;
}

int check_hslb(struct hslb_info* hslb, char* msg)
{
  int fd = hslb->fd;
  int i = hslb->fin;
  int tmpval = 0;
  int *valp = &tmpval;
  int warning = 0;
  int errcode = 0;

  *valp = 0;
  
  //for (i=0; i<4; i++) {
  //int o_readonly = 1;
  //int fd;
  int id;
  int ver;
  int csr;
  int j;
    
  //if ((use_slot & (1 << i)) == 0) continue;
  
  //fd = openfn(i, o_readonly, "tesths");
  if (fd < 0) {
    errcode = errno;
    return errcode;
  }
    
  id  = readfn32(fd, HSREGL_ID);   /* 0x80 */
  ver = readfn32(fd, HSREGL_VER);  /* 0x81 */
  csr = readfn32(fd, HSREGL_STAT); /* 0x83 */
  
  if (id != 0x48534c42 && id != 0x48534c37) {
    sprintf(msg, "hslb-%c not found (id=0x%08x != 0x48534c42)\n", 'A'+i, id);
    *valp = id;
    errcode = EHSLB_NOTFOUND;
    return errcode;
  }
  if (id == 0x48534c42 && ver < 34){
    sprintf(msg, "hslb-%c too old firmware (ver=0.%02d < 0.34)\n", 'A'+i, ver);
    *valp = ver;
    errcode = EHSLB_TOOOLD;
    return errcode;
  }
  if (id == 0x48534c37 && ver < 6){
    sprintf(msg, "hslb7-%c too old firmware (ver=0.%02d < 0.06)\n", 'A'+i, ver);
    *valp = ver;
    errcode = EHSLB7_TOOOLD;
    return errcode;
  }
  if (csr & 0x20000000) {
    int j;
    int recvok = 0;
    int uptime0 = readfn32(fd, HSREGL_UPTIME);
    int uptime1;
    usleep(1000*1000);
    uptime1 = readfn32(fd, HSREGL_UPTIME);
    
    if (uptime0 == 0) {
      sprintf(msg, "hslb-%c clock is missing\n", 'A'+i);
      errcode = EHSLB_CLOCKNONE;
      return errcode;
    } else if (uptime0 == uptime1) {
      sprintf(msg, "hslb-%c clock is lost or too slow\n", 'A'+i);
      errcode = EHSLB_CLOCKLOST;
      return errcode;
    } else if (uptime1 > uptime0 + 1) {
      sprintf(msg, "hslb-%c clock is too fast\n", 'A'+i);
      errcode = EHSLB_CLOCKFAST;
      return errcode;
    }
    
    for (j = 0; j < 100; j++) {
      int recv = readfn32(fd, HSREGL_RXDATA) & 0xffff;
      if (recv == 0x00bc) recvok++;
    }
    if (recvok < 80) {
        sprintf(msg, "hslb-%c PLL lost and can't receive data (csr=%08x)\n",
	       'A'+i, csr);
        errcode = EHSLB_PLLLOST;
        return errcode;
    }
    sprintf(msg, "hslb-%c PLL lost (csr=%08x) is probably harmless and ignored\n",
	   'A'+i, csr);
    csr &= ~0x20000000;
    warning = WHSLB_PLLLOST;
  }
  for (j=0; j<100; j++) {
    int csr2 = readfn32(fd, HSREGL_STAT); /* 0x83 */
    if ((csr ^ csr2) & 0x100) break;
  }
  if (j==100) csr |= 0x20000000;
  
  /*
    bit 00000001 - link is not established
    bit 00000002 - hslb is disabled
    bit 00000020 - bad 127MHz detected
    bit 00000040 - GTP PLL not locked (never happen?)
    bit 00000080 - PLL2 not locked
    bit 00000100 - LSB of statff, should be toggling
    bit 20000000 - PLL1 not locked, but somehow not correctly working,
    so it is reused for j=100 condition
    bit 80000000 - link down happened in the past
  */
  
  if ((csr & 0x200000e1)) {
    int count;
    int oldcsr = csr;
    
    count = reset_b2l(fd, &csr);
    
    if ((csr & 0x200000e1)) {
      *valp = csr;
      if (csr & 1) {
	sprintf(msg, "hslb-%c Belle2link is down, csr=%08x\n", 'A'+i, csr);
	errcode = EHSLB_B2LDOWN;
	return errcode;
      } else if (csr & 0x20) {
	sprintf(msg, "hslb-%c bad clock detected, csr=%08x\n", 'A'+i, csr);
	errcode = EHSLB_CLOCKBAD;
	return errcode;
      } else if (csr & 0x80) {
	sprintf(msg, "hslb-%c PLL2 lock lost, csr=%08x\n", 'A'+i, csr);
	errcode = EHSLB_PLL2LOST;
	return errcode;
      } else if (csr & 0x40) {
	sprintf(msg, "hslb-%c GTP PLL lock lost, csr=%08x\n", 'A'+i, csr);
	errcode = EHSLB_GTPPLL;
	return errcode;
      } else if (csr & 0x20000000) {
	sprintf(msg, "hslb-%c FF clock is stopped, csr=%08x\n", 'A'+i, csr);
	errcode = EHSLB_FFCLOCK;
	return errcode;
      }
    } else {
      *valp = count;
      if (oldcsr & 1) {
	warning = WHSLB_B2LDOWN;
	sprintf(msg, "hslb-%c Belle2link recovered, csr=%08x (retry %d)\n",
	       'A'+i, csr, count);
      } else if (oldcsr & 0x20) {
	warning = WHSLB_B2LDOWN;
	sprintf(msg, "hslb-%c bad clock recovered, csr=%08x (retry %d)\n",
	       'A'+i, csr, count);
      } else if (oldcsr & 0x80) {
	warning = WHSLB_PLL2LOST;
	sprintf(msg, "hslb-%c PLL2 lock recovered, csr=%08x (retry %d)\n",
	       'A'+i, csr, count);
      } else if (oldcsr & 0x40) {
	warning = WHSLB_GTPPLL;
	sprintf(msg, "hslb-%c GTP PLL lock recovered, csr=%08x (retry %d)\n",
	       'A'+i, csr, count);
      } else if (oldcsr & 0x20000000) {
	warning = WHSLB_FFCLOCK;
	sprintf(msg, "hslb-%c FF clock is recovered, csr=%08x (retry %d)\n",
	       'A'+i, csr, count);
      }
    }
  }
  
  /* 2015.0605.1524 this doesn't seem to be a solution when statepr != 0 */
  /* 2015.0618.1408 although not perfect, still better than nothing */
  if (csr & 0x000f0000) {
    int val;
    readfee32(fd, 0, &val);
    csr = readfn32(fd, HSREGL_STAT); /* 0x83 */
  }
  
  if ((csr & 0x5fffeec1) != 0x18000000 && (csr & 0x200000e1) == 0) {
    sprintf(msg, "hslb-%c hslb in bad state (csr=%08x)\n", 'A'+i, csr);
    *valp = csr;
    errcode = EHSLB_BADSTATE;
    return errcode;
  }
  if (csr & 2) {
    sprintf(msg, "hslb-%c is disabled, ttrx reg 130 bit%d=0\n",
	   'A'+i, i);
    errcode = EHSLB_DISABLED;
    return errcode;
  }
  /*
    if (csr & 0x80000000) {
    sprintf(msg, "hslb-%c belle2link was down and recovered (csr=%08x), %s\n",
    'A'+i, csr, "need runreset");
    }
  */
  sprintf(msg, "hslb-%c 0.%02d %08x\n", 'A'+i, ver, csr);
  
  //  close(fd);
  //}
  return errcode;
}
