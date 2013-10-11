/* ---------------------------------------------------------------------- *\

   bootmgt.c - boot MGT FINESSE prototype bitmap file

   Mikihiko Nakao, KEK IPNS

   2009042100  retry option
   2009101600  parallel version based on confmgt6.c
   2009112500  selectMAP fix (had to change M012 to 7 at the end)
   2009112501  merge serial and parallel modes
   
\* ---------------------------------------------------------------------- */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <ctype.h>
#include <time.h>

#include "find_pci_path.h"
#include "cprfin_fngeneric.h"

#include "mgt.h"
#include "mgt_register.h"
#include "mgt_control.h"

Mgt_t* mgt_open(int ch, int mode) {
  char *DEVICE = 0;
  const int o_mode = (mode & 1) ? O_RDWR                 : O_RDONLY;
  const int m_mode = (mode & 1) ? (PROT_READ|PROT_WRITE) : PROT_READ;
  if ( mode & MGT_MMAP ) {
    unsigned long base = 0;
    int fd_tmp;
    struct pcichip {
      unsigned long word[4];
      unsigned long bar[4];
    } chip;

    const char *plx9054_path = get_copper_plx9054_path();
    if ( (fd_tmp = open(plx9054_path, O_RDONLY)) < 0 ) {
      fprintf(stderr, "DEBUG: cannot open %s: %s\n",
	      plx9054_path, strerror(errno));
      return 0;
    }
    if ( read(fd_tmp, &chip, sizeof(chip)) != sizeof(chip) ) {
      fprintf(stderr, "DEBUG: cannot read %s: %s\n",
	      plx9054_path, strerror(errno));
      close(fd_tmp);
      return 0;
    }
    close(fd_tmp);
    base = chip.bar[2];

    if ( ch < 0 || ch > 4 ) {
      int fd = open("/dev/mem", o_mode);
      if ( fd == -1 ) {
	fprintf(stderr, "DEBUG: cannot open /dev/mem: %s\n",
	        strerror(errno));
	return 0;
      }
      volatile unsigned long* xmemp = 
	mmap(0, 0x1000000, m_mode, MAP_SHARED, fd, base);
      if ( xmemp == (unsigned long *)0xffffffff ) {
	fprintf(stderr, "DEBUG: cannot mmap /dev/mem: %s\n",
		strerror(errno));
	return 0;
      }
      Mgt_t* mgt = (Mgt_t*)malloc(sizeof(Mgt_t));
      mgt->fd = fd;
      mgt->slot = ch;
      mgt->xmemp = xmemp;
      return mgt;
    }
  } else {
    switch (ch) {
    case  0:
    case 'a': DEVICE = "/dev/copper/fngeneric:a"; break;
    case  1:
    case 'b': DEVICE = "/dev/copper/fngeneric:b"; break;
    case  2:
    case 'c': DEVICE = "/dev/copper/fngeneric:c"; break;
    case  3:
    case 'd': DEVICE = "/dev/copper/fngeneric:d"; break;
    default: 
      errno = ENODEV; return 0;
    }
    int fd = open(DEVICE, o_mode);
    if ( fd < 0 ) {
      fprintf(stderr, "DEBUG: cannot open %s: %s\n",
	      DEVICE, strerror(errno));
      return 0;
    }
    Mgt_t* mgt = (Mgt_t*)malloc(sizeof(Mgt_t));
    mgt->fd = fd;
    mgt->slot = ch;
    mgt->xmemp = 0;
    return mgt;
  }
  return 0;
}

void mgt_close(Mgt_t* mgt) {
  if ( mgt != 0 ) {
    if ( mgt->xmemp != 0 ) {
      munmap((void*)mgt->xmemp, 0x1000000);
    }
    close(mgt->fd);
    free(mgt);
  }
}

