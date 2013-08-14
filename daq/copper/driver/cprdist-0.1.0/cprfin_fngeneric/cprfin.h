/* cprfin.h */


#ifndef CPRFIN_H
#define CPRFIN_H


#ifdef __KERNEL__


/* call request_mem_region in cprfin (define) or use requested region by pcilcl (undef) ? */
// #undef CPRFIN_CALL_REQUEST_MEM_REGION
#define CPRFIN_CALL_REQUEST_MEM_REGION


/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

#include <linux/ioctl.h>

#ifndef PCI_VENDOR_ID_PLX
#define PCI_VENDOR_ID_PLX   (0x10b5)
#endif
#define PCI_DEVICE_ID_9054  (0x9054)
#define PCI_SUBSYS_ID_9054  (0x905410b6)

#define cprfin_finesse_base_addr(slot)  ((unsigned char*)(CPRFIN_finesse_mem[slot].m_map_addr))
#define cprfin_finesse_reg(slot, addr) ((*(unsigned int*)(cprfin_finesse_base_addr(slot) + addr)) >> 24)


struct cprfin_finesse_mem {
  unsigned int m_prg_addr;
  unsigned int m_map_addr;
  unsigned int m_size;
};

#define FINESSE_GENERIC_REG_CSR      (480)
#define FINESSE_GENERIC_REG_FIFO_EMU (484)
#define FINESSE_GENERIC_REG_FPGA_VER (488)
#define FINESSE_GENERIC_REG_SERIAL_L (492)
#define FINESSE_GENERIC_REG_SERIAL_H (496)
#define FINESSE_GENERIC_REG_TYPE_L   (500)
#define FINESSE_GENERIC_REG_TYPE_H   (504)
#define FINESSE_GENERIC_REG_RESERVED (508)

enum finesse_generic_info {
  FINESSE_GENERIC_INFO_NOP,
  FINESSE_GENERIC_INFO_CSR,
  FINESSE_GENERIC_INFO_FIFO_EMU,
  FINESSE_GENERIC_INFO_FPGA_VER,
  FINESSE_GENERIC_INFO_SERIAL,
  FINESSE_GENERIC_INFO_TYPE,
  FINESSE_GENERIC_INFO_RESERVED,
};



/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

static struct cprfin_finesse_mem CPRFIN_finesse_mem[4];
static char CPRFIN_FINESSE_exist[4] = { 0, 0, 0, 0 };


#define cprfin_exist_finesse(slot) (CPRFIN_FINESSE_exist[slot])


/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

unsigned int
cprfin_read_finesse_generic_info(const int slot, const enum finesse_generic_info read_what)
{
  unsigned char tmp_h, tmp_l;


  switch (read_what) {
    case FINESSE_GENERIC_INFO_CSR:
      tmp_h = 0;
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_CSR);
      break;

    case FINESSE_GENERIC_INFO_FIFO_EMU:
      tmp_h = 0;
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_FIFO_EMU);
      break;

    case FINESSE_GENERIC_INFO_FPGA_VER:
      tmp_h = 0;
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_FPGA_VER);
      break;

    case FINESSE_GENERIC_INFO_SERIAL:
      tmp_h = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_SERIAL_H);
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_SERIAL_L);
      break;

    case FINESSE_GENERIC_INFO_TYPE:
      tmp_h = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_TYPE_H);
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_TYPE_L);
      break;

    case FINESSE_GENERIC_INFO_RESERVED:
      tmp_h = 0;
      tmp_l = cprfin_finesse_reg(slot, FINESSE_GENERIC_REG_RESERVED);
      break;
  }


  return tmp_h << 8 | tmp_l;
}


unsigned int
cprfin_ioctl_finesse_generic_info(const int slot, const int cmd)
{
  if (cmd & 0xf0) {
    /* cmd & 0x0f == read_what for cprfin_read_finesse_generic_info() */
    return cprfin_read_finesse_generic_info(slot, cmd & 0x0f);
  } else {
    switch (cmd & 0x0f) {
      case 0x01:
        return slot;
    }
    return 0;
  }
}



/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/



static struct pci_dev*
cprfin_find_plx9054(void)
{
  int ndev = 0;
  struct pci_dev* pdev = NULL;

  for (;;) {
    unsigned int subsys_vendor_id;

    pdev = pci_find_device(PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_9054, pdev);

    if (!pdev) {
      printk("cprfin: cannot find PLX9054\n");
      return NULL;
    }

    pci_read_config_dword(pdev, PCI_SUBSYSTEM_VENDOR_ID, &subsys_vendor_id);
    printk("cprfin: [%d] subsystem vendor id = %08x\n", ndev++, subsys_vendor_id);

    if (subsys_vendor_id == PCI_SUBSYS_ID_9054) return pdev;
  }

  return NULL;
}


