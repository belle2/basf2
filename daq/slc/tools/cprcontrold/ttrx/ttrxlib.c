/* ---------------------------------------------------------------------- *\
   ttrxlib.c

   both user mode with driver and direct access will be used

   Mikihiko Nakao, KEK IPNS

   version
   2003120400  working version
   (there must be many versions in between)
   2005081500  start_ttrx and stop_ttrx
\* ---------------------------------------------------------------------- */

static char VERSION[] = "2005081500";

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/mman.h>
#ifdef __linux__
#include <asm/page.h>
#endif
#include <errno.h>

#include "ttrx.h"

#define TTXX_T         ttrx_t
#define OPEN_TTXX      open_ttrx
#define CLOSE_TTXX     close_ttrx
#define READ_TTXX      read_ttrx
#define READFIFO_TTXX  readfifo_ttrx
#define WRITE_TTXX     write_ttrx
#define DEBUG_TTXX     debug_ttrx
#define TTXX_RDWR      TTRX_RDWR
#define TTXX_RDONLY    TTRX_RDONLY
#define TTXX_MMAPMODE  TTRX_MMAPMODE
#define TTXX_MMAPSIZE  TTRX_MMAPSIZE
#define TTXX_DEVNAME   TTRX_DEV_NAME
#define TTXX_DEVFIFONAME   TTRX_DEVFIFO_NAME

#define R(r,n,w)   (((r)>>(n))&((1<<w)-1))
static int debug = 0;

/* ---------------------------------------------------------------------- *\
   get_ttrx
\* ---------------------------------------------------------------------- */