int mgt_check_FEE(Mgt_t* mgt) {
  mgt_write(mgt, CONTROL, 0x05);
  mgt_write(mgt, CONTROL, 0x06);
  mgt_write(mgt, FEE_TYPE, 0x02);
  mgt_write(mgt, FEE_SERIAL, 0x02);
  mgt_write(mgt, FEE_HWV, 0x02);
  mgt_write(mgt, FEE_FWV, 0x02);
  mgt_write(mgt, CONTROL, 0x07);
  ///*
  int n = 0;
  for ( n = 1; n < 4; n++ ) {
    usleep(100000);
    printf("waited for %d00 ms\n", n);
    if ( n > 2 ) {
      printf( "No response from :  ");
      printf(" HSLB:%c ", ('a' + mgt->slot));
      printf("\n");
      return 1;
    }
    int val = mgt_read(mgt, STATUS);
    printf("value of status register  on HSLB:%c : %x \n",'a'+ mgt->slot ,val);
    if ( val == 0x11 ) break;
  }
  //*/
  int feetype = mgt_read(mgt, FEE_TYPE);
  switch (feetype) {
  case 0x01:
    printf("The type of FEE readout board which is linked to HSLB:%c is CDC\n",'a' + mgt->slot);
    break;
  case 0x02:
    printf("The type of FEE readout board which is linked to HSLB:%c is ECL\n",'a' + mgt->slot);
    break;
  default:
    printf("error type number : %x\n", feetype);
  }
  int result = mgt_read(mgt, FEE_SERIAL);
  printf("The serial# of FEE readout board which is linked to HSLB:%c is %x\n",
         'a' + mgt->slot, result);
  result = mgt_read(mgt, FEE_FWV);
  printf("The version of the FEE readout board hardware which is linked to HSLB:%c is %x\n",
         'a'+mgt->slot, result);
  result = mgt_read(mgt, FEE_HWV);
  printf("The version of the FEE readout board hardware which is linked to HSLB:%c is %x\n",
         'a'+mgt->slot, result);
  result = mgt_read(mgt, FEE_HWV);
  printf("The version of the FEE readout board firmware which is linked to HSLB:%c is %x\n",
         'a' + mgt->slot, result);
  printf("written %.2x to address %x on HSLB:%c\n",
         0x06, CONTROL,'a'+ mgt->slot);
  mgt_write(mgt, CONTROL, 0x06);
  return 0;
}

int mgt_execute(Mgt_t* mgt, int ctl_code) {
  int result = 1;
  result |= mgt_write(mgt, CONTROL, 0x05);
  result |= mgt_write(mgt, CONTROL, 0x06);
  result |= mgt_write(mgt, FEE_CONTROL, ctl_code);
  result |= mgt_write(mgt, CONTROL, 0x0a);
  return result;
}

int mgt_set_param(Mgt_t* mgt, int address, int value) {
  int result = 1;
  result |= mgt_write(mgt, CONTROL, 0x05);
  result |= mgt_write(mgt, address, value);
  result |= mgt_write(mgt, CONTROL, 0x0a);
  return result;
}

int mgt_set_param2(Mgt_t* mgt, int address, int value) {
  int result = 1;
  result |= mgt_write(mgt, CONTROL, 0x05);
  result |= mgt_write(mgt, address, value);
  address += 1;
  value = value >> 8;
  result |= mgt_write(mgt, address, value);
  result |= mgt_write(mgt, CONTROL, 0x0a);
  return result;
}

int mgt_get_param(Mgt_t* mgt, int address) {
  return mgt_read(mgt, address);
}

int mgt_read(Mgt_t* mgt, int reg) {
  if ( mgt == NULL ) return -1;
  if ( mgt->xmemp ) {
    unsigned long csr[4] = { 0x00100000, 0x00100200,
			     0x00100400, 0x00100600 };
    volatile unsigned long *regp = 
      mgt->xmemp + csr[mgt->slot] + reg*4;
    return (*regp >> 24) & 0xff;
  } else {
    int val;
    int ret = ioctl(mgt->fd, FNGENERICIO_GET(reg), &val);
    if (ret < 0) return ret;
    return val;
  }
}