static int
cprfin_find_finesse(const int find_type)
{
  int slot;
  int n_finesse = 0;

  for (slot = 0; slot < 4; slot++) {
    const int type = cprfin_read_finesse_generic_info(slot, FINESSE_GENERIC_INFO_TYPE);

    if (type == find_type) {
      printk("cprfin: finesse[%c]: type = %04x: requested type of finesse found\n", slot + 'a', type);
      CPRFIN_FINESSE_exist[slot] = 1;
      n_finesse++;
    } else {
      printk("cprfin: finesse[%c]: type = %04x: not requested type of finesse\n", slot + 'a', type);
    }
  }

  return n_finesse;
}


static int
cprfin_finesse_mmap(struct pci_dev* pdev)
{
#ifdef CPRFIN_CALL_REQUEST_MEM_REGION

  /* map 4 FINESSE modules in bar[2] of plx9054 */

  int i;
  int where = PCI_BASE_ADDRESS_0 + 2 * 4; /* bar[2] */
  unsigned int bar2_prg_addr;


  /* clean up */
  for (i = 0; i < 4; i++) {
    CPRFIN_finesse_mem[i].m_prg_addr = 0;
    CPRFIN_finesse_mem[i].m_map_addr = 0;
    CPRFIN_finesse_mem[i].m_size     = 0;
  }


  bar2_prg_addr = 0;
  pci_read_config_dword(pdev, where, &bar2_prg_addr);
  if (!bar2_prg_addr) return 0;  /* cannot get bar2_prg_addr */


  for (i = 0; i < 4; i++) {
    const unsigned int size = 0x200; /* memory space size per FINESSE module */
    const unsigned int prg_addr = bar2_prg_addr + 0x00100000 + 0x200 * i;
    unsigned int map_addr;
    struct resource* pres;

    pres = request_mem_region(prg_addr & PCI_BASE_ADDRESS_MEM_MASK, size, "cprfin");
    if (!pres) {
      printk("cprfin: finesse[%c]: request mem region at %p (%x) failed\n", i + 'a', prg_addr, size);
      return 0;
    }
    map_addr = ioremap(prg_addr & PCI_BASE_ADDRESS_MEM_MASK, size);
    printk("cprfin: finesse[%c]: memory map at %p (%x)\n", i + 'a', map_addr, size);

    CPRFIN_finesse_mem[i].m_prg_addr = prg_addr;
    CPRFIN_finesse_mem[i].m_map_addr = map_addr;
    CPRFIN_finesse_mem[i].m_size     = size;
  }


  return 1;

#else

  /* PCILCL_plx9054_bar2 is defined in pcilcl.o */
  int i;
  extern unsigned char* PCILCL_plx9054_bar2;

  for (i = 0; i < 4; i++) {
    CPRFIN_finesse_mem[i].m_prg_addr = 0;
    CPRFIN_finesse_mem[i].m_map_addr = (unsigned int)(PCILCL_plx9054_bar2 + 0x00100000 + 0x200 * i);
    CPRFIN_finesse_mem[i].m_size     = 0x200;
  }

  return 1;

#endif /* CPRFIN_CALL_REQUEST_MEM_REGION */
}


static void
cprfin_finesse_munmap(void)
{
#ifdef CPRFIN_CALL_REQUEST_MEM_REGION

  int i;

  for (i = 0; i < 4; i++) {
    if (!CPRFIN_finesse_mem[i].m_prg_addr) continue;

    iounmap((void*)CPRFIN_finesse_mem[i].m_map_addr);
    release_mem_region(CPRFIN_finesse_mem[i].m_prg_addr & PCI_BASE_ADDRESS_MEM_MASK, CPRFIN_finesse_mem[i].m_size);
    printk("cprfin: finesse[%c]: memory unmap\n", i + 'a');
  }

#else

  /* do nothing ... */

#endif /* CPRFIN_CALL_REQUEST_MEM_REGION */
}


static int
cprfin_init(const int type)
{
  struct pci_dev* pdev;

  pdev = cprfin_find_plx9054();
  if (!pdev) return -ENXIO;

  if (!cprfin_finesse_mmap(pdev)) return -ENXIO;

  cprfin_find_finesse(type);

  return 0;
}


static void
cprfin_term(void)
{
  cprfin_finesse_munmap();
}


#endif /* __KERNEL__ */



/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

#define CPRFIN_MAGIC        0xe0

#define FNIO_GET_SLOT     _IOR(CPRFIN_MAGIC, 0x01, sizeof(unsigned int))

#define FNIO_GET_CSR      _IOR(CPRFIN_MAGIC, 0xf1, sizeof(unsigned int))
#define FNIO_GET_FIFOEMU  _IOR(CPRFIN_MAGIC, 0xf2, sizeof(unsigned int))
#define FNIO_GET_FPGAVER  _IOR(CPRFIN_MAGIC, 0xf3, sizeof(unsigned int))
#define FNIO_GET_SERIAL   _IOR(CPRFIN_MAGIC, 0xf4, sizeof(unsigned int))
#define FNIO_GET_TYPE     _IOR(CPRFIN_MAGIC, 0xf5, sizeof(unsigned int))
#define FNIO_GET_RESERVED _IOR(CPRFIN_MAGIC, 0xf6, sizeof(unsigned int))


#endif  /* CPRFIN_H */