#define	SYS_BUS_PCI_DEVICES	"/sys/bus/pci/devices"
static int
get_ttrx(unsigned int *base_plx_p, unsigned int *base_rx_p, int rxid)
{
  DIR * sys_bus_pci_devices = opendir(SYS_BUS_PCI_DEVICES);
  struct dirent * entry;
  char fname[1024];
  
  FILE *fp_bus;
  char buf[256];
  int bus, dev, func;
  int plx = 0;
  int devvendor, devid, subvendor, subid;
  unsigned int base0, base1;

  if (! sys_bus_pci_devices) {
    perror("cannot fopen /sys/bus/pci/devices\n");
    exit(1);
  }

  memset(buf, 0, sizeof(buf));
  while (entry = readdir(sys_bus_pci_devices)) {
    if (entry->d_name[0] == '.') {
	continue;
    }
    strncpy(fname, SYS_BUS_PCI_DEVICES, sizeof(fname));
    strncat(fname, "/", sizeof(fname));
    strncat(fname, entry->d_name, sizeof(fname));
    strncat(fname, "/config", sizeof(fname));

    if (! (fp_bus = fopen(fname, "r"))) {
      fprintf(stderr, "cannot fopen %s\n", fname);
      continue;
    }
    fread(buf, 256, 1, fp_bus);
    fclose(fp_bus);

    devvendor = *(unsigned short *)(buf + 0x00);
    devid     = *(unsigned short *)(buf + 0x02);
    subvendor = *(unsigned short *)(buf + 0x2c);
    subid     = *(unsigned short *)(buf + 0x2e);
    base0     = *(unsigned int   *)(buf + 0x10);
    base1     = *(unsigned int   *)(buf + 0x18);

    if (devvendor == 0x10b5 && devid == 0x9054 &&
	subvendor == 0x10b5 && subid == 0x5258) {
      printf("TTRX is found at %s\n", entry->d_name);
      if (rxid-- == 0) {
	*base_plx_p  = base0;
	*base_rx_p = base1;
	return 0;
      }
    }
  }
  closedir(sys_bus_pci_devices);
  return -1;
}
/* ---------------------------------------------------------------------- *\
   linux_mmap
\* ---------------------------------------------------------------------- */
static unsigned *
linux_mmap(int baseadrs, int mapsize, int oflags)
{
  int mflags = ((oflags == O_RDWR) ? PROT_WRITE : 0) | PROT_READ;
  int fd     = open("/dev/mem", oflags, 0);
  unsigned *mapadrs;

  if (fd < 0) return 0;

  if (baseadrs % PAGE_SIZE) {
    errno = EFAULT;
    return 0;
  }

  mapadrs = (unsigned *)mmap((caddr_t) 0, mapsize, mflags, MAP_SHARED,
			     fd, baseadrs);

  if (mapadrs == (unsigned *)-1) return 0;

  return mapadrs;
}
/* ---------------------------------------------------------------------- *\
   OPEN_TTXX
\* ---------------------------------------------------------------------- */
TTXX_T *
OPEN_TTXX(int id, int flag)
{
  int oflag    = flag & TTXX_RDWR ? O_RDWR : O_RDONLY;
  TTXX_T *ttxx = malloc(sizeof(TTXX_T));

  if (! ttxx) {
    errno = ENOSPC;
    return 0;
  }
  ttxx->id = id;
  ttxx->writeflag = flag & TTXX_RDWR;
  ttxx->fd = -1;
  ttxx->fdfifo = -1;
  ttxx->mapadrs = 0;
  ttxx->prev = 0;
  
  if (flag & TTRX_MMAPMODE) {
    unsigned int plxbase, baseadrs;
    if (get_ttrx(&plxbase, &baseadrs, id) < 0) {
      free(ttxx);
      errno = ENXIO;
      return 0;
    }
    if (! (ttxx->mapadrs = linux_mmap(baseadrs, TTXX_MMAPSIZE, oflag))) {
      free(ttxx);
      return 0;
    }
  } else {
    char devname[256];
    sprintf(devname, "%s%d", TTXX_DEVNAME, id);
    if ((ttxx->fd = open(devname, oflag)) < 0) {
      free(ttxx);
      return 0;
    }
    if (flag & TTRX_USEFIFO) {
      sprintf(devname, "%s%d", TTRX_DEVFIFO_NAME, id);
      if ((ttxx->fdfifo = open(devname, O_RDONLY)) < 0) {
        free(ttxx);
        return 0;
      }
    }
  }
  return ttxx;
}
/* ---------------------------------------------------------------------- *\
   CLOSE_TTXX
\* ---------------------------------------------------------------------- */
int
CLOSE_TTXX(TTXX_T *ttxx)
{
  if (ttxx && (ttxx->fd > 0 || ttxx->mapadrs)) {
    if (ttxx->fd > 0) close(ttxx->fd);
    if (ttxx->fdfifo > 0) close(ttxx->fdfifo);
    memset(ttxx, 0, sizeof(ttxx)); /* to avoid double free */
    free(ttxx);
    return 0;
  } else {
    return -1;
  }
}
/* ---------------------------------------------------------------------- *\
   READ_TTXX
\* ---------------------------------------------------------------------- */
int
READ_TTXX(TTXX_T *ttxx, int offset)
{
  if (! ttxx) return -4;
  if (ttxx->fd > 0) {
    int value = 0;
    if (lseek(ttxx->fd, offset, SEEK_SET) < 0) return -2;
    if (read(ttxx->fd, &value, sizeof(value)) < 0) return -1;
    return value;
  } else if (ttxx->mapadrs) {
    if (ttxx->prev != 0) usleep(1);
    ttxx->prev = 0;
    return ttxx->mapadrs[offset];
  } else {
    return -3;
  }
}
/* ---------------------------------------------------------------------- *\
   READFIFO_TTXX
\* ---------------------------------------------------------------------- */
int
READFIFO_TTXX(TTXX_T *ttxx, unsigned *bufp)
{
  if (! ttxx) return -4;
  if (! bufp) return -5;
  bufp[0] = bufp[1] = (unsigned)-1;
  if (ttxx->fdfifo > 0) {
    int value = 0;
    if (read(ttxx->fdfifo, bufp, 2*sizeof(int)) < 2*sizeof(int)) return -1;
  } else if (ttxx->mapadrs) {
    if (ttxx->prev != 0) usleep(1);
    ttxx->prev = 0;
    bufp[0] = ttxx->mapadrs[TTRX_FIFO1];
    bufp[1] = ttxx->mapadrs[TTRX_FIFO2];
  } else {
    return -3;
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   WRITE_TTXX
\* ---------------------------------------------------------------------- */
int
WRITE_TTXX(TTXX_T *ttxx, int offset, int value)
{
  if (! ttxx) return -4;
  if (ttxx->fd > 0) {
    if (debug & 1) printf("write_ttrx: %08x <= %08x\n", offset<<2, value);
    if (lseek(ttxx->fd, offset, SEEK_SET) < 0) return -2;
    if (write(ttxx->fd, &value, sizeof(value)) < 0) return -1;
    return 0;
  } else if (ttxx->mapadrs) {
    if (ttxx->prev != offset+1) usleep(1);
    ttxx->prev = offset+1;
    if (debug & 1) printf("write_ttrx: %08x <= %08x\n", offset<<2, value);
    ttxx->mapadrs[offset] = value;
    return 0;
  } else {
    return -3;
  }
}
/* ---------------------------------------------------------------------- *\
   DEBUG_TTXX
\* ---------------------------------------------------------------------- */
void
DEBUG_TTXX(int val)
{
  debug = val;
}
/* ---------------------------------------------------------------------- *\
   stop_ttrx
\* ---------------------------------------------------------------------- */
int
stop_ttrx(ttrx_t *ttrx)
{
  int id, trig;

  id = read_ttrx(ttrx, TTRX_ID);

  if (id != 0x58525454) {
    return -1; /* not a TT-RX? */
  }

  trig = (3 << 24) | (1 << 31);      /* disable trigger before FIFO reset */
  write_ttrx(ttrx, TTRX_TRIG, trig); /* and disable busy during setup */
  return 0;
}
/* ---------------------------------------------------------------------- *\
   start_ttrx_opt
   start TT-RX in the default mode (external clock and trigger)
\* ---------------------------------------------------------------------- */
static int
start_ttrx_opt(ttrx_t *ttrx, int nofifo)
{
  int i;
  unsigned csr, xmask, trig, rate, nim;

  if (! ttrx || stop_ttrx(ttrx) < 0) {
    return -1;
  }

  write_ttrx(ttrx, TTRX_MASK, 0x10);

  csr = 0;
  if (nofifo) csr |= (3<<5); /* fifo=no */
  csr |= (1<<11);  /* clear-fifo */
  csr |= (1<<7);   /* clear-exbusy */
  csr |= (1<<26);  /* uclk */
  write_ttrx(ttrx, TTRX_CSR, csr);
     
  csr = read_ttrx(ttrx, TTRX_CSR);
  if (csr & (1<<9))  return -2; /* busy is not cleared */
  if (csr & (1<<11)) return -3; /* exbusy is not cleared */
  if (csr & (1<<29)) return -4; /* TTD link failure */

  rate = 0;
  write_ttrx(ttrx, TTRX_RATE, rate);

  trig = 0x80ffffff; /* stop trig with EXT-free setup */
  write_ttrx(ttrx, TTRX_TRIG, trig);

  trig = 0x00ffffff; /* start trig with EXT-free setup */
  write_ttrx(ttrx, TTRX_TRIG, trig);

  return 0;
}
/* ---------------------------------------------------------------------- *\
   start_ttrx and start_ttrx_nofifo
   start TT-RX in the default mode (external clock and trigger)
\* ---------------------------------------------------------------------- */
int start_ttrx(ttrx_t *ttrx)        { return start_ttrx_opt(ttrx, 0); }
int start_ttrx_nofifo(ttrx_t *ttrx) { return start_ttrx_opt(ttrx, 1); }
/* ---------------------------------------------------------------------- *\
   init_ttrx
\* ---------------------------------------------------------------------- */
ttrx_t *
init_ttrx()
{
  ttrx_t *ttrx = open_ttrx(0, TTRX_RDWR|TTRX_USEFIFO);
  if (ttrx) {
    if (stop_ttrx(ttrx) == 0) return ttrx;
    close_ttrx(ttrx);
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   trgdelay_ttrx()
\* ---------------------------------------------------------------------- */
int
trgdelay_ttrx(ttrx_t *ttrx, int delay_in_sclk_unit)
{
  int depth = read_ttrx(ttrx, TTRX_DEPTH);
  if (delay_in_sclk_unit >= 0 && delay_in_sclk_unit < 256) {
    depth &= ~0xff00;
    depth |= (delay_in_sclk_unit << 8);
    write_ttrx(ttrx, TTRX_DEPTH, depth);
  }
  return (depth >> 8) & 0xff;
}
/* ---------------------------------------------------------------------- *\
   getversion_ttrx()
\* ---------------------------------------------------------------------- */
char *
getversion_ttrx(ttrx_t *ttrx)
{
  static char buf[256];
  if (! ttrx) return "no TT-RX";
  int pid  = read_ttrx(ttrx, TTRX_PID);
  int pver = read_ttrx(ttrx, TTRX_PVER);
  int ser1 = read_ttrx(ttrx, TTRX_SER1);
  int ser2 = read_ttrx(ttrx, TTRX_SER2);
  int ser3 = read_ttrx(ttrx, TTRX_SER3);
  int id   = read_ttrx(ttrx, TTRX_ID);
  int ver  = read_ttrx(ttrx, TTRX_VER);
  int serial = ser1 + ser2 * 16 + ser3 * 256;

  if (pid != 4 || id != 0x58525454) return "broken TT-RX firmware";
  sprintf(buf, "TT-RX v4 #%03d CPLD v0.%02d FPGA v%d.%02dr%d",
	  serial, pver, R(ver,16,12), R(ver,8,8), R(ver,0,8));
  return buf;
}
