/* ---------------------------------------------------------------------- *\

   reghs.c - HSLB FINESSE register access

   Mikihiko Nakao, KEK IPNS

   20140426  0.01 initial version (copy from regmgt.c 2009121600)
   20140428  0.02 read_window trial
   20140428  0.03 readback implementation
   20140430  0.04 reghs_stat check
   20140513  0.05 general fee register access
   20140606  0.06 a16d32 fee access, fee version etc rearranged
   20140607  0.07 (read|write)fee(8|32) as separate functions
   20140628  0.08 libhslb.c, stream
   20140703  0.09 byte stream write fix (argv[2])
   20140714  0.10 DEMO FEE type
   20140926  0.11 FEE type update to include EECL / EKLM
   20150331  0.12 readonly flag
   20150416  0.13 linked with new libhslb / test_repeat flag
   
\* ---------------------------------------------------------------------- */

#define VERSION  13
#define MOD_DATE 20150416

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "libhslb.h"
#include "hsreg.h"

char *ARGV0  = 0;
char *DEVICE = 0;

/* 0 parameter */
#define HSDO_NOP      0x100
#define HSDO_LINK     0x101
#define HSDO_CHECKFEE 0x102
#define HSDO_TRGOFF   0x104
#define HSDO_TRGON    0x105

/* 0 parameter (obsolete, unused) */
#define HSDO_TTTRG    0x106
#define HSDO_DUMTRG   0x107

/* 0 parameter (CDC special) */
#define HSDO_SUPMODE  0x108
#define HSDO_RAWMODE  0x109

/* 1 parameter */
#define HSDO_STREAM   0x200

/* 1 parameter (CDC special) */
#define HSDO_PDSTLALL 0x201
#define HSDO_ADCTH    0x202
#define HSDO_TDCTH    0x203
#define HSDO_DELAY    0x204
#define HSDO_WINDOW   0x205

/* 2 parameters */
#define HSDO_FEE8     0x300
#define HSDO_FEE32    0x301

/* 2 parameters (CDC special) */
#define HSDO_PDSTL    0x302

int test_repeat = 0;

