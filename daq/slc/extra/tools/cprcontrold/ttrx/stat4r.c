/* ---------------------------------------------------------------------- *\

   stat4r.c - TTRX tt4r firmware status

   Mikihiko Nakao, KEK IPNS

   20140103  new
   
\* ---------------------------------------------------------------------- */

#define VERSION 20140103

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "ttrx.h"
#include "tt4r.h"

#define P printf
#define PP printf

char *PROGRAM = 0;
int doassert = 1;

#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define W(r,n,a,b) ((((r)>>(n))&1)?(a):(b))
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#define WARN(msg,cond)                                          \
  do {if(cond){PP("\n"); P("WARNING: %s %s!\n\n", msg, #cond); } } while(0)
#define ASSERT(msg,cond)                                        \
  do {if(cond){ PP("\n"); \
  P("%s: %s %s!\n", doassert?"ABORT":"ERROR",msg, #cond); \
  if(doassert) return;} } while(0)

/* ---------------------------------------------------------------------- *\
   readregs_4r19
\* ---------------------------------------------------------------------- */
void
readregs_4r19(ttrx_t *ttrx, tt4r_t *t)
{
  t->cpldid  = read_ttrx(ttrx, TT4R_CPLDID)  & 0xf;
  t->cpldver = read_ttrx(ttrx, TT4R_CPLDVER) & 0xf;
  t->prgm    = read_ttrx(ttrx, TT4R_PRGM)    & 0xf;
  t->sclk    = read_ttrx(ttrx, TT4R_SCLK)    & 0xf;
  t->serial  = ((read_ttrx(ttrx, TT4R_SER1)  & 0xf) << 0)
    |          ((read_ttrx(ttrx, TT4R_SER2)  & 0xf) << 4)
    |          ((read_ttrx(ttrx, TT4R_SER3)  & 0xf) << 8);

  t->id      = read_ttrx(ttrx, TT4R_ID);
  t->ver     = read_ttrx(ttrx, TT4R_VER);
  t->csr     = read_ttrx(ttrx, TT4R_CSR);
  t->csr2    = read_ttrx(ttrx, TT4R_CSR2);
  t->uio     = read_ttrx(ttrx, TT4R_UIO);
  t->fin     = read_ttrx(ttrx, TT4R_FIN);
  t->uin     = read_ttrx(ttrx, TT4R_UIN);
  t->fsta    = read_ttrx(ttrx, TT4R_FSTA);
  t->uraw    = read_ttrx(ttrx, TT4R_URAW);
  t->utag    = read_ttrx(ttrx, TT4R_UTAG);
  t->revo    = read_ttrx(ttrx, TT4R_REVO);
  t->nibb    = read_ttrx(ttrx, TT4R_NIBB);
  t->nclk    = read_ttrx(ttrx, TT4R_NCLK);
  t->dumtrg  = read_ttrx(ttrx, TT4R_DUMTRG);
  t->tlast   = read_ttrx(ttrx, TT4R_TLAST);
  t->rate    = read_ttrx(ttrx, TT4R_RATE);
  t->dtag    = read_ttrx(ttrx, TT4R_DTAG);
  t->dtagr   = read_ttrx(ttrx, TT4R_DTAGR);
  t->depth   = read_ttrx(ttrx, TT4R_DEPTH);
}
/* ---------------------------------------------------------------------- *\
   tt4r19_show
\* ---------------------------------------------------------------------- */
void
tt4r19_show(const tt4r_t *t)
{
  char *trigsels[] = { "std", "random", "pulse", "none" };
  //char *usels[]    = { "std", "incr", "thru", "usr" };
  char *pidstr;
  int i;

  switch (t->cpldid) {
  case 2: pidstr = "v3"; break;
  case 3: pidstr = "v4 prototype"; break;
  case 4: pidstr = "v4"; break;
  default: pidstr = "v?";
  }
  
  //printf("--------------------------------------");
  //printf("--------------------------------------\n");
  printf("stat4r %d --- ", VERSION);
  printf("TT-RX %s serial #%03d CPLD v0.%02d FPGA v%d.%02d rev %d\n",
	 pidstr, t->serial, t->cpldver,
	 D(t->ver,27,16), D(t->ver,15,8), D(t->ver,7,0));
  //printf("--------------------------------------");
  //printf("--------------------------------------\n");
  
  P("00  cpid=%01x %s\n", t->cpldid, pidstr);
  
  P("01 cpver=%01x (CPLD version 0.%02d)\n", t->cpldver, t->cpldver);
  
  P("02  prgm=%01x done=%d init=%d m012=%d(%s)\n", t->prgm,
    B(t->prgm,3),
    B(t->prgm,2),
    B(t->prgm,1),
    W(t->prgm,1,"pci","flash"));
  
  P("10    id=%08x %s\n", t->id,
    t->id==0x58525454?"OK":(t->id==0x058545454?"OK (Tx mode)":"NG"));
  
  P("11   ver=%08x TT-RX version %d FPGA version %d.%02d rev %02d\n",
    t->ver,
    D(t->ver,31,28),
    D(t->ver,27,16),
    D(t->ver,15,8),
    D(t->ver,7,0));
  
  P("12   csr=%08x nordy=%d entest=%d exbsy=%d nobsy=%d busy=%d ready=%d\n",
    t->csr,
    B(t->csr,16),
    B(t->csr,12),
    B(t->csr,11),
    B(t->csr,10),
    B(t->csr,9),
    B(t->csr,8));
  P("%18scprful=%d disf=%d dish=%d frst=%d orun=%d full=%d half=%d emp=%d\n",
    "",
    B(t->csr,7),
    B(t->csr,6),
    B(t->csr,5),
    B(t->csr,4),
    B(t->csr,3),
    B(t->csr,2),
    B(t->csr,1),
    B(t->csr,0));
  
  P("13  csr2=%08x ulock*=%d urst=%d cksel=%x/%x d=%x en=%x\n",
    t->csr2,
    B(t->csr2,31),
    D(t->csr2,27,24),
    D(t->csr2,23,20),
    D(t->csr2,17,16),
    D(t->csr2,7,4),
    D(t->csr2,3,0));
  
  P("14   uio=%08x uerr=%d uin=%03x uout=%03x o=%x\n",
    t->uio,
    B(t->uio,31),
    D(t->uio,29,20),
    D(t->uio,13,4),
    D(t->uio,3,0));
  
  P("15   fin=%08x nobsy=%x exbsy=%x bsyin=%x mask=%x\n",
    t->fin,
    D(t->fin, 28, 24),
    D(t->fin, 20, 16),
    D(t->fin, 12,  8),
    D(t->fin,  4,  0));
  
  P("16   uin=%08x uerr=%02x/%d udat=%03x cntup=%d link=%d\n",
    t->uin,
    D(t->uin, 31, 24),
    B(t->uin, 14),
    D(t->uin, 8, 0),
    D(t->uin, 23, 16),
    B(t->uin, 15));
  
  P("17  fsta=%08x ful=%x emp=%x nwff=%d bsy=%x\n",
    t->fsta,
    D(t->fsta, 23, 20),
    D(t->fsta, 19, 16),
    B(t->fsta, 15),
    D(t->fsta, 11, 8));
  
  P("18  uraw=%08x linkup=%d err=%d in=",
    t->uraw,
    B(t->uraw,31),
    B(t->uraw,30));
  for (i=29; i>=0; i--) {
    PP("%d", B(t->uraw,i));
    /* if (i && (i%10) == 0) PP(" "); */
    if ((i%10) == 9 || (i%10) == 1) PP("-");
  }
  PP("\n");
  
  P("19  utag=%08x %d\n", t->utag, t->utag);
  P("1a  revo=%08x %d\n", t->revo, t->revo);
  
  P("1b  nibb=%08x %d %d %d\n",
    t->nibb,
    D(t->nibb,23,16),
    D(t->nibb,15,8),
    D(t->nibb,7,0));
  
  P("1c  nclk=%08x sck=%d lck=%d dbg=%02x ckrst=%d\n",
    t->nclk,
    D(t->nclk, 31, 16),
    D(t->nclk, 15, 14),
    D(t->nclk, 13, 8),
    D(t->nclk, 7, 0));
  
  P("20  trig=%08x disbusy=%d decg=%d trigsel=%s",
    t->dumtrg,
    B(t->dumtrg,31),
    B(t->dumtrg,30),
    trigsels[D(t->dumtrg,25,24)]);
  if (D(t->dumtrg,23,0) == 0xffffff) {
    PP(" (free-run)\n");
  } else {
    PP(" n=%d\n", D(t->dumtrg,23,0));
  }
  
  P("21 tlast=%08x", t->tlast);
  if (D(t->tlast,23,0) == 0xffffff) {
    printf(" (free-run)\n");
  } else {
    printf(" n=%d\n", D(t->tlast,23,0));
  }
  
  P("22  rate=%08x refclk=%d Hz[28:24] rndset=%d[17:16] plsset=1/%d[9:0]\n",
    t->rate,
    42333000/(1<<D(t->rate,28,24)),
    D(t->rate,17,16),
    D(t->rate,9,0));
  
  P("23  dtag=%08x %d(%d in fifo)\n", t->dtag, t->dtag, t->dtagr);
  
  P("25 depth=%08x\n", t->depth);
}
/* ---------------------------------------------------------------------- *\
   tt4r19_fifo
\* ---------------------------------------------------------------------- */
void
tt4r19_fifo(ttrx_t *ttrx, const tt4r_t *t)
{
  uint32_t fifo0 = read_ttrx(ttrx, TT4R_FIFO0);
  uint32_t fifo1 = read_ttrx(ttrx, TT4R_FIFO1);
  P("30   fifo0=%08x\n", fifo0);
  P("31   fifo1=%08x\n", fifo1);
}
/* ---------------------------------------------------------------------- *\
   tt4r19_summary
\* ---------------------------------------------------------------------- */
void
tt4r19_summary(const tt4r_t *t)
{
  int trgmode = D(t->dumtrg,25,24);
  char *trgstr;
  double freq = 0;

  switch (trgmode) {
  case 2:
    trgstr = "PULSE";
    /* freq = 42333000.0/(1<<((t->rate>>24)&31))/(1+t->rate&((1<<10)-1)); */
    freq = 42333000.0 / D(t->rate,28,24) / (1 + D(t->rate,9,0));
    break;
  case 1:
    trgstr = "RANDOM";
    /* freq = 42333000.0/(1<<((rate>>24)&31))/512; */
    freq = 42333000.0 / D(t->rate,28,24) / 512;
    break;
  case 0:
    trgstr = "EXT";
    break;
  case 3:
    trgstr = "NONE";
  }
    
  P("trgsrc=%s count=%d", trgstr, t->dtag);
    
  if (freq > 0) PP(" freq=%3.1f Hz", freq);
  if (D(t->dumtrg,23,0) == 0xffffff) {
    PP(" (out of %d)", D(t->dumtrg,23,0));
  } else {
    PP(" (no limit)");
  }
  
  if (B(t->csr,8)) {
    PP(" READY\n");
  } else {
    int rx_fifofull = 0;
    int msk   = D(t->fin,  7,  0);
    int bsy   = D(t->fin, 15,  8) & ~msk;
    int exbsy = D(t->fin, 23, 16) & ~msk;
    int nobsy = D(t->fin, 31, 24) & ~msk;
    
    PP(" NOTREADY fin=%s%s%s%s%s%s",
       W(t->fin,0,"","A"), W(t->fin,1,"","B"),
       W(t->fin,2,"","C"), W(t->fin,3,"","D"),
       W(t->fin,4,"","self"), D(t->fin,4,0) == 0x1f ? "none" : "");
    if (exbsy) {
      PP(" exbusy=%s%s%s%s%s",
	 W(exbsy,0,"A",""), W(exbsy,1,"B",""),
	 W(exbsy,2,"C",""), W(exbsy,3,"D",""), W(exbsy,4,"self",""));
    }
    if (nobsy) {
      PP(" nobusy=%s%s%s%s%s",
	 W(nobsy,0,"A",""), W(nobsy,1,"B",""),
	       W(nobsy,2,"C",""), W(nobsy,3,"D",""), W(nobsy,4,"self",""));
    }
    if (bsy) {
      PP(" busy=%s%s%s%s%s",
	 W(bsy,0,"A",""), W(bsy,1,"B",""),
	 W(bsy,2,"C",""), W(bsy,3,"D",""), W(bsy,4,"self",""));
    }
    
    if (B(t->csr,2) && !B(t->csr,6)) rx_fifofull++; /* full */
    if (B(t->csr,1) && !B(t->csr,5)) rx_fifofull++; /* half */
    
    if (rx_fifofull) {
      PP(" fifo-full");
    }
    
    if (D(t->tlast,23,0) == 0) {
      PP(" trig-limit");
    }
    PP("\n");
  }
}
