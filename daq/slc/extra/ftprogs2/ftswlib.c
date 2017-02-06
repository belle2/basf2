/* ---------------------------------------------------------------------- *\
   ftswlib.c

   both solaris and vxworks codes

   Mikihiko Nakao, KEK IPNS

   version
   2010091800  copied from ttiolib.c
   2014101500  widened range to open multiple FTSWs
   2014103100  range limit
   2016040800  to coexist ftsw and ttio
\* ---------------------------------------------------------------------- */

static char VERSION[] = "2016040800";

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#ifdef USE_LINUX_VME_UNIVERSE
/* #ifdef __linux__ */
#include <asm/page.h>
#endif
#include <errno.h>

#include "ftsw.h"

#define TTXX_T         ftsw_t
#define OPEN_TTXX      open_ftsw
#define CLOSE_TTXX     close_ftsw
#define READ_TTXX      read_ftsw
#define READFIFO_TTXX  readfifo_ftsw
#define WRITE_TTXX     write_ftsw
#define DEBUG_TTXX     debug_ftsw
#define TTXX_RDWR      FTSW_RDWR
#define TTXX_RDONLY    FTSW_RDONLY
#define TTXX_MMAPMODE  FTSW_MMAPMODE
#define TTXX_MMAPSIZE  FTSW_MMAPSIZE
#define TTXX_MAXID     FTSW_MAXID
#define TTXX_BASEID    FTSW_BASEID
#define TTXX_VMEBASE   FTSW_VMEBASE
#define TTXX_VMEUNIT   FTSW_VMEUNIT
#define TTXX_FIFO1     FTSW_FIFO1
#define TTXX_FIFO2     FTSW_FIFO2
#define TTXX_FIFO3     FTSW_FIFO3
#define TTXX_FIFO4     FTSW_FIFO4

static int debug = 0;

