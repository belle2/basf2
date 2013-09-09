/*
 *  A ttrx device driver
 *  eunil@hep.korea.ac.kr
 *
 * $Id: ttrx.c,v 1.1.1.1 2005/02/23 09:44:12 ttrx Exp $
 * $Log: ttrx.c,v $
 * Revision 1.1.1.1  2005/02/23 09:44:12  ttrx
 * First into cvs
 *
 *
 */

#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#define MODVERSIONS
#endif

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

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

#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
static struct proc_dir_entry *proc_entry = 0;
static int    proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len)
{
   if (len <= off+count) *eof = 1;
   *start = page + off;
   len -= off;
   if (len>count) len = count;
   if (len<0) len = 0;
   return len;
}

static int    ttrx_procfile_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
  int i, len = 0;
  int *base;
  unsigned int  pid, pver,prgm,sclk,rclk,id,ver,creg,freg,intr,trig,tlast;
  unsigned int  rate,tag,tagr,depth,tio,ttest,tstat,tsav,tsav2,tng32;

  char buf[256];
  double freq;
 
  len += sprintf(page+len,TTRX_NAME" device count :%2d \n",device_count);
  for (i=0;i<device_count;i++)
  {
    dev_p dv = &dvtbl[i];
    base = (int *) dv->mreg_page;
    pid     = base[TTRX_PID]   & 0xf;
    pver    = base[TTRX_PVER]  & 0xf;
    prgm    = base[TTRX_PRGM]  & 0xf;
    sclk    = base[TTRX_SCLK]  & 0xf;

    rclk    = base[TTRX_RCLK];
    id      = base[TTRX_ID];
    ver     = base[TTRX_VER];
    creg    = base[TTRX_CREG];
    freg    = base[TTRX_FREG];
    intr    = base[TTRX_INTR];

    trig    = base[TTRX_TRIG];
    tlast   = base[TTRX_TLAST];
    rate    = base[TTRX_RATE];
    tag     = base[TTRX_TAG];
    tagr    = base[TTRX_TAGR];
    depth   = base[TTRX_DEPTH];

    tio     = base[TTRX_TIO];
    ttest   = base[TTRX_TTEST];
    tstat   = base[TTRX_TSTAT];
    tsav    = base[TTRX_TSAV];
    tsav2   = base[TTRX_TSAV2];
    tng32   = base[TTRX_TNG32];

    len += sprintf(page+len,"------------------------------------");
    len += sprintf(page+len,"------------------------------------\n");
    len += sprintf(page+len,"/dev/%.32s ",dv->name);
    len += sprintf(page+len,"ttrx version %d\n", TTRX_VERS);
    len += sprintf(page+len,"------------------------------------");
    len += sprintf(page+len,"------------------------------------\n");
    len += sprintf(page+len,"CPLD:\n");
    len += sprintf(page+len,"000    ID=%01x %s\n", pid,  pid==0x3?"OK":(pid==2?"OK(v3)":"NG"));
    len += sprintf(page+len,"010   VER=%01x (CPLD version 0.%02d)\n", pver, pver);
    len += sprintf(page+len,"020  PRGM=%01x (lock*=%d m012=%d init=%d done=%d)\n", prgm, (prgm>>3)&1, (prgm>>2)&1, (prgm>>1)&1, prgm&1);
    len += sprintf(page+len,"030  SCLK=%01x (tclksel=%d clk=%s edgei=%d edgeo=%d)\n", sclk, (sclk>>3)&1, sclk&4?"global":"local", (sclk>>1)&1, sclk&1);
    
    len += sprintf(page+len,"\n");
    len += sprintf(page+len,"FPGA:\n");
    len += sprintf(page+len,"080  RCLK=%08x rdiv=%d ttrig=%d localtrg=%d trig=%d trigin=%d ext=%x\n",rclk, rclk&0xff,(rclk>>8)&1, (rclk>>9)&1, (rclk>>10)&1,(rclk>>11)&1,(rclk>>12)&15);
    len += sprintf(page+len,"100    ID=%08x %s\n", id, id==0x58525454?"OK":"NG");
    len += sprintf(page+len,"110   VER=%08x TTRX version %d FPGA version %d.%02d rev %02d\n", ver, (ver>>28)&0xf, (ver>>16)&0xfff, (ver>>8)&0xff, ver&0xff);
    len += sprintf(page+len,"120  CREG=%08x revo=%c frst=%c srst=%c lrst=%c iena=%c abrt=%c\n",
           creg,
           ((creg>>31)&1)?((creg>>23)&1)+'0':'Z',
           ((creg>>30)&1)?((creg>>22)&1)+'0':'Z',
           ((creg>>29)&1)?((creg>>21)&1)+'0':'Z',
           ((creg>>28)&1)?((creg>>20)&1)+'0':'Z',
           ((creg>>27)&1)?((creg>>19)&1)+'0':'Z',
           ((creg>>26)&1)?((creg>>18)&1)+'0':'Z');
    len += sprintf(page+len,"%19sentestbsy=%d enful=%d exbsy=%x bsydone=%x ready=%d\n","",(creg>>17)&1, (creg>>16)&1,(creg>>12)&15, (creg>>8)&15, (creg>>7)&1);
    len += sprintf(page+len,"%19snwff=%d testbsy=%d swbsy=%d orun=%d full=%d half=%d emp=%d\n","", (creg>>6)&1, (creg>>5)&1, (creg>>4)&1,(creg>>3)&1, (creg>>2)&1, (creg>>1)&1, (creg>>0)&1);

    len += sprintf(page+len,"130  FREG=%08x io2/io1/ful/emp/bsy/hand/flg/mask:DCBA\n", freg);
    len += sprintf(page+len,"140  INTR=%08x irq=%d encdbg=%d\n", intr, intr&1, (intr>>31)&1);
    len += sprintf(page+len,"200  TRIG=%08x disg=%d decg=%d enau=%d pls=%d rnd=%d free=%d n=%d\n", trig, (trig>>31)&1, (trig>>30)&1, (trig>>29)&1, (trig>>28)&1, (trig>>27)&1, (trig>>26)&1, trig&((1<<24)-1)); 
    len += sprintf(page+len,"210 TLAST=%08x n=%d\n", tlast, tlast);
    len += sprintf(page+len,"220  RATE=%08x refclk=%d Hz[28:24] rndset=%d[17:16] plsset=1/%d[9:0]\n", rate, 42333000/(1<<((rate>>24)&31)), (rate>>16)&3, (rate&((1<<10)-1))+1); 
    len += sprintf(page+len,"230   TAG=%08x\n", tag);
    len += sprintf(page+len,"250 DEPTH=%08x\n", depth);
    /*
     * One should not allow FIFO reading in proc entry
     */
    if (0) {
      unsigned int fifo1   = base[0x30 << 2];
      unsigned int fifo2   = base[(0x30 << 2) + 1];
      len += sprintf(page+len,"\n");
      len += sprintf(page+len,"300   fifo1=%08x\n", fifo1);
      len += sprintf(page+len,"304   fifo2=%08x\n", fifo2);
    }
    len += sprintf(page+len,"400   TIO=%08x in=%03x out=%03x usr=%03x\n",tio, (tio>>20)&0x3ff, (tio>>10)&0x3ff, tio&0x3ff);
    len += sprintf(page+len,"410 TTEST=%08x othru=%d oincr=%d enusr=%d\n",ttest, (ttest>>30)&1, (ttest>>29)&1, (ttest>>28)&1);
    len += sprintf(page+len,"%19stcmd=%1x tout=%02x tin_de=%02x tdiff=%02x\n", "", (ttest>>24)&15, (ttest>>16)&255, (ttest>>8)&255, ttest&255);
    len += sprintf(page+len,"420 TSTAT=%08x Nng1=%d Nng2=%d Nngc=%d Nngany=%d\n",
           tstat, (tstat>>24)&255, (tstat>>16)&255, (tstat>>8)&255, tstat&255);
    len += sprintf(page+len,"430  TSAV=%08x ng1=%d ng2=%d x1=%d x2=%d x=%d ngc=%d\n", tsav, (tsav>>31)&1, (tsav>>30)&1, (tsav>>29)&1, (tsav>>28)&1, (tsav>>27)&1, (tsav>>24)&1); 
    len += sprintf(page+len,"%19stincr=%02x tin=%02x texp=%02x\n", "", (tsav>>16)&255, (tsav>>8)&255, tsav&255);
    len += sprintf(page+len,"440 TSAV2=%08x ng1=%d ng2=%d x1=%d x2=%d x=%d ngc=%d\n", tsav2, (tsav2>>31)&1, (tsav2>>30)&1, (tsav2>>29)&1, (tsav2>>28)&1, (tsav2>>27)&1, (tsav2>>24)&1);
    len += sprintf(page+len,"%19stincr=%02x tin=%02x texp=%02x\n", "", (tsav2>>16)&255, (tsav2>>8)&255, tsav2&255);
    len += sprintf(page+len,"450 TNG32=%08x count=%d\n", tng32, tng32);
    len += sprintf(page+len,"------------------------------------");
    len += sprintf(page+len,"------------------------------------\n");
    if (1) {
      len += sprintf(page+len,"Summary:\n");
      buf[0] = 0;
      freq = 0;
      if (trig & (1<<28)) {
        sprintf(buf+strlen(buf), "%s%s", *buf?"+":"", "Pulse");
        freq = 42333000.0/(1<<((rate>>24)&31))/(1+(rate&((1<<10)-1)));
      }
      if (trig & (1<<27)) {
        sprintf(buf+strlen(buf), "%s%s", *buf?"+":"", "Random");
        freq = 42333000.0/(1<<((rate>>24)&31))/512;
      }
      if (trig & (1<<29)) sprintf(buf+strlen(buf), "%s%s", *buf?"+":"", "User");
      if (! *buf) strcpy(buf, "None");

      len += sprintf(page+len,"    Trigger source=%s  count=%d", buf, tag);

      if (freq > 0) len += sprintf(page+len," freq=%3.1f Hz", freq);
      if (trig & ((1<<24)-1)) {
        len += sprintf(page+len," out of %d (last=%d)\n", trig & ((1<<24)-1), tlast);
      } else {
        len += sprintf(page+len," (free-run)\n");
      }

      len += sprintf(page+len,"  Handshake ");
      if (creg & (1<<7)) {
        len += sprintf(page+len,"READY\n");
      } else {
        int rx_fifofull = 0;

        len += sprintf(page+len,"NOT-READY");
        creg &= ((freg>>8)&15)<<12 | ~(15<<12);
        if (creg & (15<<12)) {
          len += sprintf(page+len," extra-busy=%s%s%s%s",
               (creg & (1<<12)) ? "A" : "",
               (creg & (2<<12)) ? "B" : "",
               (creg & (4<<12)) ? "C" : "",
               (creg & (8<<12)) ? "D" : "");
        }
        freg &= ~((freg&15)<<20);      /* mask => full */
        freg &= ~((freg&15)<<12);      /* mask => busy */
        freg &= ~(((freg>>12)&15)<<8); /* busy => hand (for nobusy) */
        freg &= ~(((freg>>4)&15)<<12); /* flag => busy */
        if (freg & (15<<8)) {
          len += sprintf(page+len," no-busy=%s%s%s%s",
               (freg & (1<<8)) ? "A" : "",
               (freg & (2<<8)) ? "B" : "",
               (freg & (4<<8)) ? "C" : "",
               (freg & (8<<8)) ? "D" : "");
        }
        if (freg & (15<<12)) {
          len += sprintf(page+len," busy=%s%s%s%s",
               (freg & (1<<12)) ? "A" : "",
               (freg & (2<<12)) ? "B" : "",
               (freg & (4<<12)) ? "C" : "",
               (freg & (8<<12)) ? "D" : "");
        }
        if ((creg & (1<<2)) && ! (creg & (1<<25))) rx_fifofull++;
        if ((creg & (1<<1)) && ! (creg & (1<<24))) rx_fifofull++;

        if ((freg & (15<<20)) || (creg & (1<<6)) || rx_fifofull) {
          len += sprintf(page+len," fifo-full=%s%s%s%s%s%s",
               (freg & (1<<20)) ? "A" : "",
               (freg & (2<<20)) ? "B" : "",
               (freg & (4<<20)) ? "C" : "",
               (freg & (8<<20)) ? "D" : "",
               (creg & (1<<6))  ? "(NWFF)" : "",
               rx_fifofull ? "(TT-RX)" : "");
        }
        len += sprintf(page+len,"\n");
      }
    }
    len += sprintf(page+len,"------------------------------------");
    len += sprintf(page+len,"------------------------------------\n");
  }
  return proc_calc_metrics(page, start, off, count, eof, len);
}
#endif


