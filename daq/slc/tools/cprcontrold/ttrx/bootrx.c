/* ---------------------------------------------------------------------- *\

   bootrx.c - boot TT-RX bitmap file

   Mikihiko Nakao, KEK IPNS

   2005021500  ttrxlib version
   2005042400  unified ttxxlib version
   2005100300  sensible return value
   
\* ---------------------------------------------------------------------- */

#define VERSION 2005100300

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ttrx.h"

#define TTXX_T         ttrx_t
#define READ_TTXX      read_ttrx
#define WRITE_TTXX     write_ttrx

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
static void
write_fpga(TTXX_T *ttxx, int reg_prgm, int ch)
{
  int i, mask, data;

  for (mask = 0x80; mask > 0; mask >>= 1) {
    data = 0;
    data = (ch & mask) ? 3 : 2; // bit-1 = CCLK
    WRITE_TTXX(ttxx, reg_prgm, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_fpga
\* ---------------------------------------------------------------------- */
static void
dump_fpga(int prgm, char *str)
{
  printf("PRGM register=%01x M012=%d INIT=%d DONE=%d%s%s\n",
	 prgm&15, (prgm&2)>>1, (prgm&4)>>2, (prgm&8)>>3,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
static int
boot_fpga(TTXX_T *ttxx, int reg_prgm, char *file, int verbose, int forced)
{
  int i, ch, prgm;
  int count = 1, length = 0;
  FILE *fp;

  /* -- initial condition -- */
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose) dump_fpga(prgm, 0);

  /* -- download mode (M012<=1) -- */
  WRITE_TTXX(ttxx, reg_prgm, 0x06);
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose || ! (prgm & 2)) dump_fpga(prgm, "<== 6 (M012<=1)");
  if (! (prgm & 2)) {
    printf("cannot set FPGA to the download mode (M012=0?).\n");
    if (! forced) return -1;
  }
  
  /* -- programming mode (PRGM<=1) -- */
  WRITE_TTXX(ttxx, reg_prgm, 0x08);
  usleep(1000); /* VMIVME is too fast: need to sleep a bit
                   if the FPGA was previously programmed */
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose || (prgm & 8)) dump_fpga(prgm, "<== 8 (PRGM<=1)");
  if (prgm & 8) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }
  
  /* -- open the file -- */
  if (! (fp = fopen(file, "r"))) {
    printf("cannot open file: %s\n", file);
    return -1;
  }
  
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
    fclose(fp);
    return -1;
  }

  /* -- main part -- */
  do {
    write_fpga(ttxx, reg_prgm, ch);
    count++;
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_fpga(ttxx, reg_prgm, 0xff);
  }
  
  for (i=0; i<100; i++) write_fpga(ttxx, reg_prgm, 0xff);
  fclose(fp);

  prgm = READ_TTXX(ttxx, reg_prgm) & 0xf;
  if (verbose & ! (prgm & 8)) dump_fpga(prgm, "");
  if (prgm & 8) {
    printf("#%d done.\n", ttxx->id);
    return 0;
  } else {
    printf("#%d failed.\n", ttxx->id);
    return -1;
  }
}
/* ---------------------------------------------------------------------- *\
   main
\* ---------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
  int verbose  = 0;
  int forced   = 0;
  int oflag    = TTRX_RDWR;
  int reg_prgm = TTRX_PRGM;
  int i, nttxx = 0;
  int id[16];
  TTXX_T *ttxx;
  int ret = 0;

  while (argc > 3 || (argc > 1 && argv[1][0] == '-')) {
    if (strcmp(argv[1], "-m") == 0) {
      oflag |= TTRX_MMAPMODE;
    } else if (strcmp(argv[1], "-v") == 0) {
      verbose = ! verbose;
    } else if (strcmp(argv[1], "-f") == 0) {
      forced = ! forced;
    } else if (argv[1][0] == '-' && isdigit(argv[1][1])) {
      if (nttxx == 16) {
	argc = 0;
	break; /* too many id's specified */
      }
      id[nttxx++] = argv[1][1] - '0';
    } else {
      argc = 0; /* to show help */
      break;
    }
    argc--, argv++;
  }
  if (argc < 2) {
    printf("bootrx version %d\n", VERSION);
    printf("usage: bootrx [-m] <filename>\n");
    printf("options:\n");
    printf(" -m      mmap mode\n");
    printf(" -v      verbose mode\n");
    printf(" -f      forced (ignore errors)\n");
    printf(" -[<n>]  n-th TT-RX\n");
    return 1;
  }
  if (nttxx == 0) {
    id[nttxx++] = 0;
  }

  for (i=0; i<nttxx; i++) {
    if (! (ttxx = open_ttrx(id[i], oflag))) {
      perror("open");
      exit(1);
    }
    ret += boot_fpga(ttxx, reg_prgm, argv[1], verbose, forced);
  }
  return -ret;
}
