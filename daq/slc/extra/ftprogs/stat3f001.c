/*
  stat3f001.c
  
  20150309 first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "ftsw.h"
#include "ft3f001.h"

typedef ft3f_t fast_t;
typedef ft3f_t slow_t;

#include "statft.h"

/* ---------------------------------------------------------------------- *\
   regs3f001
\* ---------------------------------------------------------------------- */
void
regs3f001(ftsw_t *ftsw, ft3f_t *f)
{
  f->sta = read_ftsw(ftsw, FT3F_STA);
}
/* ---------------------------------------------------------------------- *\
   dump3f001
\* ---------------------------------------------------------------------- */
void
dump3f001(ftsw_t *ftsw)
{
  /* to make dump0..7,k working, it has to fetched once by writing 1 to 89 */
  ft3f_t ubuf;
  ft3f_t *d = &ubuf;
  
  write_ftsw(ftsw, FT3F_DUMPI, 1);
  usleep(100*1000); /* 100ms sleep */

  d->dumpo[0] = read_ftsw(ftsw, FT3F_DUMPO0); /* 80(R) dump octet */
  d->dumpo[1] = read_ftsw(ftsw, FT3F_DUMPO1); /* 81(R) dump octet */
  d->dumpo[2] = read_ftsw(ftsw, FT3F_DUMPO2); /* 82(R) dump octet */
  d->dumpo[3] = read_ftsw(ftsw, FT3F_DUMPO3); /* 83(R) dump octet */
  d->dumpo[4] = read_ftsw(ftsw, FT3F_DUMPO4); /* 84(R) dump octet */
  d->dumpo[5] = read_ftsw(ftsw, FT3F_DUMPO5); /* 85(R) dump octet */
  d->dumpo[6] = read_ftsw(ftsw, FT3F_DUMPO6); /* 86(R) dump octet */
  d->dumpo[7] = read_ftsw(ftsw, FT3F_DUMPO7); /* 87(R) dump octet */
  d->dumpk    = read_ftsw(ftsw, FT3F_DUMPK);  /* 88(R) dump K-symbol */
  d->dumpi    = read_ftsw(ftsw, FT3F_DUMPI);  /* 89(R) dump info */

  d->dump[0]  = read_ftsw(ftsw, FT3F_DUMP0);  /* 90(R) dump raw input */
  d->dump[1]  = read_ftsw(ftsw, FT3F_DUMP1);  /* 91(R) dump raw input */
  d->dump[2]  = read_ftsw(ftsw, FT3F_DUMP2);  /* 92(R) dump raw input */
  d->dump[3]  = read_ftsw(ftsw, FT3F_DUMP3);  /* 93(R) dump raw input */
  d->dump[4]  = read_ftsw(ftsw, FT3F_DUMP4);  /* 94(R) dump raw input */
  d->dump[5]  = read_ftsw(ftsw, FT3F_DUMP5);  /* 95(R) dump raw input */
  d->dump[6]  = read_ftsw(ftsw, FT3F_DUMP6);  /* 96(R) dump raw input */
  d->dump[7]  = read_ftsw(ftsw, FT3F_DUMP7);  /* 97(R) dump raw input */
  d->dump[8]  = read_ftsw(ftsw, FT3F_DUMP8);  /* 98(R) dump raw input */
  d->dump[9]  = read_ftsw(ftsw, FT3F_DUMP9);  /* 99(R) dump raw input */

  printf("d->dumpk / i = %08x / %08x\n", d->dumpk, d->dumpi );

  statft_dump(d->dumpk, d->dumpi, d->dumpo, d->dump);
}
/* ---------------------------------------------------------------------- *\
   stat3f
\* ---------------------------------------------------------------------- */
#ifndef POCKET_TTD
void
stat3f001(ftsw_t *ftsw, int ftswid)
{
  struct timeval tv;
  int fpgaver = read_ftsw(ftsw, FTSWREG_FPGAVER) & 0x3ff;
  
  printf("ft3f%03d - under construction.\n", fpgaver);

  if (! showdump) {
    ft3f_t f;
    regs3f001(ftsw, &f);
    P("20     sta=%08x ck%s tt%s trg%s rst%s bit%d(cnt%d) %c%02x%s\n",
      f.sta,
      B(f.sta,30) ? "up" : "dn",
      B(f.sta,31) ? "up" : "dn",
      B(f.sta,29) ? "H" : "L",
      B(f.sta,28) ? "H" : "L",
      D(f.sta,11,10),
      D(f.sta,14,12),
      B(f.sta,8)?'K':'D',
      D(f.sta,7,0),
      B(f.sta,9)?"C":"");
  }
  

  if (showdump) {
    dump3f001(ftsw);
    return;
  }
}
#endif
