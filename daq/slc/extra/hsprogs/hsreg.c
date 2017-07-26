/* ---------------------------------------------------------------------- *\

   hsreg.c - read HSLB FINESSE status registers (separated from staths)

   Mikihiko Nakao, KEK IPNS

   20160114  0.01 initial version

\* ---------------------------------------------------------------------- */

#include <string.h>
#include "libhslb.h"
#include "hsreg.h"

int hsreg_getfee(int fd, hsreg_t *hsp)
{
  int ret;
  int hwtype;
  int serial;
  int fwtype;
  int fwver;

  writefn(fd, HSREG_CSR, 0x05); /* reset address fifo */
  writefn(fd, HSREG_CSR, 0x06); /* reset status register */
  if ((ret = readfn(fd, HSREG_STAT))) {
    return -1;
  }
  writefn(fd, HSREG_FEEHWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEESERIAL, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWTYPE, 0x02); /* dummy value write */
  writefn(fd, HSREG_FEEFWVER,  0x02); /* dummy value write */
  writefn(fd, HSREG_CSR, 0x07);

  if (hswait_quiet(fd) < 0) {
    hsp->feeser = -1;
    hsp->feever = -1;
    hsp->feehw  = -1;
    hsp->feefw  = -1;
    hsp->feecrce = -1;
    return -1;
  }

  hwtype = readfn(fd, HSREG_FEEHWTYPE);
  serial = readfn(fd, HSREG_FEESERIAL);
  fwtype = readfn(fd, HSREG_FEEFWTYPE);
  fwver  = readfn(fd, HSREG_FEEFWVER);

  hsp->feeser = (serial | (hwtype << 8)) & 0xfff;
  hsp->feever = (fwver  | (fwtype << 8)) & 0xfff;
  hsp->feehw  = (hwtype >> 4) & 0x0f;
  hsp->feefw  = (fwtype >> 4) & 0x0f;
  
  hsp->feecrce = readfee8(fd, HSREG_CRCERR);

  return 0;
}

int hsreg_read(int fd, hsreg_t *hsp)
{
  hsp->fintyp   = readfn(fd, HSREG_FINTYPE1) |         /* 7d */
    (readfn(fd, HSREG_FINTYPE2) << 8);    /* 7e */
  if (hsp->fintyp == 0x000a) {
    
    hsp->stat    = readfn(fd, HSREG_STAT);
    hsp->csr     = readfn(fd, HSREG_CSR);
    hsp->xbusy   = readfn(fd, HSREG_CCLK);
    hsp->conf    = readfn(fd, HSREG_CONF);
    hsp->fincpld = readfn(fd, HSREG_CPLDVER);
    hsp->finser  = readfn(fd, HSREG_SERIAL1) |         /* 7b */
      (readfn(fd, HSREG_SERIAL2) << 8);    /* 7c */
    hsp->hslbid  = readfn32(fd, HSREGL_ID);
  } else {
    hsp->stat    = 0;
    hsp->csr     = 0;
    hsp->xbusy   = 0;
    hsp->conf    = 0;
    hsp->fincpld = 0;
    hsp->finser  = 0;
    hsp->hslbid  = 0;
  }

  if (hsp->hslbid == 0x48534c42) {
    int j;
      
    /* bit-5 (frozen ff state) is detected by software */
    hsp->hslbsta    = readfn32(fd, HSREGL_STAT);     /* 83 */
    for (j=0; (hsp->hslbsta&0x20)==0 && j<100; j++) {
      int tmp = readfn32(fd, HSREGL_STAT);
      if ((hsp->hslbsta ^ tmp) & 0x100) break;
    }
    if (j == 100) hsp->hslbsta |= 0x20;
    
    hsp->hslbver    = readfn32(fd, HSREGL_VER);      /* 81 */
    hsp->rxdata     = readfn32(fd, HSREGL_RXDATA);   /* 84 */
    hsp->eventsz    = readfn32(fd, HSREGL_EVENTSZ);  /* 85 */
    hsp->nevent     = readfn32(fd, HSREGL_NEVENT);   /* 86 */
    hsp->nkbyte     = readfn32(fd, HSREGL_NKBYTE);   /* 87 */
    hsp->nword      = readfn32(fd, HSREGL_NWORD);    /* 88 */
    hsp->vetoset    = readfn32(fd, HSREGL_VETOSET);  /* 89 */
    hsp->vetocnt    = readfn32(fd, HSREGL_VETOCNT);  /* 8a */
    hsp->vetobuf[0] = readfn32(fd, HSREGL_VETOBUF0); /* 8b */
    hsp->vetobuf[1] = readfn32(fd, HSREGL_VETOBUF1); /* 8c */
    hsp->cntff00     = readfn32(fd, HSREGL_CNTFF00);     /* 95 */
    hsp->cntdatapkt  = readfn32(fd, HSREGL_CNTDATAPKT);  /* 96 */
    hsp->cntdataword = readfn32(fd, HSREGL_CNTDATAWORD); /* 97 */
  } else {
    hsp->hslbsta    = -1;
    hsp->hslbver    = 0;
    hsp->rxdata     = 0;
    hsp->eventsz    = 0;
    hsp->nevent     = 0;
    hsp->nkbyte     = 0;
    hsp->nword      = 0;
    hsp->vetoset    = 0;
    hsp->vetocnt    = 0;
    hsp->vetobuf[0] = 0;
    hsp->vetobuf[1] = 0;
    hsp->cntff00     = 0;
    hsp->cntdatapkt  = 0;
    hsp->cntdataword = 0;
  }

  if ((hsp->hslbsta & 1) != 0) {
    hsp->feeser = 0xffff;
    hsp->feever = 0xffff;
  }

  return 0;
}