/*
 * initialize the deivce
 */
static int ttrx_initialize(dev_p dv)
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
    printk("<1>(ttrx) %s not a memory mapped device\n",dv->name);
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
static int ttrx_open(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR (inode->i_rdev);
  dev_p dv;

  if (minor >= device_count) return -ENXIO;
  dv = &dvtbl[minor];
  if (dv->nopen < 0) return -EBUSY;         
  if (!dv->initialized) return -EIO;        // hardware error?

  dv->nopen++;

  printk("<1>(ttrx) ttrx_open \n");
  MOD_INC_USE_COUNT;                        // do not allow the module to be
                                            // removed while in use
  
  return 0;                                 // successful end
}

/*
 * close the device
 */
static int ttrx_close(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR(inode->i_rdev);

  dvtbl[minor].nopen--;
  MOD_DEC_USE_COUNT;
  printk("<1>(ttrx) ttrx_close \n");
  return 0;                                 // successful end
}

/*
 * slow read (no DMA)
 */
static ssize_t ttrx_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
  unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
  ssize_t count = 0;
  int err;
  dev_p dv = &dvtbl[minor];
  int *base, reg_value;
  int i;

  /*
   * check the requested buffer size from the user space
   */
  if (length > (TTRX_REG_LENGTH+4))
  {
    printk("<1>(ttrx) ttrx_read cannot access buffer larger than %d \n",length);
    return -EFAULT;
  }

  err = access_ok(VERIFY_WRITE,(void *) buffer, length);
  if (err <0) return err;
    
  i = (file->f_pos);
  base = (int *) dv->mreg_page;
  while (count < length)
  {
    if ((i==TTRX_FIFO1)||(i==TTRX_FIFO2))
    {
      printk("<1>(ttrx) ttrx_read: TTRX_FIFO not readable at addr=0x%x \n", i);
      return -1;
    } 
    else
    {
      reg_value = *(base+i);
      put_user(reg_value, (int *) buffer);
      count  += sizeof(int)*TTRX_ADDR_OFF;
      buffer += sizeof(int)*TTRX_ADDR_OFF; 
      i+= TTRX_ADDR_OFF;
    }
  }
  return count;
}

