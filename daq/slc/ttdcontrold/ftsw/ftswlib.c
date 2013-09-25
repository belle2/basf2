/* ---------------------------------------------------------------------- *\
   ftswlib.c

   both solaris and vxworks codes

   Mikihiko Nakao, KEK IPNS

   version
   2010091800  copied from ttiolib.c
\* ---------------------------------------------------------------------- */

static char VERSION[] = "2010091800";

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#ifdef __linux__
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
  int baseadrs = TTXX_VMEBASE + TTXX_VMEUNIT * id;
  int oflag    = flag & TTXX_RDWR ? O_RDWR : O_RDONLY;
  TTXX_T *ttxx = malloc(sizeof(TTXX_T));
  
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
  ttxx->mapadrs = linux_vmic_mmap(baseadrs, TTXX_MMAPSIZE, oflag,
				  &ttxx->bus_handle, &ttxx->window_handle);
#else /* __sparc__ */
  ttxx->mapadrs = sparc_mmap(baseadrs, TTXX_MMAPSIZE, oflag);
#endif

  if (ttxx->mapadrs == 0 && ttxx->fd < 0) {
    free(ttxx);
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

/* ---------------------------------------------------------------------- *\
   write_ftsw_fpga
\* ---------------------------------------------------------------------- */
void write_ftsw_fpga(ftsw_t *ftsw, int m012, int ch, int n) {
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
      /* data = ftsw_read(ftsw, FTSWREG_CCLK); */
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
    write_ftsw(ftsw, FTSWREG_CCLK, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_ftsw_fpga
\* ---------------------------------------------------------------------- */
void dump_ftsw_fpga(int conf, char *str) {
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_ftsw_fpga
\* ---------------------------------------------------------------------- */
int boot_ftsw_fpga(ftsw_t *ftsw, const char *file, int verbose, int forced, int m012) {
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
  if (verbose) dump_ftsw_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  write_ftsw(ftsw, FTSWREG_CONF, 0x08 | m012);
  conf = read_ftsw(ftsw, FTSWREG_CONF) & 7;
  if (verbose || conf != m012) dump_ftsw_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d?).\n", conf);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  write_ftsw(ftsw, FTSWREG_CONF, 0x41);
  write_ftsw(ftsw, FTSWREG_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose || (conf & 0x80)) dump_ftsw_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  write_ftsw(ftsw, FTSWREG_CONF, 0x86);
  dump_ftsw_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_ftsw_fpga(conf, "(PRGM=0)");
  
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
    write_ftsw_fpga(ftsw, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 100000) == 0) {
      printf("%d bytes written (%d)\n", count, (int)time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_ftsw_fpga(ftsw, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) write_ftsw_fpga(ftsw, m012, 0xff, nbyte++);
  fclose(fp);

  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_ftsw_fpga(conf, "before");
  /* write_ftsw(ftsw, FTSWREG_CONF, 0x40);  clear ce_b */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_ftsw_fpga(conf, "clr ce_b");
  write_ftsw(ftsw, FTSWREG_CONF, 0x0f); /* clear m012 = 6 */
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  dump_ftsw_fpga(conf, "clr m012");
  conf = read_ftsw(ftsw, FTSWREG_CONF);
  if (verbose & ! (conf & 0x80)) dump_ftsw_fpga(conf, "");
  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
}
/* ---------------------------------------------------------------------- *\
   trigger_ftsw_single
\* ---------------------------------------------------------------------- */
int trigger_ftsw_single(ftsw_t *ftsw) {
  int offset = strtoul("450", 0, 16) >> 2;
  int value = 1;
  return write_ftsw(ftsw, offset, value);
}
/* ---------------------------------------------------------------------- *\
   stop_ftsw_trigger
\* ---------------------------------------------------------------------- */
int stop_ftsw_trigger(ftsw_t *ftsw) {
  int offset = strtoul("190", 0, 16) >> 2;
  int value = 10000000;
  return write_ftsw(ftsw, offset, value);
}
