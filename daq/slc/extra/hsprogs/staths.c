/* ---------------------------------------------------------------------- *\

   staths.c - HSLB FINESSE status

   Mikihiko Nakao, KEK IPNS

   20140707  0.01 initial version
   20150417  0.02 many modifications
   20150527  0.03 libhslb fix
   20150602  0.04 no HSLB found message

\* ---------------------------------------------------------------------- */

#define VERSION  4
#define MOD_DATE 20150602

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libhslb.h"
#include "hsreg.h"

#ifndef D
#define D(a,b,c) (((a)>>(c))&((1<<((b)+1-(c)))-1))
#define B(a,b)   D(a,b,b)
#define Bs(a,b,s)   (B(a,b)?(s):"")
#define Ds(a,b,c,s)   (D(a,b,c)?(s):"")
#endif

/* ---------------------------------------------------------------------- *\
   getfee
\* ---------------------------------------------------------------------- */
int
getfee(int fd, hsreg_t *hsp)
{
  int i;
  const char *feetype;
  int cmd = 0;
  int cmd_cdc = 0;

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
/* ---------------------------------------------------------------------- *\
   staths
\* ---------------------------------------------------------------------- */
int
staths(int ch, hsreg_t hsregs[])
{
  int i;
  int first = 1;
  int quiet = 1;

  for (i=0; i<4; i++) {
    hsreg_t *h = &hsregs[i];
    double total;
    char prompt[128];
    
    if ((ch & (1<<i)) == 0) continue;

    if (h->hslbid != 0x48534c42) {
      if (ch > 0) {
        if (! first) printf("\n");
        first = 0;
        if (h->fintyp != 0x000a) {
          printf("HSLB-%c invalid fintyp=%04x != 000a\n", 'a'+i, h->fintyp);
        } else if (! (h->conf & 0x80)) {
          printf("HSLB-%c firmware is not programmed\n", 'a'+i);
        } else {
          printf("HSLB-%c invalid firmware=%08x != 48534c42\n",
                 'a'+i, h->hslbid);
        }
        quiet = 0;
      }
      continue;
    }

    quiet = 0;
    if (! first) printf("\n");
    first =0;
    
    sprintf(prompt, "(%c)    ", i+'a');
    
    printf("HSLB-%c version %d.%02d / ",
           i+'a',
           h->hslbver / 100, h->hslbver % 100);
    if (h->hslbsta & 1) {
      printf("b2link is down\n");
    } else if (h->feeser & 0x8000) {
      printf("fee info is not available\n");
    } else {
      printf("%s serial %d version %d\n",
             feename(h->feehw, h->feefw), h->feeser, h->feever);
    }
    if (h->hslbver < 46) {
      printf("%s", prompt);
      printf("(old HSLB firmware < 0.46 gives incorrect stats)\n");
    }
    
    printf("%s", prompt);
    printf("stat=%08x (ff=%x rx=%x pr=%x pt=%x tx=%x%s%s%s%s%s%s%s%s)\n",
           h->hslbsta,
           D(h->hslbsta,11,8),
           D(h->hslbsta,15,12),
           D(h->hslbsta,19,16),
           D(h->hslbsta,23,20),
           D(h->hslbsta,28,24),
           Bs(h->hslbsta,0," linkdown"),
           B(h->hslbsta,31)&&!B(h->hslbsta,1)?" linklost":"",
           Bs(h->hslbsta,5," ffstate"),
           Bs(h->hslbsta,29," pll"),
           Bs(h->hslbsta,7," pll2"),
           Bs(h->hslbsta,1," masked"),
           Bs(h->hslbsta,2," fful"),
           Bs(h->hslbsta,4," nwff"));
           
    printf("%s", prompt);
    printf("rxdata=%04x rxlinkdown=%d rxcrcerr=%d feecrcerr=%d\n",
           D(h->rxdata,15,0), D(h->rxdata,31,24), D(h->rxdata,23,16),
           h->feecrce);
    
    printf("%s", prompt);
    total = (h->nkbyte * 256.0 + (h->nword & 0xff)) * 4.0;
    printf("event=%d total=%1.0fkB", h->nevent, total / 1000);
    if (h->nevent) {
      double avg = total / h->nevent;
      printf(" (avg=%1.0fB", avg);
      if (D(h->eventsz,31,16) == 0xffff) {
        printf(" last=oflow");
      } else {
        printf(" last=%dB", D(h->eventsz,31,16)*4);
      }
      if (D(h->eventsz,15,0) == 0xffff) {
        printf(" max=oflow)");
      } else {
        printf(" max=%dB)", D(h->eventsz,15,0)*4);
      }
    }
    printf("\n");

    printf("%s", prompt);
    if ((h->vetoset & 3) == 0) {
      printf("no b2link error correction");
    } else if (h->vetocnt == 0) {
      printf("no b2link error");
    } else {
      printf("b2link error %d(%c) %d(%c) %c%02x %c%02x %c%02x %c%02x %c%02x %c%02x",
             D(h->vetocnt,31,16),
             B(h->vetoset,1) ? 'i' : '-',
             D(h->vetocnt,15,0),
             B(h->vetoset,0) ? 'd' : '-',
             B(h->vetobuf[0],25)?'K':'D', D(h->vetobuf[0],15,8),
             B(h->vetobuf[0],24)?'K':'D', D(h->vetobuf[0],7,0),
             B(h->vetobuf[0],21)?'K':'D', D(h->vetobuf[1],31,24),
             B(h->vetobuf[0],20)?'K':'D', D(h->vetobuf[1],23,16),
             B(h->vetobuf[0],17)?'K':'D', D(h->vetobuf[1],15,8),
             B(h->vetobuf[0],16)?'K':'D', D(h->vetobuf[1],7,0));
    }
    printf("\n");
  }

  if (quiet) {
    printf("no HSLB was found.\n");
  }
}

/* ---------------------------------------------------------------------- *\
   readregs
\* ---------------------------------------------------------------------- */
int
readregs(int ch, hsreg_t hsregs[])
{
  int i;
  int fd;
  int o_readonly = 1;
  int notfound = 1;

  for (i=0; i<4; i++) {
    hsreg_t *hsp = &hsregs[i];
    memset(hsp, 0, sizeof(*hsp));
    if ((ch & (1<<i)) == 0) continue;
    if ((fd = openfn(i, o_readonly, ch < 0 ? 0 : "staths")) < 0) {
      /* if ch >= 0, giving "staths" program name will cause the program
         to exit for any error */
      continue;
    }
    
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

        hsp->feeser = 0xffff;
        hsp->feever = 0xffff;
        if ((hsp->hslbsta & 1) == 0) {
          getfee(fd, hsp);
        }
      }
    }
    close(fd);
  }
  return notfound;
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int ch = 0;
  hsreg_t hsregs[4];

  while (argc > 1) {
    if (argv[1][0] == '-') {
      int i;
      for (i=1; argv[1][i]; i++) {
        switch (argv[1][i]) {
        case 'a':
        case 'b':
        case 'c':
        case 'd': ch |= (1 << (argv[1][i]-'a')); break;
        }
      }
    }
    argc--, argv++;
  }

  if (ch == 0) ch = -1;

  printf("staths version %d (%d)\n", VERSION, MOD_DATE);
  readregs(ch, hsregs);
  staths(ch, hsregs);
}