static ssize_t ttrx_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
  unsigned int minor = MINOR(file->f_dentry->d_inode->i_rdev);
  ssize_t count = 0;
  int err;
  dev_p dv = &dvtbl[minor];
  int *base, reg_value;
  int i=0;

  /*
   * check the requested buffer size from the user space
   */
  if (length > (TTRX_REG_LENGTH+4))
  {
    printk("<1>(ttrx) ttrx_write cannot access buffer larger than %d \n",length);
    return 0;
  }

  if (length == 0) return 0;

  err = access_ok(VERIFY_READ,(void *) buffer, length);
  if (err <0) return err;

  i = (file->f_pos);
  base = (int *) dv->mreg_page;
  while(count < length)
  {
    get_user(reg_value, (int *) buffer);
    *(base+i) = reg_value;    
    count  += sizeof(int)*TTRX_ADDR_OFF;
    buffer += sizeof(int)*TTRX_ADDR_OFF;
    i += TTRX_ADDR_OFF;
  }
  return count;
}

/*
 * lseek of the device 
 */
loff_t ttrx_lseek(struct file *filp, loff_t off, int whence)
{
  loff_t newpos;

  /*
   * check the requested buffer size from the user space
   */
  if (off > (TTRX_REG_LENGTH+4))
  {
    printk("<1>(ttrx) ttrx_lseek cannot access buffer larger than %u \n",(unsigned) off);
    return -EFAULT;
  }

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
static int ttrx_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
  return 0;
}