/* ---------------------------------------------------------------------- *\
   hsdo
\* ---------------------------------------------------------------------- */
int
hsdo(int fin, int fd, int op, int val, int val2, int rd)
{
  int i;
  const char *feetype;
  int cmd = 0;
  int cmd_cdc = 0;

  switch (op) {
  case HSDO_LINK:    cmd = 0x01; break;
  case HSDO_TRGOFF:  cmd = 0x03; break;
  case HSDO_TRGON:   cmd = 0x04; break;
  case HSDO_SUPMODE: cmd_cdc = 0x07; break; /* suppress mode */
  case HSDO_RAWMODE: cmd_cdc = 0x08; break; /* raw mode */
  }

  if (op == HSDO_CHECKFEE) {
    int ret;
    int hwtype;
    int serial;
    int fwtype;
    int fwver;
    static const char *feetype[] = {
      "UNDEF", "SVD", "CDC", "BPID", "EPID", "ECL", "EECL", "KLM", "EKLM",
      "TRG", "UNKNOWN-10", "UNKNOWN-11",
      "UNKNOWN-12", "UNKNOWN-13", "DEMO", "TEST" };
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


    writefn(fd, HSREG_CSR, 0x05); /* reset address fifo */
    writefn(fd, HSREG_CSR, 0x06); /* reset status register */
    if ((ret = readfn(fd, HSREG_STAT))) {
      printf("checkfee: cannot clear HSREG_STAT=%02x\n", ret);
      return -1;
    }
    writefn(fd, HSREG_FEEHWTYPE, 0x02); /* dummy value write */
    writefn(fd, HSREG_FEESERIAL, 0x02); /* dummy value write */
    writefn(fd, HSREG_FEEFWTYPE, 0x02); /* dummy value write */
    writefn(fd, HSREG_FEEFWVER,  0x02); /* dummy value write */
    writefn(fd, HSREG_CSR, 0x07);

    if (hswait(fin, fd) < 0) {
      return -1;
    }

    hwtype = readfn(fd, HSREG_FEEHWTYPE);
    serial = readfn(fd, HSREG_FEESERIAL);
    fwtype = readfn(fd, HSREG_FEEFWTYPE);
    fwver  = readfn(fd, HSREG_FEEFWVER);

    serial |= (hwtype & 0xf) << 8;
    fwver  |= (fwtype & 0xf) << 8;
    hwtype = (hwtype >> 4) & 0xf;
    fwtype = (fwtype >> 4) & 0xf;

    if (hwtype == 14 && fwtype > 0 && fwtype <= 4) {
      printf("FEE type %s serial %d version %d at HSLB-%c\n",
             demotype[fwtype], serial, fwver, 'a'+fin);
    } else if (hwtype == 9 && fwtype >= 0 && fwtype <= 10) {
      printf("FEE type %s serial %d version %d at HSLB-%c\n",
             trgtype[fwtype], serial, fwver, 'a'+fin);
    } else {
      printf("FEE type %s serial %d firmware %d version %d at HSLB-%c\n",
             feetype[hwtype], serial, fwtype, fwver, 'a'+fin);
    }
    
  } else if (cmd) {

    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    writefn(fd, HSREG_CSR,     0x06); /* reset read ack */
    writefn(fd, HSREG_FEECONT, cmd);
    writefn(fd, HSREG_CSR,     0x0a); /* parameter write */
    
  } else if (cmd_cdc) {

    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    writefn(fd, HSREG_CSR,     0x06); /* reset read ack */
    writefn(fd, HSREG_CDCCONT, cmd_cdc);
    writefn(fd, HSREG_CSR,     0x0a); /* parameter write */
    
  } else if (op == HSDO_FEE8) {

    int adr = val & 0xff;
    
    if (rd == 0) {
      writefee8(fd, adr, val2 & 0xff);
    } else {
      val2 = readfee8(fd, adr);
      if (val2 < 0) {
	printf("no response from FEE at HSLB:%c\n", 'a'+fin);
      } else {
	printf("reg%02x = %02x\n", adr, readfn(fd, adr));
      } 
    }
    
  } else if (op == HSDO_FEE32) {
    
    int adr = val;
    
    if (rd == 0) {
      writefee32(fd, adr, val2);
    } else {
      if (readfee32(fd, adr, &val2) < 0) {
	printf("no response from FEE at HSLB:%c\n", 'a'+fin);
      } else {

        if (test_repeat) {
          int j;
          for (j=0; j<test_repeat; j++) {
            int val3;
            int ret = readfee32(fd, adr, &val3);
            if (ret < 0 || val2 != val3) {
              printf("%c: %02x val %08x broken at j=%d %08x ret %d\n",
                     'a' + i, adr, val2, j, val3, ret);
            }
          }
        }

	printf("reg%04x = %08x\n", adr, val2);
      }
    }
    
  } else if (op == HSDO_PDSTL) {

    if (val <= 0 || val > 48) {
      printf("pedestal channel %d out of range [1,48]\n", val);
      return -1;
    }
    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    writefn(fd, HSREG_PDSTLMIN + (val-1)*2+0, (val2>>0) & 0xff);
    writefn(fd, HSREG_PDSTLMIN + (val-1)*2+1, (val2>>8) & 0xff);
    writefn(fd, HSREG_CSR,     0x0a); /* parameter write */

  } else if (op == HSDO_PDSTLALL) {

    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    for (i=1; i<=48; i++) {
      writefn(fd, HSREG_PDSTLMIN + (i-1)*2+0, (val>>0) & 0xff);
      writefn(fd, HSREG_PDSTLMIN + (i-1)*2+1, (val>>8) & 0xff);
    }
    writefn(fd, HSREG_CSR,     0x0a); /* parameter write */

  } else if (op == HSDO_ADCTH || op == HSDO_TDCTH) {

    int adr = (op == HSDO_ADCTH) ? HSREG_ADCTH : HSREG_TDCTH;
    
    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    writefn(fd, adr + 0, (val>>0) & 0xff);
    writefn(fd, adr + 1, (val>>8) & 0xff);
    if (val != -1) {
      writefn(fd, HSREG_CSR,   0x0a); /* parameter write */
    } else {
      if (hswait(fin, fd) < 0) return -1;
      printf("reg%02x = %02x%02x\n", adr,
             readfn(fd, adr + 1) & 0xff,
             readfn(fd, adr + 0) & 0xff);
    }

  } else if (op == HSDO_WINDOW || op == HSDO_DELAY) {
    
    int adr = (op == HSDO_WINDOW) ? HSREG_WINDOW : HSREG_DELAY;
    
    writefn(fd, HSREG_CSR,     0x05); /* reset read fifo */
    writefn(fd, adr, val & 0xff);
    if (val != -1) {
      writefn(fd, HSREG_CSR,   0x0a); /* parameter write */
    } else {
      writefn(fd, HSREG_CSR,   0x07); /* parameter read */
      if (hswait(fin, fd) < 0) return -1;
      printf("reg%02x = %02x\n", adr, readfn(fd, adr));
    }
    
  } else {

    printf("undefined action %x\n", op);
    return -1;
    
  }
  return 0;
}
/* ---------------------------------------------------------------------- *\
   hsreg
\* ---------------------------------------------------------------------- */
int
hsreg(const char *name)
{
  static struct { char *name; int adrs; } regs[] = {
    { "-p",       HSDO_NOP },
    { "fee8",     HSDO_FEE8 },
    { "fee32",    HSDO_FEE32 },
    { "stream",   HSDO_STREAM },
    { "link",     HSDO_LINK },
    { "checkfee", HSDO_CHECKFEE },
    
    { "trghold",  HSDO_TRGOFF },
    { "trigger",  HSDO_TRGON },
    { "realtrg",  HSDO_TTTRG },
    { "simtrg" ,  HSDO_DUMTRG },
    { "simple",   HSDO_SUPMODE },
    { "verbose",  HSDO_RAWMODE },
    
    { "trgoff",   HSDO_TRGOFF },
    { "trgon",    HSDO_TRGON },
    { "tttrg",    HSDO_TTTRG },
    { "dumtrg" ,  HSDO_DUMTRG },
    { "supmode",  HSDO_SUPMODE },
    { "rawmode",  HSDO_RAWMODE },
    
    { "pdstl",    HSDO_PDSTL },
    { "pdstlall", HSDO_PDSTLALL },
    { "adcth",    HSDO_ADCTH },
    { "tdcth",    HSDO_TDCTH },

    { "delay",    HSDO_DELAY },
    { "window",   HSDO_WINDOW },
    
    { "cclk",     HSREG_CCLK },
    { "conf",     HSREG_CONF },
    { "cpldver",  HSREG_CPLDVER },
    { "serial1",  HSREG_SERIAL1 },
    { "serial2",  HSREG_SERIAL2 },
    { "fintype1", HSREG_FINTYPE1 },
    { "fintype2", HSREG_FINTYPE2 },
    { "d32",      HSREG_D32 },
    { "a32",      HSREG_A32 },
    { "id",       HSREGL_ID },
    { "ver",      HSREGL_VER },
    { "reset",    HSREGL_RESET },
    { "stat",     HSREGL_STAT },
  };
  int i;

  if (isdigit(name[0])) {
    return strtoul(name, 0, 16);
  }

  for (i = 0; i<sizeof(regs)/sizeof(regs[0]); i++) {
    if (strcmp(regs[i].name, name) == 0) {
      return regs[i].adrs;
    }
  }
    
  i = strtoul(name, 0, 16);
  return (i > 0 && i < 0x100) ? i : -1;
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int fd[4];
  int use[4];
  int i, j;
  int o_readonly = 1;
  int force_readonly = 0;

  ARGV0 = argv[0];

  use[0] = use[1] = use[2] = use[3] = 0;
  fd[0]  = fd[1]  = fd[2]  = fd[3]  = -1;

  /* parse options */
  while (argc >= 2 && argv[1][0] == '-' && argv[1][1] != 0) {
    for (i = 1; argv[1][i]; i++) {
      DEVICE = 0;
      if (! strchr("abcdrpt", argv[1][i])) {
	argc = -1;
	break;
      }
      if (argv[1][i] == 'p') {
        break;
      } else if (argv[1][i] == 'r') {
        force_readonly = 1;
      } else if (argv[1][i] == 't') {
        test_repeat = atoi(argv[2]);
        argv++, argc--;
        break;
      } else {
        j = argv[1][i] - 'a';
        if (use[j]) {
          fprintf(stderr, "FINESSE %c is specified twice\n", 'a'+j);
          exit(1);
        }
        use[j] = 1;
      }
    }
    argv++, argc--;
  }

  if (argc < 2 || (use[0]+use[1]+use[2]+use[3]) == 0) {
    fprintf(stderr, "%s version %d.%02d date %d\n"
	    , ARGV0, VERSION/100, VERSION%100, MOD_DATE);
    fprintf(stderr, "usage: %s -{a,b,c,d} <adr(hex)> [<val(hex)>]\n", ARGV0);
    exit(1);
  }

  /* parse arguments beforehand */
  for (i=1; i<argc; ) {
    unsigned int adr = hsreg(argv[i]);
    if (adr < 0) {
      fprintf(stderr, "%s: no such reigster \"%s\"\n", ARGV0, argv[i]);
      exit(1);
    }
    if (adr >= 0x100 || (i+1 < argc && strcmp(argv[i+1], "-") != 0)) {
      o_readonly = 0;
    }
    if (adr == HSDO_PDSTL || adr == HSDO_FEE8 || adr == HSDO_FEE32) {
      i += (force_readonly ? 2 : 3);
    } else if (adr >= 0x100 && adr < 0x200) {
      i += 1;
    } else {
      i += 2;
    }
  }

  /* open device */
  for (i=0; i<4; i++) {
    if (! use[i]) continue;
    if (fd[i] < 0) fd[i] = openfn(i, o_readonly, ARGV0);
    if (fd[i] < 0) exit(1);
  }

  /* HSLB operation, or register read or write */
  while (argc >= 2) {
    unsigned int adr = hsreg(argv[1]);
    int val;
    
    if (! force_readonly && argc >= 3 && strcmp(argv[2], "-") != 0) {
      o_readonly = 0;
      val = strtoul(argv[2], 0, adr >= 0x100 ? 0 : 16);
    } else {
      o_readonly = 1;
      val = -1;
    }
    
    for (i=0; i<4; i++) {
      if (! use[i]) continue;

      if (adr >= 0x100) { /* HSLB operation sequence */
	int val2 = 0;
	int rd = 0;
	if (adr == HSDO_PDSTL || adr == HSDO_FEE8 || adr == HSDO_FEE32) {
	  if (! force_readonly && argc >= 4 && strcmp(argv[3], "-") != 0) {
	    val2 = strtoul(argv[3], 0, 0);
	  } else {
	    val2 = -1;
	    rd = 1;
	  }
	}
	if (adr == HSDO_STREAM) {
	  writestream(fd[i], argv[2]);
	} else {
	  hsdo(i, fd[i], adr, val, val2, rd);
	}
	
      } else if (o_readonly) { /* register read */
	if (adr >= 0x80) {
	  val = readfn32(fd[i], adr);
          if (test_repeat) {
            int j;
            for (j=0; j<test_repeat; j++) {
              int val2 = readfn32(fd[i], adr);
              if (val != val2) {
                printf("%c: %02x val %08x broken at j=%d %08x\n",
                       'a' + i, adr, val, j, val2);
              }
            }
          }
	  printf("%c: %02x val %08x\n", 'a' + i, adr, val);
	} else {
	  val = readfn(fd[i], adr);
	  printf("%c: %02x val %02x\n", 'a' + i, adr, val);
	}
	
      } else { /* register write */
	if (adr >= 0x80) {
	  writefn32(fd[i], adr, val);
	} else {
	  writefn(fd[i], adr, val);
	}
      }
    }
    if (adr == HSDO_PDSTL || adr == HSDO_FEE8 || adr == HSDO_FEE32) {
      if (force_readonly) {
        argc -= 2;
        argv += 2;
      } else {
        argc -= 3;
        argv += 3;
      }
    } else if (force_readonly || (adr >= 0x100 && adr < 0x200)) {
      argc -= 1;
      argv += 1;
    } else {
      argc -= 2;
      argv += 2;
    }
  }

  close(fd);
}
