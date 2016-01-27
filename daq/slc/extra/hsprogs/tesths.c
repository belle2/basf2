/* ------------------------------------------------------------------ *\
 *
 *  tesths.c
 *
 *  save only headers as a text
 *
 *  Mikihiko Nakao (KEK)
 *  
 *  0.01  20130906  started from readhslb.c
 *  0.02  20140503  alarm
 *  0.03  20140512  renamed from testhead-hslb.c to tesths.c
 *  0.04  20140718  check HSLB version and belle2link status
 *  0.05  20150309  ftclk check
 *  0.06  20150319  b2link reset trial
 *  0.07  20150330  0 event read mode, PLL auto recovery
 *  0.08  20150330  -t -0 is default, no -f needed for stdout
 *  0.09  20150331  bad state can be recovered by gtp reset
 *  0.10  20150331  pllreset fix
 *  0.11  20150402  ignore bit-29 PLL lost
 *  0.12  20150409  first b2l_reset should be issued anyway
 *  0.13  20150410  statepr=1 can be cleared by reading fee32 0
 *  0.14  20150415  pll check fix
 *  0.15  20150416  use libhslb instead of "/dev/copper/fngeneric"
 *  0.16  20150417  b2l_reset rearranged
 *  0.17  20150527  libhslb fix
 *  0.18  20150604  clock check code
 *  0.19  20150605  no retry upon bad statepr, not to exit at first hslb
 *  0.20  20150618  restore retry upon bad statepr
 *  0.21  20150630  do not die at first event
 *
\* ------------------------------------------------------------------ */

#define VERSION 0.21

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#include <string.h>
#include <time.h>   // for getdate and gettime
#include <unistd.h> // for getdate and gettime

#ifdef STDINT_H_IS_MISSING
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#include "copper.h"
#include "libhslb.h"
#include "hsreg.h"

const char *PROGRAM = "tesths";
int SIGNALED = 0;
int SIGEXIT  = 1;
int ALARMED  = 0;
int USEALARM = 0;

int flag_head  = 0;
int flag_head3 = 1;

