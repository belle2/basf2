/* ---------------------------------------------------------------------- *\

   bootrx.c - boot TT-RX bitmap file

   Mikihiko Nakao, KEK IPNS

   2005021500  ttrxlib version
   2005042400  unified ttxxlib version
   2005100300  sensible return value
   2006051800  retry option
   
\* ---------------------------------------------------------------------- */

#define VERSION 2006051800

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "ttrx.h"

#define TTXX_T         ttrx_t
#define READ_TTXX      read_ttrx
#define WRITE_TTXX     write_ttrx

/* ---------------------------------------------------------------------- *\
   write_fpga
\* ---------------------------------------------------------------------- */
void
write_ttrx_fpga(TTXX_T *ttxx, int reg_prgm, int ch)
{
  int mask, data;//i, 

  for (mask = 0x80; mask > 0; mask >>= 1) {
    data = 0;
    data = (ch & mask) ? 3 : 2; // bit-1 = CCLK
    WRITE_TTXX(ttxx, reg_prgm, data);
  }
}
/* ---------------------------------------------------------------------- *\
   dump_fpga
\* ---------------------------------------------------------------------- */
void
dump_ttrx_fpga(int prgm, char *str)
{
  printf("PRGM register=%01x M012=%d INIT=%d DONE=%d%s%s\n",
	 prgm&15, (prgm&2)>>1, (prgm&4)>>2, (prgm&8)>>3,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
int
boot_ttrx_fpga(TTXX_T *ttxx, int reg_prgm, const char *file, 
	       int verbose, int forced)
{
  int i, ch, prgm;
  int count = 1, length = 0;
  FILE *fp;

  /* -- initial condition -- */
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose) dump_ttrx_fpga(prgm, 0);

  /* -- download mode (M012<=1) -- */
  WRITE_TTXX(ttxx, reg_prgm, 0x06);
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose || ! (prgm & 2)) dump_ttrx_fpga(prgm, "<== 6 (M012<=1)");
  if (! (prgm & 2)) {
    printf("cannot set FPGA to the download mode (M012=0?).\n");
    if (! forced) return -1;
  }
  
  /* -- programming mode (PRGM<=1) -- */
  WRITE_TTXX(ttxx, reg_prgm, 0x08);
  usleep(1000); /* VMIVME is too fast: need to sleep a bit
                   if the FPGA was previously programmed */
  prgm = READ_TTXX(ttxx, reg_prgm);
  if (verbose || (prgm & 8)) dump_ttrx_fpga(prgm, "<== 8 (PRGM<=1)");
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
    return -1;
  }

  /* -- main part -- */
  do {
    write_ttrx_fpga(ttxx, reg_prgm, ch);
    count++;
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    write_ttrx_fpga(ttxx, reg_prgm, 0xff);
  }
  
  for (i=0; i<100; i++) write_ttrx_fpga(ttxx, reg_prgm, 0xff);
  fclose(fp);

  prgm = READ_TTXX(ttxx, reg_prgm) & 0xf;
  if (verbose & ! (prgm & 8)) dump_ttrx_fpga(prgm, "");
  if (prgm & 8) {
    printf("#%d done.\n", ttxx->id);
    return 0;
  } else {
    printf("#%d failed.\n", ttxx->id);
    return -1;
  }
}
