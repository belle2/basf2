/* ------------------------------------------------------------------ * \
 *
 *  readhs.c
 *
 *  full rewrite of record-nakao.c
 *
 *  Mikihiko Nakao (KEK)
 *  
 *  0.01  20130529  first version
 *  0.02  20130906  renamed to readhslb.c and added more functions
 *  0.03  20131008  interactive mode
 *  0.04  20131118  SAVEHDRS (-h option)
 *  0.05  20140427  EAGAIN handling
 *  0.06  20140513  renamed from readhslb.c
 *  0.07  20150416  use libhslb instead of "/dev/copper/fngeneric"
 *
\* ------------------------------------------------------------------ */

#define VERSION 0.07

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#include "copper.h"
#include "libhslb.h"
#include <string.h>
#include <time.h>   // for getdate and gettime
#include <unistd.h> // for getdate and gettime

#ifdef STDINT_H_IS_MISSING
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

const char *PROGRAM = "readhs";
int SIGNALED = 0;
int SIGEXIT  = 0;
int ALARMED  = 0;
int USEALARM = 0;
int VERBOSE  = 0;
int SAVEHDRS = 0;

/* ------------------------------------------------------------------ *\
   sighandler
\* ------------------------------------------------------------------ */
void
sighandler(int signo)
{
  SIGNALED++;
  if (SIGEXIT) {
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
openfile(const char *filename_arg, int force_overwrite, int force_exit)
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
    fclose(fp);
    if (force_overwrite) {
      printf("%s: overwriting existing file %s\n", PROGRAM, filename);
    } else {
      printf("%s: file %s already exists\n", PROGRAM, filename);
      if (! force_exit) return 0;
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
opencopper(int use_slot, int clef_1)
{
  int ret;
  int cprfd;
  int finfd[4];
  
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

  /* choose FINESSE slot to use */
  ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));
  if (ret < 0) {
    printf("%s: can't ioctl(FINESSE_STA) /dev/copper/copper, %s\n",
	   PROGRAM, strerror(errno));
    exit(0);
  }
  
  /* open FINESSE just to check availability */
  finfd[0] = finfd[1] = finfd[2] = finfd[3] = 0;
  if (use_slot & 1) finfd[0] = openfn(0, O_RDWR, "readhs");
  if (use_slot & 2) finfd[1] = openfn(1, O_RDWR, "readhs");
  if (use_slot & 4) finfd[2] = openfn(2, O_RDWR, "readhs");
  if (use_slot & 8) finfd[3] = openfn(3, O_RDWR, "readhs");

  if (finfd[0] < 0 || finfd[1] < 0 || finfd[2] < 0 || finfd[3] < 0) {
    printf("%s: missing HSLB on COPPER:  %d:%d:%d:%d\n",
	   PROGRAM, finfd[0], finfd[1], finfd[2], finfd[3]);
    exit(1);
  }

  return cprfd;
}
/* ------------------------------------------------------------------ *\
   readcopper
\* ------------------------------------------------------------------ */
FILE *
cmdline(int cprfd, FILE *fp, const char *filename, int force_overwrite)
{
  char filenamebuf[512];
  static int num = 0;
  
  while (! fp) {
    printf("enter filename-->");
    SIGEXIT = 1;
    if (fgets(filenamebuf, sizeof(filenamebuf), stdin) == 0) {
      exit(1);
    }
    if (strcmp(filenamebuf, "stat") == 0) {
      continue;
    } else if (strncmp(filenamebuf, "reg ", 4) == 0) {
      continue;
    }
    
    SIGEXIT = 0;
    if (filenamebuf[0]) {
      filename = filenamebuf;
    } else if (filename) {
      sprintf(filenamebuf, "%s-%d", filename, num);
      filename = filenamebuf;
    }
    fp = openfile(filename, force_overwrite, 0);
    /* closefp = 1; */
    if (fp) break;
  }
  
  num++;
  return fp;
}
/* ------------------------------------------------------------------ *\
   readcopper
\* ------------------------------------------------------------------ */
int
readcopper(int cprfd, FILE *fp, const char *filename, int force_overwrite)
{
  int ret;
  int hslb;
  uint32_t header[13];
  int headlen = 0;
  int datasiz = 0;
  int datalen = 0;
  static uint32_t data[64*1024 + 3]; /* max: 64K word / FINESSE */
  int closefp = 0;
  FILE *newfp;

  newfp = cmdline(cprfd, fp, filename, force_overwrite);
  if (newfp != fp) {
    closefp = 1;
    fp = newfp;
  }

  
  /* EINTR-safe read of header */
  while (headlen < 13 && ! SIGNALED) {
    if (USEALARM) {
      ALARMED = 0;
      alarm(5);
    }
    
    ret = read(cprfd, (char *)header+headlen, sizeof(header)-headlen);
    if (ret == 0 || (ret < 0 && errno != EINTR && errno != EAGAIN)) break;
    if (ret < 0 && errno == EINTR && ALARMED) {
      printf("alarm!\n");
      ioctl(cprfd, CPRIO_FORCE_DMA, 0);
      ALARMED = 0;
    }
    if (ret < 0) continue;
    headlen += ret;
  }
  if (SIGNALED) return -1;
  if (ret < 0) {
    printf("%s: can't read copper, %s\n", PROGRAM, strerror(errno));
    exit(1);
  }
  
  /* check header */
  if (header[0] != 0x7fff0008) {
    int i;
    printf("bad header[0]=%08x (!= 0x7fff0008)\n", header[0]);
    for (i = 0; i<16; i++) {
      printf("header %2d = %08x\n", i, header[i]);
    }
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
    if (ret == 0 || (ret < 0 && errno != EINTR && errno != EAGAIN)) break;
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
  if (SAVEHDRS) {
    fwrite(header, sizeof(header), 1, fp);
    fwrite(data, datasiz, 1, fp);
  } else {
    fwrite(data, datasiz-3*4, 1, fp);
  }

  if (VERBOSE) {
    printf("data read out\n");
  }

  if (closefp) fclose(fp);
  
  return 0;
}
/* ------------------------------------------------------------------ *\
   main
\* ------------------------------------------------------------------ */
int
main(int argc, char **argv)
{
  /* output file */
  FILE   *fp;
  int nevent = 1;
  int force_overwrite = 0;
  int interactive = 0;
  const char *filename;
  int clef_1 = 1; /* for COPPER ioctl (length fifo threshold to start DMA) */

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

    if (isdigit(argv[1][1]) && argv[1][1] != '0') {
      nevent = atoi(&argv[1][1]);
    } else {
      for (pos=1; argv[1][pos] != 0; pos++) {
	int opt =  argv[1][pos];
	if (opt >= 'a' && opt <= 'd') {
	  use_slot |= (1 << (opt-'a'));
	} else if (opt == 'f') {
	  force_overwrite = 1;
	} else if (opt == 'i') {
	  interactive = 1;
	} else if (opt == 'v') {
	  VERBOSE = 1;
	} else if (opt == 'z') {
	  clef_1 = 0;
	} else if (opt == 'w') {
	  clef_1 = 128;
	} else if (opt == 'h') {
	  SAVEHDRS = 1;
	} else if (opt == 'A') {
	  USEALARM = 1;
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
    printf("usage: %s -[<num>] -[abcd] [-i] [-f] [<file>]\n", PROGRAM);
    printf("<num> for number of events (default = 1)\n");
    printf("-i to interructively open file per event\n");
    printf("-f to force overwriting existing file\n");
    printf("default file name is in %%Y%%m%%d.%%H%%M.dat format\n");
    exit(1);
  } else {
    int slot;
    printf("%s: slot", PROGRAM);
    for (slot=0; slot<4; slot++) {
      if (use_slot & (1<<slot)) printf(" %c", 'A'+slot);
    }
    printf("\n");
  }

  /* open file (no return if error) */
  filename = argc > 1 ? argv[1] : 0;
  if (interactive) {
    fp = 0;
  } else {
    fp = openfile(filename, force_overwrite, 1);
  }

  /* open copper (no return if error) */
  cprfd = opencopper(use_slot, clef_1);

  /* main loop */
  for (i = 0; i < nevent; i++) {
    if (readcopper(cprfd, fp, filename, force_overwrite) < 0) {
      break;
    }
  }

  printf("%d event read.\n", i);
  if (fp) fclose(fp);
  return 0;
}

