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
      sprintf(ringbuf[i], "%s firmware %d", feetype[hwtype], fwtype);
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
      if (PROGRAM) {
        sprintf(errmsg, "%s: cannot read %s: %s\n",
                PROGRAM, DEVICE, strerror(errno));
        exit(1);
      }
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
      if (PROGRAM) {
        sprintf(errmsg, "%s: cannot write %s: %s\n",
                PROGRAM, DEVICE, strerror(errno));
        exit(1);
      } else {
        return -1;
      }
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
      if (PROGRAM) {
        sprintf(errmsg, "%s: cannot read %s: %s\n",
                PROGRAM, DEVICE, strerror(errno));
        exit(1);
      } else {
        return -1;
      }
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
      if (PROGRAM) {
        sprintf(errmsg, "%s: cannot write %s: %s\n",
                PROGRAM, DEVICE, strerror(errno));
        exit(1);
      } else {
        return -1;
      }
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