/* ------------------------------------------------------------------ *\
   sighandler
\* ------------------------------------------------------------------ */
void
sighandler(int signo)
{
  SIGNALED++;
  if (SIGEXIT || signo == SIGINT) {
    printf("\n");
    exit(0);
  }
}
/* ------------------------------------------------------------------ *\
   alarmhandler
\* ------------------------------------------------------------------ */
void
alarmhandler(int signo)
{
  ALARMED++;
}
/* ------------------------------------------------------------------ *\
   open output file
\* ------------------------------------------------------------------ */
FILE *
openfile(char *filename_arg, int force_overwrite)
{
  FILE *fp;
  time_t now = time(0);
  char filename[256];
  
  /* file name and open */
  if (filename_arg) {
    strcpy(filename, filename_arg);
  } else {
    strftime(filename, sizeof(filename), "%Y%m%d.%H%M.dat", localtime(&now));
    printf("File name: %s\n", filename);
  }

  if (fp = fopen(filename, "r")) {
    if (force_overwrite) {
      printf("%s: overwriting existing file %s\n", PROGRAM, filename);
    } else {
      printf("%s: file %s already exists\n", PROGRAM, filename);
      exit(0);
    }
  }
  if (! (fp = fopen(filename, "w"))) {
    printf("%s: can't open file %s to write, %s\n",
	   PROGRAM, filename, strerror(errno));
    exit(0);
  }

  return fp;
}
/* ------------------------------------------------------------------ *\
   open finesse
\* ------------------------------------------------------------------ */
int
opencopper(int use_slot)
{
  int ret;
  int cprfd;
  int finfd[4];
  int clef_1 = 1; /* for COPPER ioctl (length fifo set?) */
  int clef_180 = 0x180; /* for COPPER ioctl (length fifo set?) */
  
  /* COPPER device ioctl */
  if ((cprfd = open("/dev/copper/copper", O_RDONLY)) < 0) {
    printf("%s: can't open /dev/copper/copper, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }
  if (ioctl(cprfd, CPRIOSET_LEF_WA_FF, &clef_1) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WB_FF, &clef_1) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WC_FF, &clef_1) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WD_FF, &clef_1) < 0) {
    printf("%s: can't ioctl(LEF_Wx_FF) /dev/copper/copper, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }
  if (ioctl(cprfd, CPRIOSET_LEF_WA_AF, &clef_180) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WB_AF, &clef_180) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WC_AF, &clef_180) < 0 ||
      ioctl(cprfd, CPRIOSET_LEF_WD_AF, &clef_180) < 0) {
    printf("%s: can't ioctl(LEF_Wx_FF) /dev/copper/copper, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }
  /* choose FINESSE slot to use */
  ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));
  if (ret < 0) {
    printf("%s: can't ioctl(FINESSE_STA) /dev/copper/copper, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }
  
#if 0 /* no need to check again */
  /* open FINESSE just to check availability */
  finfd[0] = finfd[1] = finfd[2] = finfd[3] = 0;
  if (use_slot & 1) finfd[0] = openfn(0, O_RDWR, "tesths");
  if (use_slot & 2) finfd[1] = openfn(1, O_RDWR, "tesths");
  if (use_slot & 4) finfd[2] = openfn(2, O_RDWR, "tesths");
  if (use_slot & 8) finfd[3] = openfn(3, O_RDWR, "tesths");

  if (finfd[0] < 0 || finfd[1] < 0 || finfd[2] < 0 || finfd[3] < 0) {
    printf("%s: missing HSLB on COPPER:  %d:%d:%d:%d\n",
	   PROGRAM, finfd[0], finfd[1], finfd[2], finfd[3]);
    exit(1);
  }
#endif

  return cprfd;
}
/* ------------------------------------------------------------------ *\
   readcopper
\* ------------------------------------------------------------------ */
int
readcopper(int iev, int cprfd, FILE *fp)
{
  int ret;
  int hslb;
  uint32_t header[13];
  int headlen = 0;
  int datasiz = 0;
  int datalen = 0;
  static uint32_t data[256*1024 + 3]; /* max: 64K word / FINESSE */
  static int return_in_10 = 0;
  static int prevsiz = -1;
  static int nsizchg = 0;

  /* EINTR-safe read of header */
  while (headlen < 13 && ! SIGNALED) {
    if (USEALARM) {
      ALARMED = 0;
      alarm(5);
    }
    ret = read(cprfd, (char *)header+headlen, sizeof(header)-headlen);
    if (ret == 0 || (ret < 0 && (errno != EINTR && errno != EAGAIN))) break;
    if (ret < 0 && errno == EINTR && ALARMED) {
      printf("alarm!\n");
      ioctl(cprfd, CPRIO_FORCE_DMA, 0);
      ALARMED = 0;
    }
    if (ret < 0) continue;
    headlen += ret;
  }
  if (SIGNALED) {
    return -1;
  }
  if (ret < 0) {
    printf("%s: can't read copper, %s\n", PROGRAM, strerror(errno));
    exit(1);
  }
  
  /* check header */
  if (header[0] != 0x7fff0008) {
    printf("bad header[0]=%08x (!= 0x7fff0008)\n", header[0]);
    exit(1);
  }
  if (header[7] != 0xfffffafa) {
    printf("bad header[0]=%08x (!= 0xffaa0000)\n", header[7]);
    exit(1);
  }
  datasiz = header[8] - 7;
  if (datasiz != header[9] + header[10] + header[11] + header[12]) {
    printf("bad header[1]=%08x (!=%x+%x+%x+%x+7)\n", header[8],
	   header[9], header[10], header[11], header[12]);
    exit(1);
  }
  if (datasiz <= 0 || datasiz >= 64*1024) {
    printf("bad datasize=%d\n", datasiz);
    exit(1);
  }
  datasiz = (datasiz + 3) * 4;
  
  /* EINTR-safe read of data + footer */
  while (datalen < datasiz && ! SIGNALED) {
    ret = read(cprfd, (char *)data+datalen, datasiz-datalen);
    if (ret == 0 || (ret < 0 && (errno != EINTR && errno != EAGAIN))) break;
    if (ret < 0) continue;
    datalen += ret;
  }
  if (SIGNALED) return -1;
  if (ret < 0) {
    printf("%s: can't read copper, %s\n", PROGRAM, strerror(errno));
    exit(1);
  }
  
  /* check footer */
  if (data[datasiz/4-3] != 0xfffff5f5) {
    printf("bad footer=%08x (!= 0xfffff5f5)\n", datasiz);
    exit(1);
  }
  if (data[datasiz/4-1] != 0x7fff0009) {
    printf("bad footer=%08x (!= 0x7fff0009)\n", datasiz);
    exit(1);
  }
  
  /* write into file */
  /* only this part is modified from readhslb.c
     fwrite(data, datasiz-3*4, 1, fp); */
  if (flag_head || flag_head3) {
    static int prev = -1;
    int printed = 0;
    static int nline = 0;
    char buf[256];
    static int iprev = 0;
    static int jprev = 0;
    static char prevbuf[20][256];
    if (prevsiz != -1 && prevsiz != datasiz && nsizchg < 10) {
      printf("size change %d -> %d\n", prevsiz, datasiz);
      nsizchg++;
    }
    prevsiz = datasiz;
    sprintf(buf, "%03x:%08x %08x %08x %08x %08x %08x %08x %08x %08x-%08x %08x %08x\n",
	    datasiz/4, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
	    data[datasiz/4-6], data[datasiz/4-5], data[datasiz/4-4]);

    if (return_in_10 == 0 && (nline % 10000 == 0 || nline < 10)) {
      fprintf(fp, "%s", buf);
      printed = 1;
    }
    ++nline;
    if (return_in_10) {
      fprintf(fp, "+%d %s", 10-return_in_10, buf);
      return_in_10--;
      if (! return_in_10) return -1;
    } else if (prev >= 0 && data[2] != prev + 1 && flag_head3) {
      int ibuf;
      int jbuf;
      if (! printed) {
        fprintf(fp, "%s", buf);
      }
      jbuf = (iprev == jprev) ? 0 : (jprev + 20 - iprev) % 20;
      fprintf(fp, "bad event found.\n");
      for (ibuf = iprev; ibuf != jprev; ibuf = (ibuf + 1) % 20) {
	fprintf(fp, "-%d %s", jbuf--, prevbuf[ibuf]);
      }
      fprintf(fp, "+0 %s", buf);
      
      return_in_10 = 9;
    } else {
      strcpy(prevbuf[jprev], buf);
      jprev = (jprev + 1) % 20;
      if (iprev == jprev) iprev = (iprev + 1) % 20;
      prev = data[2];
    }
  } else {
    fprintf(fp, "%08x %08x\n", data[1], data[2]);
  }

  return 0;
}
/* ------------------------------------------------------------------ *\
   reset_b2l
\* ------------------------------------------------------------------ */
int
reset_b2l(int fd, int *csrp)
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
/* ------------------------------------------------------------------ *\
   check_hslb
\* ------------------------------------------------------------------ */

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


int
check_hslb(int use_slot)
{
  int i;
  int tmpval = 0;
  int *valp = &tmpval;
  int warning = 0;
  int errcode = 0;

  *valp = 0;
  
  for (i=0; i<4; i++) {
    int o_readonly = 1;
    int fd;
    int id;
    int ver;
    int csr;
    int j;
    
    if ((use_slot & (1 << i)) == 0) continue;

    fd = openfn(i, o_readonly, "tesths");
    if (fd < 0) {
      errcode = errno;
      continue;
    }
    
    id  = readfn32(fd, HSREGL_ID);   /* 0x80 */
    ver = readfn32(fd, HSREGL_VER);  /* 0x81 */
    csr = readfn32(fd, HSREGL_STAT); /* 0x83 */

    if (id != 0x48534c42 && id != 0x48534c37) {
      printf("hslb-%c not found (id=0x%08x != 0x48534c42)\n", 'A'+i, id);
      *valp = id;
      errcode = EHSLB_NOTFOUND;
      continue;
    }
    if (id == 0x48534c42 && ver < 34){
      printf("hslb-%c too old firmware (ver=0.%02d < 0.34)\n", 'A'+i, ver);
      *valp = ver;
      errcode = EHSLB_TOOOLD;
      continue;
    }
    if (id == 0x48534c37 && ver < 6){
      printf("hslb7-%c too old firmware (ver=0.%02d < 0.06)\n", 'A'+i, ver);
      *valp = ver;
      errcode = EHSLB7_TOOOLD;
      continue;
    }
    if (csr & 0x20000000) {
      int j;
      int recvok = 0;
      int uptime0 = readfn32(fd, HSREGL_UPTIME);
      int uptime1;
      usleep(1000*1000);
      uptime1 = readfn32(fd, HSREGL_UPTIME);

      if (uptime0 == 0) {
        printf("hslb-%c clock is missing\n", 'A'+i);
        errcode = EHSLB_CLOCKNONE;
        continue;
      } else if (uptime0 == uptime1) {
        printf("hslb-%c clock is lost or too slow\n", 'A'+i);
        errcode = EHSLB_CLOCKLOST;
        continue;
      } else if (uptime1 > uptime0 + 1) {
        printf("hslb-%c clock is too fast\n", 'A'+i);
        errcode = EHSLB_CLOCKFAST;
        continue;
      }

      for (j = 0; j < 100; j++) {
        int recv = readfn32(fd, HSREGL_RXDATA) & 0xffff;
        if (recv == 0x00bc) recvok++;
      }
      if (recvok < 80) {
        printf("hslb-%c PLL lost and can't receive data (csr=%08x)\n",
             'A'+i, csr);
        errcode = EHSLB_PLLLOST;
        continue;
      }
      printf("hslb-%c PLL lost (csr=%08x) is probably harmless and ignored\n",
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
          printf("hslb-%c Belle2link is down, csr=%08x\n", 'A'+i, csr);
          errcode = EHSLB_B2LDOWN;
          continue;
        } else if (csr & 0x20) {
          printf("hslb-%c bad clock detected, csr=%08x\n", 'A'+i, csr);
          errcode = EHSLB_CLOCKBAD;
          continue;
        } else if (csr & 0x80) {
          printf("hslb-%c PLL2 lock lost, csr=%08x\n", 'A'+i, csr);
          errcode = EHSLB_PLL2LOST;
          continue;
        } else if (csr & 0x40) {
          printf("hslb-%c GTP PLL lock lost, csr=%08x\n", 'A'+i, csr);
          errcode = EHSLB_GTPPLL;
          continue;
        } else if (csr & 0x20000000) {
          printf("hslb-%c FF clock is stopped, csr=%08x\n", 'A'+i, csr);
          errcode = EHSLB_FFCLOCK;
          continue;
        }
      } else {
        *valp = count;
        if (oldcsr & 1) {
          warning = WHSLB_B2LDOWN;
          printf("hslb-%c Belle2link recovered, csr=%08x (retry %d)\n",
                 'A'+i, csr, count);
        } else if (oldcsr & 0x20) {
          warning = WHSLB_B2LDOWN;
          printf("hslb-%c bad clock recovered, csr=%08x (retry %d)\n",
                 'A'+i, csr, count);
        } else if (oldcsr & 0x80) {
          warning = WHSLB_PLL2LOST;
          printf("hslb-%c PLL2 lock recovered, csr=%08x (retry %d)\n",
                 'A'+i, csr, count);
        } else if (oldcsr & 0x40) {
          warning = WHSLB_GTPPLL;
          printf("hslb-%c GTP PLL lock recovered, csr=%08x (retry %d)\n",
                 'A'+i, csr, count);
        } else if (oldcsr & 0x20000000) {
          warning = WHSLB_FFCLOCK;
          printf("hslb-%c FF clock is recovered, csr=%08x (retry %d)\n",
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
      printf("hslb-%c hslb in bad state (csr=%08x)\n", 'A'+i, csr);
      *valp = csr;
      errcode = EHSLB_BADSTATE;
      continue;
    }
    if (csr & 2) {
      printf("hslb-%c is disabled, ttrx reg 130 bit%d=0\n",
	     'A'+i, i);
      errcode = EHSLB_DISABLED;
      continue;
    }
    /*
    if (csr & 0x80000000) {
      printf("hslb-%c belle2link was down and recovered (csr=%08x), %s\n",
	     'A'+i, csr, "need runreset");
    }
    */
    printf("hslb-%c 0.%02d %08x\n", 'A'+i, ver, csr);
    
    close(fd);
  }
  return errcode;
}
/* ------------------------------------------------------------------ *\
   main
\* ------------------------------------------------------------------ */
int
main(int argc, char **argv)
{
  /* output file */
  FILE   *fp;
  char   filename[256];
  int nevent = 0;
  int force_overwrite = 0;

  /* copper and finesse devices */
  int cprfd;
  int hslb_datalen[4];
  int use_slot = 0; /* bit mask */

  /* event and buffer */
  int i;

  /* generic variables */
  int ret;
  
  signal(SIGALRM, alarmhandler);
  signal(SIGINT, sighandler);
  PROGRAM = argv[0];
  
  /* option analysis: FINESSE slot selection, etc */
  while (argc > 1 && argv[1][0] == '-') {
    int pos;

    if (isdigit(argv[1][1])) {
      nevent = atoi(&argv[1][1]);
    } else {
      for (pos=1; argv[1][pos] != 0; pos++) {
	int opt =  argv[1][pos];
	if (opt >= 'a' && opt <= 'd') {
	  use_slot |= (1 << (opt-'a'));
	} else if (opt == 't') {
	  flag_head3 = 0;
	} else if (opt == 'T') {
	  flag_head3 = 0;
	  flag_head = 1;
	} else if (opt == 'A') {
	  USEALARM = 1;
	} else if (opt == 'f') {
	  force_overwrite = 1;
	} else {
	  printf("%s: unknown option %c\n", PROGRAM, opt);
	  exit(1);
	}
      }
    }
    argc--, argv++;
  }

  /* usage if no proper slot selection */
  if (! use_slot) {
    printf("%s version %4.2f\n", PROGRAM, VERSION);
    printf("usage: %s -[<num>] -[abcd] [-f] [<file>]\n", PROGRAM);
    printf("<num> for number of events (default = 1)\n");
    printf("-f to force overwriting existing file\n");
    printf("default file name is in %%Y%%m%%d.%%H%%M.dat format\n");
    exit(1);
  } else {
    int slot;
    printf("%s: version %4.2f slot", PROGRAM, VERSION);
    for (slot=0; slot<4; slot++) {
      if (use_slot & (1<<slot)) printf(" %c", 'A'+slot);
    }
    printf("\n");
  }

  /* check hslb */
  if (check_hslb(use_slot) < 0) {
    return 0;
  }

  /* open file (no return if error) */
  if (argc == 1) {
    fp = stdout;
  } else {
    fp = openfile(argc > 1 ? argv[1] : 0, force_overwrite);
  }

  /* open copper (no return if error) */
  cprfd = opencopper(use_slot);

  /* main loop */
  SIGEXIT = 0;
  for (i = 0; i < nevent; i++) {
    if (readcopper(i, cprfd, fp) < 0) {
      break;
    }
  }

  if (nevent > 0) printf("%d event read.\n", i);
  fclose(fp);
  return 0;
}