int mgt_write(Mgt_t* mgt, int reg, int val) {
  if ( mgt == NULL ) return -1;
  if ( mgt->xmemp ) {
    unsigned long csr[4] = { 0x00100000, 0x00100200,
			     0x00100400, 0x00100600 };
    volatile unsigned long *regp =
      mgt->xmemp + csr[mgt->slot] + reg*4;
    *regp = (val & 0xff) << 24;
    return 0;
  } else {
    return ioctl(mgt->fd, FNGENERICIO_SET(reg), val);
  }
}
/* ---------------------------------------------------------------------- *\
   mgt_write_fpga
\* ---------------------------------------------------------------------- */
void mgt_write_fpga(Mgt_t* mgt, int m012, int ch, int n) {
  int i, mask, data;
  if ( m012 == M012_SERIAL ) {
    for ( mask = 0x80; mask > 0; mask >>= 1 ) {
      data = (ch & mask) ? 1 : 0; // bit-0 = DIN
      mgt_write(mgt, MGT_CCLK, data);
    }
  } else if ( m012 == M012_SELECTMAP ) {
    for ( i=0; n == 0 && i<1000; i++ ) {
      data = mgt_read(mgt, MGT_CCLK);
      mask = mgt_read(mgt, MGT_CONF);
      if (data == 0 && (mask & 0x0f) == 0x0e) break;
      usleep(1);
    }
    if ( i == 1000 ) {
      printf("time out at byte %d\n", n);
      exit(1);
    }
    if ( i > 0 ) printf("wait %d at byte %d\n", i, n);
    data = 0;
    for ( i = 0, mask = 0x80; mask > 0; mask >>= 1, i++ ) {
      data |= (ch & mask) ? (1<<i) : 0;
    }
    mgt_write(mgt, MGT_CCLK, data);
  }
}
/* ---------------------------------------------------------------------- *\
   mgt_dump_fpga
\* ---------------------------------------------------------------------- */
void mgt_dump_fpga(int conf, const char *str) {
  printf("CONF register=%02x M012=%d INIT=%d DONE=%d%s%s\n",
	 conf&0xff, conf&7, (conf>>3)&1, (conf>>7)&1,
	 str?" ":"", str?str:"");
}
/* ---------------------------------------------------------------------- *\
   boot_fpga
\* ---------------------------------------------------------------------- */
int mgt_boot_fpga(Mgt_t* mgt, const char *file, 
		  int verbose, int forced, int m012) {
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
  conf = mgt_read(mgt, MGT_CONF);
  if (verbose) mgt_dump_fpga(conf, 0);

  /* -- download mode (set M012) -- */
  mgt_write(mgt, MGT_CONF, 0x08 | m012);
  conf = mgt_read(mgt, MGT_CONF) & 7;
  if (verbose || conf != m012) mgt_dump_fpga(conf, "(set M012)");
  if (conf != m012) {
    printf("cannot set FPGA to the download mode (M012=%d).\n", conf);
    fclose(fp);
    if (! forced) return -1;
  }
  
  /* -- programming mode (CONF<=1) -- */
  mgt_write(mgt, MGT_CONF, 0x41);
  mgt_write(mgt, MGT_CONF, 0x87);
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  conf = mgt_read(mgt, MGT_CONF);
  if (verbose || (conf & 0x80)) mgt_dump_fpga(conf, "(PRGM<=1)");
  if (conf & 0x80) {
    printf("cannot set FPGA to the programming mode.\n");
    if (! forced) return -1;
  }

  mgt_write(mgt, MGT_CONF, 0x86);
  mgt_dump_fpga(conf, "(PRGM=0)");
  usleep(1000); /* not sure if this sleep is needed for COPPER */
  
  /* -- skip first 16 bytes -- */
  for (i = 0; i<16; i++) ch = getc(fp);
    
  /* -- get and print header -- */
  if (verbose) printf("== file %s ==\n", file);
  while ((ch = getc(fp)) != 0xff && ch != EOF) {
    if (verbose) putchar(isprint(ch) ? ch : '.');
  }
  if (verbose) putchar('\n');
  if (ch == EOF) {
    printf("immature EOF for %s\n", file);
    return -1;
  }

  /* -- main part -- */
  do {
    mgt_write_fpga(mgt, m012, ch, nbyte++);
    count++;
    if (verbose && (count % 10000) == 0) {
      printf("%d bytes written (%d)\n", count, (int)time(0));
    }
  } while ((ch = getc(fp)) != EOF);
  if (verbose) printf("count = %d\n", count);

  while (count++ < length / 8) {  /* -- is it needed? -- */
    mgt_write_fpga(mgt, m012, 0xff, nbyte++);
  }
  
  for (i=0; i<100; i++) mgt_write_fpga(mgt, m012, 0xff, nbyte++);
  fclose(fp);
  if (verbose ) mgt_dump_fpga(conf, "");
  mgt_write(mgt, MGT_CONF, 0x40); /* clear ce_b */
  conf = mgt_read(mgt, MGT_CONF);
  if (verbose) mgt_dump_fpga(conf, "");
  mgt_write(mgt, MGT_CONF, 0x0f); /* clear m012 = 6 */
  conf = mgt_read(mgt, MGT_CONF);
  if (verbose) mgt_dump_fpga(conf, "");
  conf = mgt_read(mgt, MGT_CONF);
  if (verbose & ! (conf & 0x80)) mgt_dump_fpga(conf, "");

  if (conf & 0x80) {
    printf("done.\n");
    return 0;
  } else {
    printf("failed.\n");
    return -1;
  }
}

