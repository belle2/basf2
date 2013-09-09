/*
 * A ttrx device driver
 * eunil@hep.korea.ac.kr
 *
 * $Id: ttrx.h,v 1.1.1.1 2005/02/23 09:44:12 ttrx Exp $
 * $Log: ttrx.h,v $
 * Revision 1.1.1.1  2005/02/23 09:44:12  ttrx
 * First into cvs
 *
 *
 */


#define TTRX_NAME         "ttrx"
#define TTRX_FIFO_NAME    "ttrx_fifo"
#define TTRX_FNAME        "/dev/ttrx"
#define TTRX_FIFO_FNAME   "/dev/ttrx_fifo"

#ifndef CONFIG_PCI
# error "This driver needs PCI support to be available"
#endif
#define TTRX_VENDOR      0x10b5
#define TTRX_DEVID       0x9054
#define TTRX_SUB_VENDOR  0x10b5
#define TTRX_SUB_ID      0x5258
#define TTRX_MAX_DEV     2       // there are two PCI slots for ttrx
#define TTRX_REG_LENGTH  0x0450  // the length of the register address space
#define TTRX_ADDR_OFF    4       // last two bits seem to be dropped
#define TTRX_FIFO_LENGTH 8       // number of fifo * unsigned int

#include <linux/ioctl.h>

void cleanup_module(void);

/*
 * local to ttrx device
 */
typedef struct {
  unsigned int nopen;
  unsigned int pver;
} ttrx_regs;

typedef struct {
  struct        pci_dev* pcidev;
  int           id;
  int           initialized;
  unsigned int  nopen;
  char          name[32];
  caddr_t       mreg_page;

  volatile      ttrx_regs* mreg;
  u32*           ttrx_base;

} ttrx_t;

typedef ttrx_t* dev_p;
