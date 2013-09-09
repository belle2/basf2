/*
 *  A ttrx device driver
 *  eunil@hep.korea.ac.kr
 *
 * This is a device driver specific to reading fifo only
 *
 * $Id: ttrx_fifo.c,v 1.1.1.1 2005/02/23 09:44:12 ttrx Exp $
 * $Log: ttrx_fifo.c,v $
 * Revision 1.1.1.1  2005/02/23 09:44:12  ttrx
 * First into cvs
 *
 *
 */

#include <linux/autoconf.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/stddef.h>
#include <linux/ioport.h>
#include <asm/uaccess.h>

#include "ttrx.h"
#include "ttrxlib.h"

static int      device_count = 0;
static int      ttrx_major;
static ttrx_t   dvtbl[TTRX_MAX_DEV];

#define  TTRX_VERS   0x00000001
#define  TTRX_DATE   0x20050116

/*
 * initialize the deivce
 */
static int ttrx_fifo_initialize(dev_p dv)
{
  unsigned long phys, offset;
  int *tbase;

  /*
   * memory mapped resources 
   */
  phys   = pci_resource_start(dv->pcidev,2);    // get the base2 ptr
  phys  &= PCI_BASE_ADDRESS_MEM_MASK; 
  offset = phys & ~PAGE_MASK;
  if (phys & PCI_BASE_ADDRESS_SPACE)
  {
    printk("<1>(ttrx_fifo) %s not a memory mapped device\n",dv->name);
  }
  dv->mreg_page = ioremap(phys & PAGE_MASK , 0x4096);
  dv->mreg = (ttrx_regs *) ((caddr_t) dv->mreg_page + offset);
  tbase = (int *) dv->mreg_page;
  dv->ttrx_base = (u32 *) dv->mreg_page;

  dv->nopen=0;
  dv->initialized = 1;
  return 0;
}  

/*
 * open the device
 */
static int ttrx_fifo_open(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR (inode->i_rdev);
  dev_p dv;

  if (minor >= device_count) return -ENXIO;
  dv = &dvtbl[minor];
  if (dv->nopen) return -EBUSY;             // allow only one pid to open
  if (!dv->initialized) return -EIO;        // hardware error?

  dv->nopen = current->pid;                 // save the pid that opens this

  printk("<1>(ttrx_fifo) ttrx_fifo_open \n");
  return 0;                                 // successful end
}

/*
 * close the device
 */
static int ttrx_fifo_close(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR(inode->i_rdev);

  dvtbl[minor].nopen = 0;
  printk("<1>(ttrx_fifo) ttrx_fifo_close \n");
  return 0;                                 // successful end
}

/*
 * slow read (no DMA)
 */
static ssize_t ttrx_fifo_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
  unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
  ssize_t count = 0;
  int err;
  dev_p dv = &dvtbl[minor];
  int *base, reg_value;

  /*
   * check the requested buffer size from the user space
   */
  if (length > TTRX_FIFO_LENGTH)
  {
    printk("<1>(ttrx_fifo) ttrx_fifo_read cannot access buffer larger than %d \n",length);
    return -EFAULT;
  }

  err = access_ok(VERIFY_WRITE,(void *) buffer, length);
  if (err <0) return err;
    
  base = (int *) dv->mreg_page;

  reg_value = *(base+TTRX_FIFO1);
  put_user(reg_value, (int *) buffer);
  count  += sizeof(int);
  buffer += sizeof(int); 

  reg_value = *(base+TTRX_FIFO2);
  put_user(reg_value, (int *) buffer);
  count  += sizeof(int);
  buffer += sizeof(int); 

  return count;
}

/*
 * lseek of the device 
 */
loff_t ttrx_fifo_lseek(struct file *filp, loff_t off, int whence)
{
  loff_t newpos;

  switch (whence)
  {
    case 0:
      newpos = off;                   // SEEK_SET
      break;
    case 1:
      newpos = filp->f_pos + off;     // SEEK_CUR
      break;
    case 2:
      newpos = TTRX_REG_LENGTH + off; // SEEK_END, not sure this is right
                                      //           thing to do
      break;
    default:
      return -EINVAL;
  }

  if (newpos<0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}
/*
 * ioctl of the device (don't do anything for now)
 */
static int ttrx_fifo_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
  return 0;
}