/* ---------------------------------------------------------------------- *\
   sparc_mmap
\* ---------------------------------------------------------------------- */
static unsigned *
sparc_mmap(int baseadrs, int mapsize, int oflags)
{
  int mflags = ((oflags == O_RDWR) ? PROT_WRITE : 0) | PROT_READ;
  int fd     = open(SPARC_VMEDEV, oflags, 0);
  unsigned *mapadrs;

  if (fd < 0) return 0;

  mapadrs = (unsigned *)mmap((caddr_t) 0, mapsize, mflags, MAP_SHARED,
			     fd, baseadrs);

  if (mapadrs == (unsigned *)-1) return 0;

  return mapadrs;
}
/* ---------------------------------------------------------------------- *\
   linux_vmic_mmap
\* ---------------------------------------------------------------------- */
#ifdef USE_LINUX_VME_UNIVERSE
static unsigned *
linux_vmic_mmap(int baseadrs, int mapsize, int oflags,
		vme_bus_handle_t *busp, vme_master_handle_t *windowp)
{
  unsigned *ptr;

  if (vme_init(busp)) {
    errno = ENOENT;
    return 0;
  }
  vme_set_master_endian_conversion(*busp, 1);

  if (vme_master_window_create(*busp, windowp,
			       baseadrs, VME_A32SD, mapsize,
			       0 /*VME_CTL_PWEN*/, NULL)) {
    vme_term(*busp);
    errno = EACCES;
    return 0;
  }

  if (! (ptr = vme_master_window_map(*busp, *windowp, 0))) {
    vme_master_window_release(*busp, *windowp);
    vme_term(*busp);
    errno = EFAULT;
    return 0;
  }
  return ptr;
}
/* ---------------------------------------------------------------------- *\
   linux_vmic_munmap
\* ---------------------------------------------------------------------- */
static int
linux_vmic_munmap(vme_bus_handle_t bus, vme_master_handle_t window)
{
  if (vme_master_window_unmap(bus, window)) {
    errno = ENOENT;
    vme_master_window_release(bus, window);
    vme_term(bus);
    return -1;
  }

  if (vme_master_window_release(bus, window)) {
    errno = EBUSY;
    vme_term(bus);
    return -1;
  }

  if (vme_term(bus)) {
    perror("Error terminating");
    return -1;
  }
  return 0;
}
#endif
/* ---------------------------------------------------------------------- *\
   OPEN_TTXX
\* ---------------------------------------------------------------------- */
TTXX_T *
OPEN_TTXX(int id, int flag)
{
  /* int baseadrs = TTXX_VMEBASE + TTXX_VMEUNIT * id; */
  int baseadrs = TTXX_VMEBASE;
  int oflag    = flag & TTXX_RDWR ? O_RDWR : O_RDONLY;
  TTXX_T *ttxx = malloc(sizeof(TTXX_T));
  int fd;
  
  if (! ttxx) {
    errno = ENOSPC;
    return 0;
  }
  memset(ttxx, 0, sizeof(TTXX_T));
  ttxx->id = id;
  ttxx->writeflag = flag & TTXX_RDWR;
  ttxx->fd = -1;
  ttxx->fdfifo = -1;
  ttxx->mapadrs = 0;

#if VXWORKS
  ttxx->mapadrs = (unsigned *)baseadrs;
#elif USE_LINUX_VME_UNIVERSE
  if (id < 0 || id >= TTXX_MAXID) return 0;

  /* nakao: vme_universe returns only ENOENT and it's no use for diagnosis */
  if ((fd = open("/dev/bus/vme/ctl", O_RDONLY)) < 0) {
    return 0; /* rely on "errno" that "open" returns */
  }
  close(fd);
  if (oflag == O_RDWR && (fd = open("/dev/bus/vme/ctl", oflag)) < 0) {
    return 0; /* rely on "errno" that "open" returns */
  }
  close(fd);

  ttxx->mapadrs = linux_vmic_mmap(baseadrs, TTXX_MMAPSIZE*512, oflag,
				  &ttxx->bus_handle, &ttxx->window_handle);
  if (ttxx->mapadrs) {
    ttxx->mapadrs = (unsigned *)((char *)ttxx->mapadrs
                                 + TTXX_MMAPSIZE*(id+TTXX_BASEID));
  }
#else /* __sparc__ */
  ttxx->mapadrs = sparc_mmap(baseadrs, TTXX_MMAPSIZE, oflag);
#endif

  if (ttxx->mapadrs == 0 && ttxx->fd < 0) {
    free(ttxx);
    errno = ENODEV;
    return 0;
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
#ifdef USE_LINUX_VME_UNIVERSE
    linux_vmic_munmap(ttxx->bus_handle, ttxx->window_handle);
#endif
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
    return ttxx->mapadrs[offset];
  } else {
    return -3;
  }
}
/* ---------------------------------------------------------------------- *\
   READFIFO_TTXX
\* ---------------------------------------------------------------------- */
/*
int
READFIFO_TTXX(TTXX_T *ttxx, unsigned *bufp)
{
  if (! ttxx) return -4;
  if (! bufp) return -5;
  bufp[0] = bufp[1] = -1;
  if (ttxx->fdfifo > 0) {
    int value = 0;
    if (read(ttxx->fdfifo, bufp, 4*sizeof(int)) < 4*sizeof(int)) return -1;
  } else if (ttxx->mapadrs) {
#ifdef TTXX_FIFO1
    bufp[0] = ttxx->mapadrs[TTXX_FIFO1];
    bufp[1] = ttxx->mapadrs[TTXX_FIFO2];
    bufp[2] = ttxx->mapadrs[TTXX_FIFO3];
    bufp[3] = ttxx->mapadrs[TTXX_FIFO4];
#else
    return -6;
#endif
  } else {
    return -3;
  }
  return 0;
}
*/
/* ---------------------------------------------------------------------- *\
   WRITE_TTXX
\* ---------------------------------------------------------------------- */
int
WRITE_TTXX(TTXX_T *ttxx, int offset, int value)
{
  if (! ttxx) return -4;
  if (ttxx->fd > 0) {
    if (debug & 1) printf("write_ftsw: %08x <= %08x\n", offset<<2, value);
    if (lseek(ttxx->fd, offset, SEEK_SET) < 0) return -2;
    if (write(ttxx->fd, &value, sizeof(value)) < 0) return -1;
    return 0;
  } else if (ttxx->mapadrs) {
    if (debug & 1) printf("write_ftsw: %08x <= %08x\n", offset<<2, value);
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