/*
 * file operation structure (Kernel Version 2.2.0 later)
 */
struct file_operations ttrx_fops = {
  read  :    ttrx_read,
  write :    ttrx_write,
  llseek:    ttrx_lseek,
  owner :    THIS_MODULE,
  ioctl :    ttrx_ioctl,
  open  :    ttrx_open,
  release:   ttrx_close,
};

#ifdef MODULE

MODULE_AUTHOR("Eunil Won, Korea University");
MODULE_DESCRIPTION("ttrx for COPPER-II");
MODULE_LICENSE("GPL");

int init_module(void)
{
  int  result;

  u16  sub_ven, sub_dev;
  u32  base0, base1, base2;
  struct pci_dev *dev = NULL;

  printk("<1>(ttrx) init_module \n");

  /*
   *  register device major number (dynamically)
   */
  ttrx_major = 0;
  result = register_chrdev(ttrx_major, TTRX_NAME, &ttrx_fops);
  if (result < 0)
  {
    printk("<1>(ttrx) cannot get major %d\n", ttrx_major);
    return -1;
  }
  if (ttrx_major == 0)
    ttrx_major = result;

  /* 
   * pci available in this box ?
   */		
  if (!pci_present()) {
    printk("<1>(ttrx) pci functionality not available.\n");
    cleanup_module();
    return -ENODEV;
  }
  
  /* 
   * Search for a ttrx pci device
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
      printk("<1>(ttrx) you seem to have more than %d device(s), skipping rest of them (for now)...\n",TTRX_MAX_DEV);
      break;
    }
    if (dev->vendor == TTRX_VENDOR && dev->device == TTRX_DEVID &&
        sub_ven == TTRX_SUB_VENDOR && sub_dev == TTRX_SUB_ID )
    {
      printk("<1>(ttrx) --> devfn:0x%x vendor:0x%x id:0x%x class:0x%x sub_venID:0x%x sub_devID:0x%x base0:0x%x base1:0x%x base2:0x%x\n",dev->devfn,dev->vendor,dev->device,dev->class,sub_ven,sub_dev,base0,base1,base2);

      /*
       * save the device found
       */ 
      dv = &dvtbl[device_count];
      memset(dv,0,sizeof(*dv));
      dv->pcidev = dev;
      dv->id = device_count;
      sprintf(dv->name,TTRX_NAME"%d",dv->id);
      device_count++;
      ttrx_initialize(dv);
    }
  }

  /*
   * do not install the module when 0 devices are detected
   */
  if (device_count == 0)
  {
    printk("<1>(ttrx) found no devices \n");
    cleanup_module();
    return -ENODEV;
  }

  printk("<1>(ttrx) found %d ttrx device(s).\n",device_count);
  printk("<1>(ttrx) install succeeded. (Ver. %d.%d) \n", TTRX_VERS >> 16, TTRX_VERS & 0xFFFF);

#ifdef CONFIG_PROC_FS
  /*
   * register a proc-fs entry
   */
  if ((proc_entry = create_proc_entry(TTRX_NAME,0,0)))
    proc_entry->read_proc = ttrx_procfile_read;
  else
    printk(KERN_WARNING" (ttrx) failed to register /proc filesystem \n");
#endif

  return 0;

}

/*
 *module cleanup
 */
void cleanup_module(void)
{
  printk("<1>(ttrx) cleanup_module \n");
  unregister_chrdev(ttrx_major, TTRX_NAME);

#ifdef CONFIG_PROC_FS
   if (proc_entry)
   {
      remove_proc_entry(TTRX_NAME, 0);
      proc_entry = 0; 
   }
#endif

}
#endif