/*
 * file operation structure (Kernel Version 2.2.0 later)
 */
struct file_operations ttrx_fops = {
  read  :    ttrx_fifo_read,
  llseek:    ttrx_fifo_lseek,
  owner :    THIS_MODULE,
  ioctl :    ttrx_fifo_ioctl,
  open  :    ttrx_fifo_open,
  release:   ttrx_fifo_close,
};

#ifdef MODULE

MODULE_AUTHOR("Eunil Won, Korea University");
MODULE_DESCRIPTION("ttrx_fifo for COPPER-II");
MODULE_LICENSE("GPL");

int init_module(void)
{
  int  result;

  u16  sub_ven, sub_dev;
  u32  base0, base1, base2;
  struct pci_dev *dev = NULL;

  printk("<1>(ttrx_fifo) init_module \n");

  /*
   *  register device major number (dynamically)
   */
  ttrx_major = 0;
  result = register_chrdev(ttrx_major, TTRX_FIFO_NAME, &ttrx_fops);
  if (result < 0)
  {
    printk("<1>(ttrx_fifo) cannot get major %d\n", ttrx_major);
    return -1;
  }
  if (ttrx_major == 0)
    ttrx_major = result;

  /* 
   * Search for a ttrx_fifo pci device
   */
  device_count = 0; dev = NULL; 

  while (1)
  {
    dev_p dv;

    dev = pci_find_device(TTRX_VENDOR,TTRX_DEVID,dev);
    if (dev == NULL) break;
    pci_read_config_word(dev,PCI_SUBSYSTEM_VENDOR_ID,&sub_ven);
    pci_read_config_word(dev,PCI_SUBSYSTEM_ID,&sub_dev);
    pci_read_config_dword(dev,PCI_BASE_ADDRESS_0,&base0);
    pci_read_config_dword(dev,PCI_BASE_ADDRESS_1,&base1);
    pci_read_config_dword(dev,PCI_BASE_ADDRESS_2,&base2);
    if (device_count >= TTRX_MAX_DEV)
    {
      printk("<1>(ttrx_fifo) you seem to have more than %d device(s), skipping rest of them (for now)...\n",TTRX_MAX_DEV);
      break;
    }
    if (dev->vendor == TTRX_VENDOR && dev->device == TTRX_DEVID &&
        sub_ven == TTRX_SUB_VENDOR && sub_dev == TTRX_SUB_ID )
    {
      printk("<1>(ttrx_fifo) --> devfn:0x%x vendor:0x%x id:0x%x class:0x%x sub_venID:0x%x sub_devID:0x%x base0:0x%x base1:0x%x base2:0x%x\n",dev->devfn,dev->vendor,dev->device,dev->class,sub_ven,sub_dev,base0,base1,base2);

      /*
       * save the device found
       */ 
      dv = &dvtbl[device_count];
      memset(dv,0,sizeof(*dv));
      dv->pcidev = dev;
      dv->id = device_count;
      sprintf(dv->name,TTRX_FIFO_NAME"%d",dv->id);
      device_count++;
      ttrx_fifo_initialize(dv);
    }
  }

  /*
   * do not install the module when 0 devices are detected
   */
  if (device_count == 0)
  {
    printk("<1>(ttrx_fifo) found no devices \n");
    cleanup_module();
    return -ENODEV;
  }

  printk("<1>(ttrx_fifo) found %d ttrx_fifo device(s).\n",device_count);
  printk("<1>(ttrx_fifo) install succeeded. (Ver. %d.%d) \n", TTRX_VERS >> 16, TTRX_VERS & 0xFFFF);

  return 0;

}

/*
 *module cleanup
 */
void cleanup_module(void)
{
  printk("<1>(ttrx_fifo) cleanup_module \n");
  unregister_chrdev(ttrx_major, TTRX_FIFO_NAME);
}
#endif
