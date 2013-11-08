#include "tailq.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/pci.h>		/* this includes sys/ioctl.h */
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include <linux/interrupt.h>

#include <linux/workqueue.h>

#include <asm/semaphore.h>
#include <linux/time.h>

#include "plx9054.h"
#include "copper.h"

#include "evq.h"

#define COPPER_DEVICE_NAME      "copper"
#define PLX9054_DEVICE_NAME     "plx9054"

static char *build_id = "nukaga $Id$";

static unsigned int COPPER_major = 199;
static unsigned int PLX9054_major = 198;

int plx9054_cnt;
int actual_intr_handl_cnt;
int enqueue_cnt;
int localint_cnt;

MODULE_LICENSE("GPL");

#ifndef NDMABUF
#define NDMABUF         4
#endif

#ifndef DMA_PAGES
#define DMA_PAGES       9
#endif

#define DMABUFSIZE      (PAGE_SIZE << DMA_PAGES)

#define COPPERREG(n)    ((volatile unsigned int *)(copper.reg + COPPER_##n))
#define PLX9054REG(n)   ((volatile unsigned int *)(plx9054.reg + PLX9054_##n))

#define DEBUG_READ_STATE        (1)
#define DEBUG_DMA               (1<<1)
#define DEBUG_ENQUEUE           (1<<2)
#define DEBUG_INTR              (1<<3)
#define DEBUG_DMABUF            (1<<4)
#define DEBUG_IOCTL             (1<<5)
#define DEBUG_DATA              (1<<6)
#define DEBUG_FLOW              (1<<7)
#define DEBUG_YAMADA            (1<<8)
#define DEBUG_DMA_REQUEST       (1<<9)
#define DEBUG_BAD_EVENT         (1<<10)
#define DEBUG_DMA_LOST          (1<<11)
#define DBG(n, ...)     if (debug_flag & DEBUG_##n) printk(__VA_ARGS__)

unsigned int debug_flag = 0;	// DEBUG_DMABUF; //|DEBUG_INTR;
//static unsigned int debug_flag = DEBUG_YAMADA;	// DEBUG_DMABUF; //|DEBUG_INTR;
//static unsigned int debug_flag = DEBUG_DATA | DEBUG_DMABUF; //|DEBUG_INTR;
//static unsigned int debug_flag = DEBUG_READ_STATE | DEBUG_DMA | DEBUG_ENQUEUE | DEBUG_INTR | DEBUG_DMABUF | DEBUG_IOCTL | DEBUG_DATA | DEBUG_FLOW | DEBUG_YAMADA;


// static wait_queue_head_t waitq;
static DECLARE_WAIT_QUEUE_HEAD(waitq);
static DECLARE_MUTEX(irq_handler);
static DECLARE_MUTEX(sem_critical);

static struct workqueue_struct * plx9054_workq = NULL;
void actual_interrupt_handler(struct work_struct *);
static DECLARE_WORK(work, &actual_interrupt_handler, 0);

static const char *read_state[] = { "START", "HEADER", "BODY", "FOOTER" };	/* for debug */

/* prototypes */
static void disable_pci_intr(void);
static void disable_local_intr(void);
static void disable_copper_intr(void);


static struct {
    int event_number;
    int read_state;		/* 0=start, 1=header, 2=body, 3=footer */
    int dma_count;		/* count of Actual DMA */
    int dma_try_count;		/* count of DMA try */
    int end_run;

    int fifo_full;
    int too_much;
    
} run_state;

static struct {
    int subsys;
    int crate;
    int slot;
} module_state;

static struct {
    struct copper_header buf;
    char *p;			/* head pointer for reading */
    int size;			/* remain size to read */
} event_header;

static struct {
    struct copper_footer buf;
    char *p;			/* head pointer for reading */
    int size;			/* remain size to read */
} event_footer;

unsigned int chksum_xor(unsigned int *start, int wordlen)
{
    unsigned int ret = 0;
    // int i;
    while (wordlen--) {
	ret ^= *(start++);
    }
    return ret;
}

char *fill_header(void)
{
    event_header.buf.magic = COPPER_DRIVER_HEADER_MAGIC;
    event_header.buf.event_number = run_state.event_number;
    event_header.buf.subsys = module_state.subsys;
    event_header.buf.crate = module_state.crate;
    event_header.buf.slot = module_state.slot;
    event_header.buf.ttrx[0] = 0;
    event_header.buf.ttrx[1] = 0;
    event_header.p = (char *) &(event_header.buf);
    event_header.size = sizeof(event_header.buf);

    return event_header.p;
}

void fill_footer(void)
{
    event_footer.buf.chksum_xor = 0x12345678;
    event_footer.buf.magic = COPPER_DRIVER_FOOTER_MAGIC;
    event_footer.p = (char *) &(event_footer.buf);
    event_footer.size = sizeof(event_footer.buf);
}

static void clear_run_state(void)
{
    run_state.event_number = 0;
    run_state.read_state = 0;
    run_state.dma_count = 0;
    run_state.dma_try_count = 0;
    run_state.end_run = 0;
    run_state.fifo_full = 0;
    run_state.too_much = 0;
}

static struct {
    unsigned int regaddr;
    unsigned char *reg;
    int regsize;
    struct proc_dir_entry *pentry;
} copper;

int possibly_evq_has_new_event;

static void abort_dma(void);
static void clear_dma_buffer(void);
static void clear_dma_error(void);
static void enable_pci_intr(void);
static void enable_local_intr(void);
static void enable_copper_intr(void);
static void disable_copper_intr(void);
static int start_dma(void);
static void clear_next_dma_size(void);
static void config_event_fifo_depth(void);
static void clear_copper_fifo(void);
#if 0

static inline void cli(void)
{
//        local_irq_disable();
}
static inline void sti(void)
{
//        local_irq_enable();
}
static inline void save_flags(unsigned long *x)
{
//        local_save_flags(*x);
}

#define save_flags(x) save_flags(&x)
static inline void restore_flags(unsigned long x)
{
//        local_irq_restore(x);
}

static inline void save_and_cli(unsigned long *x)
{
//        local_irq_save(*x);
}
#endif

static void config_event_fifo_depth(void)
{
    int full = (DMABUFSIZE / (4 * sizeof(int))) * 3 / 4;	/* 4 finesse */
    int quant = full / 2;

    writel(quant, COPPERREG(CONF_WA_FF));
    writel(quant, COPPERREG(CONF_WB_FF));
    writel(quant, COPPERREG(CONF_WC_FF));
    writel(quant, COPPERREG(CONF_WD_FF));

    writel(full, COPPERREG(CONF_WA_AF));
    writel(full, COPPERREG(CONF_WB_AF));
    writel(full, COPPERREG(CONF_WC_AF));
    writel(full, COPPERREG(CONF_WD_AF));


#if 0
    writel(100, COPPERREG(LEF_WA_FF));
    writel(100, COPPERREG(LEF_WB_FF));
    writel(100, COPPERREG(LEF_WC_FF));
    writel(100, COPPERREG(LEF_WD_FF));
#endif

    int lef_wa_ff = readl( COPPERREG(LEF_WA_FF));
    int lef_wb_ff = readl( COPPERREG(LEF_WB_FF));
    int lef_wc_ff = readl( COPPERREG(LEF_WC_FF));
    int lef_wd_ff = readl( COPPERREG(LEF_WD_FF));

     printk("<1>LEF_WX_FF: %d %d %d %d\n",
	     lef_wa_ff,
	     lef_wb_ff,
	     lef_wc_ff,
	     lef_wd_ff
	     );

    writel(256, COPPERREG(LEF_WA_AF));
    writel(256, COPPERREG(LEF_WB_AF));
    writel(256, COPPERREG(LEF_WC_AF));
    writel(256, COPPERREG(LEF_WD_AF));
}

static void reset_finesse(void)
{
    writel(0x10, COPPERREG(FF_RST));
    writel(0, COPPERREG(FF_RST));
}

static void clear_copper_fifo(void)
{
    writel(0x0F, COPPERREG(FF_RST));
    writel(0, COPPERREG(FF_RST));
}


/* static inline double GetTimeSec(){ */
/*   getnstimeofday( &tv); */
/*   return ( tv.tv_sec + tv.tv_nsec*1.e-9 ); */
/* } */


static void cprio_init_run(void)
{
    plx9054_cnt = 0;
    actual_intr_handl_cnt = 0;
enqueue_cnt = 0;
localint_cnt = 0;


    clear_copper_fifo();
    writel(1, COPPERREG(LEF_READSEL));	/* use LEF_TOTAL instead of LEF_AB,CD */
    writel(0, COPPERREG(FF_RW));	/* forbid single data fifo access */
    enable_copper_intr();
    clear_run_state();
    clear_event_queue();
    clear_dma_buffer();
    clear_dma_error();
}

static int
copper_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
	     unsigned long arg)
{


    int regno = _IOC_NR(cmd);
    int ret;
    unsigned int val;
    int cmd_type = _IOC_TYPE(cmd);

    if (cmd_type == COPPER_MAGIC2) {
	regno += 0x100;
    }

    if ((cmd_type != COPPER_MAGIC) && (cmd_type != COPPER_MAGIC2)) {
	return -EINVAL;
    } else if (_IOC_DIR(cmd) & _IOC_READ) {
	val = readl(copper.reg + regno);
	DBG(IOCTL, "<1> copper read ioctl regno=%x val=%x\n", regno, val);
	if (access_ok(VERIFY_WRITE, (void *) arg, sizeof(unsigned int))) {
	    ret =
		copy_to_user((unsigned int *) arg, &val,
			     sizeof(unsigned int));
	    if (ret != 0)
		return -EINVAL;
	} else {
	    printk("copper_ioctl: invalid read pointer %08lx\n", arg);
	    return -EINVAL;
	}
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
	if (access_ok(VERIFY_READ, (void *) arg, sizeof(unsigned int))) {
	    writel(*(unsigned int *) arg, copper.reg + regno);
	} else {
	    printk("copper_ioctl: invalid write pointer %08lx\n", arg);
	    return -EINVAL;
	}
    } else if (_IOC_DIR(cmd) == _IOC_NONE) {
	// unsigned long flags;
	switch (cmd) {
	case CPRIO_FORCE_DMA:
	    down(&sem_critical);	// save_flags(flags); cli();
	    start_dma();
	    up(&sem_critical);	// restore_flags(flags);
	    break;
	case CPRIO_END_RUN:
	    run_state.end_run = 1;
	    break;
	case CPRIO_RESET_FINESSE:
	    reset_finesse();
	    break;
	case CPRIO_RESET_COPPER:
	    clear_copper_fifo();
	    writel(1, COPPERREG(LEF_READSEL));	/* use LEF_TOTAL instead of LEF_AB,CD */
	    writel(0, COPPERREG(FF_RW));	/* forbid single data fifo access */
	    break;
	case CPRIO_INIT_RUN:
	    cprio_init_run();
	    break;
	case CPRIO_SET_SUBSYS:
	    module_state.subsys = arg;
	    break;
	case CPRIO_SET_CRATE:
	    module_state.crate = arg;
	    break;
	case CPRIO_SET_SLOT:
	    module_state.slot = arg;
	    break;
	case CPRIO_ENABLE_INTERRUPT:
	    enable_copper_intr();
	    enable_pci_intr();
	    enable_local_intr();
	    break;
	case CPRIO_DISABLE_INTERRUPT:
	    disable_copper_intr();
	    disable_pci_intr();
	    disable_local_intr();
	    break;
	}
    }
    return 0;
}

static int copper_open(struct inode *inode, struct file *filp)
{
    //MOD_INC_USE_COUNT;
    //clear_copper_fifo();

    if (MINOR(inode->i_rdev) != 0)
	return 0;

    disable_copper_intr();
    disable_pci_intr();
    disable_local_intr();
    abort_dma();

    cprio_init_run();

    config_event_fifo_depth();
    enable_pci_intr();
    enable_local_intr();
    return 0;
}

static int copper_close(struct inode *inode, struct file *filp)
{

    if (MINOR(inode->i_rdev) != 0)
	return 0;
    //MOD_DEC_USE_COUNT;
    disable_copper_intr();
    disable_local_intr();
    return 0;
}

typedef struct {
    unsigned char *vmaddr;
    //unsigned char * pciaddr;
    unsigned int pciaddr;
    int wpos;
    int in_use;			/* # of events in this buffer */
} dma_buffer;

static struct {
    unsigned int bar[6];	/* BAR registers */
    unsigned int regaddr;	/* base address(PCI) of PLX9054 register */
    unsigned char *reg;		/* base address(VM)  of PLX9054 register */
    int rbuf;			/* reading buffer number */
    int wbuf;			/* writing buffer number */
    dma_buffer dmabuf[NDMABUF];
    int regsize;		/* register space size */
    unsigned char irq_pci_register;	/* IRQ number */
    unsigned char irq_pdev;	/* IRQ number */
    struct proc_dir_entry *pentry;	/* /proc/plx9054 entry */
    //volatile plx9054_intcsr * intcsr;         /* pointer to INTCSR */
    int last_dma_size;
    int next_dma_size;
    int dma_active;
    int buffer_full;
    int fatal_buffer_full;
    int lost_dma_intr;
    int found_lost_dma_intr;
    int intr;
} plx9054;

#define WBUF    (plx9054.dmabuf[plx9054.wbuf])
#define RBUF    (plx9054.dmabuf[plx9054.rbuf])

void clear_dma_buffer()
{
    int i;
    for (i = 0; i < NDMABUF; i++) {
	plx9054.dmabuf[i].wpos = 0;
	plx9054.dmabuf[i].in_use = 0;
    }
    plx9054.rbuf = 0;
    plx9054.wbuf = 0;
    plx9054.buffer_full = 0;
    plx9054.lost_dma_intr = 0;
    plx9054.found_lost_dma_intr = 0;
    plx9054.fatal_buffer_full = 0;
    clear_next_dma_size();
    possibly_evq_has_new_event = 1;
    wake_up_interruptible(&waitq);
}

#define DECL_PLX9054_READER(n) \
static int \
read_plx9054_##n(char * buf, char ** start, off_t offset, int buf_len, int * eof, void * data) \
{ return snprintf(buf, buf_len, "%08x\n", *((volatile unsigned int *)(plx9054.reg + PLX9054_##n))); }

DECL_PLX9054_READER(INTCSR)
#define ADD_PLX9054_READER(n) \
pentry_file = create_proc_entry(#n, 0, plx9054.pentry);  \
if (pentry_file) pentry_file->read_proc = read_plx9054_##n
#define DEL_PLX9054_READER(n) remove_proc_entry(#n, plx9054.pentry)
void abort_dma()
{
    /* PCI 9054 Data Book v2.1 section 3.5.4.1 */
    int i;
    plx9054_dmacsr dmacsr;

    dmacsr.val = readl(PLX9054REG(DMACSR0));
    if (dmacsr.bit.dma0_done) {
	plx9054.dma_active = 0;
	return;
    }

    printk("<1>DMA is active, aborting...\n");

    /* abort DMA0 */
    dmacsr.val = 0;
    dmacsr.bit.dma0_abort = 1;
    writel(dmacsr.val, PLX9054REG(DMACSR0));

    /* when DMA0 is aborted succesfully, done bit becomes 1 */

    for (i = 0; i < 100; i++) {
	dmacsr.val = readl(PLX9054REG(DMACSR0));
	DBG(DMA_REQUEST, "<1>%d DMACSR0 %08x\n", i, dmacsr.val);
	if (dmacsr.bit.dma0_done)
	    break;
    }

    plx9054.dma_active = 0;
}

void clear_dma_error()
{
    plx9054_intcsr intcsr;

    //PLX9054REG(DMACSR0) = 4;          /* abort DMA0 */
    writel(8, PLX9054REG(DMACSR0));	/* clear DMA CSR0  */

    intcsr.val = readl(PLX9054REG(INTCSR));
    intcsr.bit.pci_error = 1;	/* clear PCI error */
    writel(intcsr.val, PLX9054REG(INTCSR));

    plx9054.dma_active = 0;
}

static int
plx9054_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
	      unsigned long arg)
{
    unsigned int val;
    int regno = _IOC_NR(cmd);
    int ret;

    if (_IOC_TYPE(cmd) != PLX9054_MAGIC) {
	return -EINVAL;
    } else if (_IOC_DIR(cmd) & _IOC_READ) {
	val = readl(plx9054.reg + regno);
	ret =
	    copy_to_user((unsigned int *) arg, &val, sizeof(unsigned int));
	if (ret != 0)
	    return -EINVAL;
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
	writel(*(unsigned int *) arg, plx9054.reg + regno);
    } else if (_IOC_DIR(cmd) == _IOC_NONE) {
	switch (cmd) {
	case PLXIOCLR_DMA:
	    clear_event_queue();
	    clear_dma_buffer();
	    clear_dma_error();
	    break;
	}
    }
    return 0;
}

static int plx9054_open(struct inode *inode, struct file *filp)
{
    //MOD_INC_USE_COUNT;
    return 0;
}

static int plx9054_close(struct inode *inode, struct file *filp)
{
    //MOD_DEC_USE_COUNT;
    return 0;
}

static void enable_pci_intr()
{
    plx9054_intcsr intcsr;
    intcsr.val = readl(PLX9054REG(INTCSR));
    intcsr.bit.pci_int_enable = 1;
    writel(intcsr.val, PLX9054REG(INTCSR));
}

static void disable_pci_intr()
{
    plx9054_intcsr intcsr;
    intcsr.val = readl(PLX9054REG(INTCSR));
    intcsr.bit.pci_int_enable = 0;
    writel(intcsr.val, PLX9054REG(INTCSR));
}

static void enable_local_intr()
{
    plx9054_intcsr intcsr;
    intcsr.val = readl(PLX9054REG(INTCSR));
    intcsr.bit.local_int_input_enable = 1;
    writel(intcsr.val, PLX9054REG(INTCSR));
    DBG(INTR, "<1> enable_local_intr set INTCSR = %08x\n", intcsr.val);
}

static void disable_local_intr()
{
    plx9054_intcsr intcsr;
    intcsr.val = readl(PLX9054REG(INTCSR));
    intcsr.bit.local_int_input_enable = 0;
    writel(intcsr.val, PLX9054REG(INTCSR));
    DBG(INTR, "<1> disable_local_intr set INTCSR = %08x\n", intcsr.val);
}

static void enable_copper_intr()
{
    /*1(event fifo almost full)
       2(length fifo almost full)
       4(length fifo threshold)        */
    writel(0x7, COPPERREG(INT_MASK));
}

static void disable_copper_intr()
{
    writel(0, COPPERREG(INT_MASK));
}

void
actual_interrupt_handler(struct work_struct * w)
{
struct timespec tv;
 if( ( debug_flag & DEBUG_YAMADA ) && plx9054_cnt >= 50000 && plx9054_cnt < 50200 ){
    getnstimeofday( &tv);  
    printk("<1>" "actual Start %d %d %d %d %d\n", 
	   actual_intr_handl_cnt,enqueue_cnt,localint_cnt, tv.tv_sec, tv.tv_nsec);
  }
  

    plx9054_intcsr status;
    int dma0_done;
    int local_int;

    down(&irq_handler);

    status.val = readl(PLX9054REG(INTCSR));
    dma0_done = status.bit.dma0_int_active;
    local_int = status.bit.local_int_active;

    {
	if (plx9054.lost_dma_intr < 10 &&
	    plx9054.dma_active &&
	    !dma0_done && (0x10 & readl(PLX9054REG(DMACSR0)))) {

	    DBG(DMA_LOST, "<1>maybe lost DMA done interrupt (dmac=%d)\n",
		   run_state.dma_count);
	    plx9054.lost_dma_intr++;
	}
    }

    if (dma0_done) {
	// unsigned long flags;

	if (plx9054.lost_dma_intr) {
	    DBG(DMA_LOST, "<1>found lost DMA done interrupt (dmac=%d)\n",
		   run_state.dma_count);
	    plx9054.found_lost_dma_intr ++;
	}

	down(&sem_critical);	// save_flags(flags); cli();

	plx9054.dma_active = 0;
	DBG(INTR, "<1>" "found DMA0 active(intcsr=%08x,dmacsr=%08x)\n",
	    status.val, readl(PLX9054REG(DMACSR0)));
	WBUF.in_use +=
	    enqueue_events((unsigned long *) (WBUF.vmaddr + WBUF.wpos),
			   plx9054.last_dma_size / sizeof(int),
			   plx9054.wbuf);
	WBUF.wpos += plx9054.last_dma_size;

	enqueue_cnt++;

	DBG(DMA, "<1>" "event_count = %d\n", count_event_queue());

	clear_dma_error();
	enable_local_intr();
	enable_pci_intr();

	if (debug_flag & DEBUG_DATA) {
	    int lef_sta = readl(COPPERREG(LEF_STA));
	    int ff_sta = readl(COPPERREG(FF_STA));
	    if ((lef_sta & 0xFFFF) == 0x1111) {
		printk("<1>" "LEF_STA=%x FF_STA=%x\n", lef_sta, ff_sta);
	    }
	}

	up(&sem_critical);	// restore_flags(flags);

	possibly_evq_has_new_event = 1;
	wake_up_interruptible(&waitq);

	//printk("<1>" "intcsr = %x\n", *plx9054.intcsr);
    }
    if (local_int) {
	// unsigned long flags;
	down(&sem_critical);	// save_flags(flags); cli();
	DBG(INTR, "<1>" "found local input active(%x)\n", status.val);
	disable_local_intr();
	//printk("<1>" "intcsr = %x\n", *plx9054.intcsr);
	if (plx9054.dma_active) {
	    DBG(DMA_REQUEST,
		"<1>found local input active(%x) but DMA is active\n", status.val);
	}
	if (readl(COPPERREG(INT_STA)) && !plx9054.dma_active) {
	    if (0 == start_dma()) {
		/* indicates DMA buffer full */
	      //		printk("<1>" "Ohh, DMA buffer is full.\n");
		if (!plx9054.buffer_full) {
		    printk("<1>failed allocate enough space on DMA.\n");
		}
		plx9054.buffer_full ++;
#if 0
		enable_pci_intr();
		enable_local_intr();
#endif
		possibly_evq_has_new_event = 1;
		//		printk("<1>" "Bef wakeup\n");
		wake_up_interruptible(&waitq);
		//		printk("<1>" "Aft wakeup.\n");
	    }

	}
	up(&sem_critical);	// restore_flags(flags);
	localint_cnt++;
    }

    up(&irq_handler);

    if( ( debug_flag & DEBUG_YAMADA ) && plx9054_cnt >= 50000 && plx9054_cnt < 50200 ){
      getnstimeofday( &tv);    
      printk("<1>" "actual Stopp %d %d %d %d %d\n", 
	     actual_intr_handl_cnt,enqueue_cnt,localint_cnt, tv.tv_sec, tv.tv_nsec);
    }
    actual_intr_handl_cnt++;

}

irqreturn_t plx9054_intr(int irq, void *dev_id, struct pt_regs *regs)
{

struct timespec tv;
    if( debug_flag & DEBUG_YAMADA ){
      getnstimeofday( &tv);    
      printk("<1>" "pl9054 Start %d %d %d\n", plx9054_cnt, tv.tv_sec, tv.tv_nsec);
    }

    int may_bogus = 0;
    plx9054_intcsr status;
    status.val = readl(PLX9054REG(INTCSR));

    if (! status.bit.pci_int_enable)
	return IRQ_NONE;

    if (status.bit.local_int_active || status.bit.dma0_int_active) {
	DBG(INTR, "<1>PLX9054 ring! (%x)\n", plx9054.intr);
    } else {
	DBG(INTR, "<1>No PLX9054 active interrupt source (%x)\n", plx9054.intr);
	may_bogus = 1;
    }

    plx9054.intr++;
    disable_pci_intr();
    queue_work(plx9054_workq, &work);

    if( debug_flag & DEBUG_YAMADA ){
      //    if( ( debug_flag & DEBUG_YAMADA ) && plx9054_cnt >= 50000 && plx9054_cnt < 50200 ){
      getnstimeofday( &tv);   
      printk("<1>" "pl9054 Stopp %d %d %d\n", plx9054_cnt, tv.tv_sec, tv.tv_nsec); 
    }
  plx9054_cnt++;

    return IRQ_HANDLED;
}

void create_plx9054_proc(void)
{
    struct proc_dir_entry *pentry_file;
    plx9054.pentry = create_proc_entry("plx9054", S_IFDIR, 0);
    ADD_PLX9054_READER(INTCSR);
}

void remove_plx9054_proc(void)
{
    DEL_PLX9054_READER(INTCSR);
    remove_proc_entry("plx9054", 0);
}

#define DECL_COPPER_READER(n) \
static int \
read_copper_##n(char * buf, char ** start, off_t offset, int buf_len, int * eof, void * data) \
{ \
    return snprintf(buf, buf_len, "%08x\n", *((volatile unsigned int *)(copper.reg + COPPER_##n))); \
}

#define ADD_COPPER_READER(n) \
pentry_file = create_proc_entry(#n, 0, copper.pentry);  \
if (pentry_file) pentry_file->read_proc = read_copper_##n

#define DEL_COPPER_READER(n) remove_proc_entry(#n, copper.pentry)

DECL_COPPER_READER(FF_STA)
    DECL_COPPER_READER(FINESSE_STA)
    DECL_COPPER_READER(LEF_READSEL)
    DECL_COPPER_READER(LEF_TOTAL)
    DECL_COPPER_READER(LEF_AB)
    DECL_COPPER_READER(LEF_CD)
    DECL_COPPER_READER(LEF_STA)

    DECL_COPPER_READER(WEA_COUNTER)
    DECL_COPPER_READER(WEB_COUNTER)
    DECL_COPPER_READER(WEC_COUNTER)
    DECL_COPPER_READER(WED_COUNTER)

    DECL_COPPER_READER(CONF_WA_AF)
    DECL_COPPER_READER(CONF_WB_AF)
    DECL_COPPER_READER(CONF_WC_AF)
    DECL_COPPER_READER(CONF_WD_AF)

    DECL_COPPER_READER(CONF_WA_FF)
    DECL_COPPER_READER(CONF_WB_FF)
    DECL_COPPER_READER(CONF_WC_FF)
    DECL_COPPER_READER(CONF_WD_FF)

    DECL_COPPER_READER(LEF_WA_FF)
    DECL_COPPER_READER(LEF_WB_FF)
    DECL_COPPER_READER(LEF_WC_FF)
    DECL_COPPER_READER(LEF_WD_FF)

    DECL_COPPER_READER(LEF_WA_AF)
    DECL_COPPER_READER(LEF_WB_AF)
    DECL_COPPER_READER(LEF_WC_AF)
    DECL_COPPER_READER(LEF_WD_AF)

    DECL_COPPER_READER(LWRA_DMA)
    DECL_COPPER_READER(LWRB_DMA)
    DECL_COPPER_READER(LWRC_DMA)
    DECL_COPPER_READER(LWRD_DMA)

static int
write_copper_register(
	int register_offset,
	struct file *pfile, const char *buf,
	unsigned long input_length, void *data)
{
    int val;
    int ret;
    int copylen;

    static char mybuf[64];
    static struct file *pfilePrev = NULL;
    static int offset;

    if (pfile != pfilePrev) {
	offset = 0;
    }

    copylen = input_length;
    if (offset + input_length > 63)
	copylen = 63 - offset;

    ret = copy_from_user(mybuf + offset, buf, copylen);
    if (ret != 0)
	return -EINVAL;
    offset += copylen;
    mybuf[offset] = 0;
    val = simple_strtol(mybuf, 0, 0);
    printk("<1> val = %d\n", val);

    writel(val, register_offset);

    pfilePrev = pfile;
    return (input_length);
}

static int
write_copper_FINESSE_STA(struct file *pfile, const char *buf,
			 unsigned long input_length, void *data)
{
    return write_copper_register(COPPERREG(FINESSE_STA), pfile, buf, input_length, data);
}

static int
write_copper_LEF_WA_FF(struct file *pfile, const char *buf,
			 unsigned long input_length, void *data)
{
    return write_copper_register(COPPERREG(LEF_WA_FF), pfile, buf, input_length, data);
}

static int
write_copper_LEF_WB_FF(struct file *pfile, const char *buf,
			 unsigned long input_length, void *data)
{
    return write_copper_register(COPPERREG(LEF_WB_FF), pfile, buf, input_length, data);
}

static int
write_copper_LEF_WC_FF(struct file *pfile, const char *buf,
			 unsigned long input_length, void *data)
{
    return write_copper_register(COPPERREG(LEF_WC_FF), pfile, buf, input_length, data);
}

static int
write_copper_LEF_WD_FF(struct file *pfile, const char *buf,
			 unsigned long input_length, void *data)
{
    return write_copper_register(COPPERREG(LEF_WD_FF), pfile, buf, input_length, data);
}

static int fetch_next_dma_size(void)
{
    int lef_sta = readl(COPPERREG(LEF_STA));
    int finesse_sta = readl(COPPERREG(FINESSE_STA));
    int mask = 0;
    int result;

    if (finesse_sta & 1)
	mask |= 0x0001;
    if (finesse_sta & 2)
	mask |= 0x0010;
    if (finesse_sta & 4)
	mask |= 0x0100;
    if (finesse_sta & 8)
	mask |= 0x1000;

    if (plx9054.next_dma_size == -1 && (result = (lef_sta & mask)) != 0) {
	printk("<1>no event is available"
	       " (lef_sta=0x%04x&mask=0x%04x = 0x%04x)\n", lef_sta, mask,
	       result);
    }

    if (plx9054.next_dma_size == -1) {
	plx9054.next_dma_size = readl(COPPERREG(LEF_TOTAL)) * 4;
    }
    return plx9054.next_dma_size;
}

static void clear_next_dma_size()
{
    plx9054.next_dma_size = -1;
}

/*
 * size in byte
 */
static int find_write_buffer(int size)
{
    int next;

    if (size > DMABUFSIZE) {
	DBG(DMABUF, "<1> size %d > bufsize %ld (too large)\n", size,
	    DMABUFSIZE);
	return -1;		/* too large */
    }

    if (DMABUFSIZE - WBUF.wpos > size) {
	return plx9054.wbuf;	/* current buffer has sufficient space */
    }

    next = (plx9054.wbuf + 1) % NDMABUF;

    DBG(DMABUF, "<1>dmac=%d bufno %d->%d (size=%d)\n",
	run_state.dma_count, plx9054.wbuf, next, size);

    if (plx9054.dmabuf[next].in_use == 0) {
	plx9054.wbuf = next;
	plx9054.dmabuf[next].wpos = 0;
	return next;
    }

    DBG(DMABUF, "<1>but it contains data\n");

    /* so, next buffer contains data. it means all buffer are occupied */
    return -1;
}

static int start_dma()
{
struct timespec tv;
 if( ( debug_flag & DEBUG_YAMADA ) && plx9054_cnt >= 50000 && plx9054_cnt < 50200 ){
      getnstimeofday( &tv);    
      printk("<1>" "stadma Start %d %d %d %d\n", run_state.dma_count + 1,	run_state.event_number, tv.tv_sec, tv.tv_nsec);
    }

    int size;
    plx9054_intcsr intcsr;
    dma_buffer *dmabuf;
    int bufno;
    int ff_sta;
    /* debug vars */
    // int wpos0  = WBUF.wpos;

    if (run_state.end_run) {
	printk("<1>start_dma is called, but this run is terminated\n");
	return -1;
    }

    if (plx9054.dma_active) {
	plx9054_dmacsr dmacsr;

	DBG(DMA_REQUEST, "<1>start_dma is called, but DMA is active\n");

	dmacsr.val = readl(PLX9054REG(DMACSR0));
	if (dmacsr.bit.dma0_done) {
	    printk("<1>DMA is marked as active, but DMACSR0 shows DMA was done. start workq\n");
	    disable_pci_intr();
	    abort_dma();
	    queue_work(plx9054_workq, &work);
	}

	return -1;
    }

    run_state.dma_try_count++;

    clear_dma_error();

    //printk("<1>" "set INTCSR = 0x%08x / 0x%08x\n", intcsr.val, readl(PLX9054REG(INTCSR)));

    ff_sta = readl(COPPERREG(FF_STA));
    if (run_state.fifo_full == 0 && (ff_sta & 0x10101010) != 0) {
	printk("<1>FIFO FULL happen, following data will be corrupted (eventno = %d)\n",
		run_state.event_number);
	plx9054.fatal_buffer_full = 1;
    }

    /* find next dma buffer */
    size = fetch_next_dma_size(); /* return value is byte */

    if (run_state.too_much == 0 && size > DMABUFSIZE) {
	printk("<1>FIFO is not yet FULL, but data %d words is too long to DMA. last event#=%d\n",
							size, run_state.event_number);
	run_state.too_much = 1;
    }

    if (size == -1) {
	printk("<1>dmac=%d no event available\n", run_state.dma_count);
	return -1;
    }

    if (size == 0) {
	printk("<1>dmac=%d LEF_TOTAL=0 INT_STA=%x\n",
	       run_state.dma_count, readl(COPPERREG(INT_STA)));
	clear_next_dma_size();
	return -1;
    }

    bufno = find_write_buffer(size);
    if (bufno == -1) {
	plx9054.buffer_full = 1;
	DBG(DMABUF,
	    "<1>dmac=%d DMA buffer full(1) size=%d bufno=%d wpos=%d,%d,%d,%d\n",
	    run_state.dma_count, size, plx9054.wbuf,
	    plx9054.dmabuf[0].wpos, plx9054.dmabuf[1].wpos,
	    plx9054.dmabuf[2].wpos, plx9054.dmabuf[3].wpos);
	return 0;
    }

    plx9054.buffer_full = 0;

    clear_next_dma_size();
    dmabuf = &(plx9054.dmabuf[bufno]);

    DBG(ENQUEUE,
	"<1>#=%d rs=%s buf[%d] wpos=%d size=%d\n",
	run_state.event_number,
	read_state[run_state.read_state], bufno, dmabuf->wpos, size);

    if (dmabuf->wpos + size >= DMABUFSIZE) {
	printk
	    ("<1>DMA buffer full(2) dmac=%d bufno=%d wpos=%d size=%d in_use=%d\n",
	     run_state.dma_count, bufno, dmabuf->wpos, size,
	     dmabuf->in_use);
	return 0;
    } else {

	plx9054.dma_active = 1;

	writel(0x205C3, PLX9054REG(DMAMODE0));
	writel(dmabuf->pciaddr + dmabuf->wpos, PLX9054REG(DMAPADR0));
	writel(COPPER_FF_DMA, PLX9054REG(DMALADR0));
	writel(size, PLX9054REG(DMASIZ0));
	writel(8, PLX9054REG(DMADPR0));	/* local->PCI */
	writel(3, PLX9054REG(DMACSR0));	/* start */

	intcsr.val = readl(PLX9054REG(INTCSR));
	intcsr.bit.dma0_int_enable = 1;
	intcsr.bit.pci_int_enable = 1;
	intcsr.bit.pci_error = 0;
	writel(intcsr.val, PLX9054REG(INTCSR));

	run_state.dma_count ++;

	plx9054.last_dma_size = size;
    }

 if( ( debug_flag & DEBUG_YAMADA ) && plx9054_cnt >= 50000 && plx9054_cnt < 50200 ){
      getnstimeofday( &tv);    
      printk("<1>" "stadma Stopp %d %d %d %d %d\n", run_state.dma_count + 1,	run_state.event_number, tv.tv_sec, tv.tv_nsec ,size);
    }


    return size;
}

static int
write_copper_debug_flag(struct file *pfile, const char *buf,
			unsigned int input_length, void *data)
{
    int copylen;
    int ret;
    static char debug_buf[64];
    static struct file *pfilePrev = NULL;
    static int offset;

    if (pfile != pfilePrev) {
	offset = 0;
    }

    copylen = input_length;
    if (offset + input_length > 63)
	copylen = 63 - offset;

    ret = copy_from_user(debug_buf + offset, buf, copylen);
    if (ret != 0)
	return -EINVAL;
    offset += copylen;
    debug_buf[offset] = 0;
    debug_flag = simple_strtol(debug_buf, 0, 0);
    pfilePrev = pfile;
    return (input_length);
}

static int
read_copper_debug_flag(char *buf, char **start, off_t offset, int buf_len,
		       int *eof, void *data)
{
    *eof = 1;
    return snprintf(buf, buf_len, "%d\n", debug_flag);
}

static int
read_copper_one(char *buf, char **start, off_t offset, int buf_len,
		int *eof, void *data)
{
    int size = start_dma();
    DBG(DMA, "start_dma() returned %d\n", size);
    *eof = 1;
    return 0;
}

static int
read_copper_bufstat(char *buf, char **start, off_t offset, int buf_len,
		    int *eof, void *data)
{
    // int i;
    *eof = 1;
    return snprintf(buf, buf_len,
		    "wpos=%d,%d,%d,%d "
		    "q=%d,%d "
		    "fifo_full=%d too_much=%d "
		    "next_dma_size=%d "
		    "lost_dma_intr/found_lost_dma_intr=%d/%d "
		    "dmac=%d/%d\n",
		    plx9054.dmabuf[0].wpos, plx9054.dmabuf[1].wpos,
		    plx9054.dmabuf[2].wpos, plx9054.dmabuf[3].wpos,
		    count_event_queue(), count_empty_queue(),
		    run_state.fifo_full, run_state.too_much,
		    plx9054.next_dma_size,
		    plx9054.lost_dma_intr, plx9054.found_lost_dma_intr,
		    run_state.dma_count, run_state.dma_try_count
			);
}


static int
read_copper_event_number(char *buf, char **start, off_t offset, int buf_len,
		    int *eof, void *data)
{
    // int i;
    *eof = 1;
    return snprintf(buf, buf_len, "%d\n", run_state.event_number);
}

static unsigned int
copper_poll(struct file *filp, struct poll_table_struct *polltab)
{
    struct event *e;
    unsigned int mask = 0;

    if (MINOR(filp->f_dentry->d_inode->i_rdev) != 0)
	return 0;

    poll_wait(filp, &waitq, polltab);

    e = peek_event();
    if (e)
	mask |= POLLIN | POLLRDNORM;

    return mask;
}

static int
copper_read(struct file *filp, char *ubuf, size_t size, loff_t * ppos)
{

    if (MINOR(filp->f_dentry->d_inode->i_rdev) != 0)
	return 0;


/*     writel(100, COPPERREG(LEF_WA_FF)); */
/*     writel(100, COPPERREG(LEF_WB_FF)); */
/*     writel(100, COPPERREG(LEF_WC_FF)); */
/*     writel(100, COPPERREG(LEF_WD_FF)); */

/*     int lef_wa_ff = readl( COPPERREG(LEF_WA_FF)); */
/*     int lef_wb_ff = readl( COPPERREG(LEF_WB_FF)); */
/*     int lef_wc_ff = readl( COPPERREG(LEF_WC_FF)); */
/*     int lef_wd_ff = readl( COPPERREG(LEF_WD_FF)); */

/*      printk("<1> copper_read : Register value : %d %d %d %d\n", */
/* 	     lef_wa_ff, */
/* 	     lef_wb_ff, */
/* 	     lef_wc_ff, */
/* 	     lef_wd_ff */
/* 	     ); */


    struct event *e;
    // unsigned long flags;
    int result = 0;
    static struct event prev;

    if (!access_ok(VERIFY_READ, (void *) ubuf, size)) {
	printk("<1>access_ok check failed %p - %d\n", ubuf, size);
	return -EINVAL;
    }

    e = peek_event();
    //    if (e == NULL || e->size == 0) {
    while( e == NULL || e->size == 0) {
	if (run_state.end_run) {
	    return 0;
	}

	possibly_evq_has_new_event = 0;
	DBG(DATA, "wait_event_interruptible is called\n");

	if (wait_event_interruptible(waitq, possibly_evq_has_new_event)) {
	    DBG(DATA, "wait_event_interruptible was interrupted\n");
	    return -EINTR;
	}
	DBG(FLOW, "<1>" "step1\n");
	e = peek_event();
    }


    DBG(FLOW, "<1>" "step2\n");

    if (run_state.event_number > 0) {
	if (e->start == prev.start && e->bufno == prev.bufno) {
	    printk("<1> same start buf %d.%p -> %d.%p\n",
		   prev.bufno, prev.start, e->bufno, e->start);
	}
    }

    DBG(FLOW, "<1>" "step3\n");

    DBG(FLOW, "<1>" "e = %p\n", e);

    prev = *e;

    DBG(FLOW, "<1>" "step4\n");

    down(&sem_critical);	// save_flags(flags); cli();


    if (run_state.read_state == 0) {
	unsigned int xor1, xor2;

	DBG(FLOW, "<1>" "step5\n");

	DBG(READ_STATE, "<1>read_state = %s\n",
	    read_state[run_state.read_state]);
	plx9054.rbuf = e->bufno;
	fill_header();
	fill_footer();
	DBG(FLOW, "<1>" "step6\n");

	xor1 =
	    chksum_xor((unsigned int *) event_header.p,
		       sizeof(event_header.buf) / 4);
	xor2 = chksum_xor((unsigned int *) e->start, e->size / 4);

	event_footer.buf.chksum_xor = xor1 ^ xor2;

	run_state.read_state = 1;
	DBG(FLOW, "<1>" "step7\n");
    }

    if (run_state.read_state == 1) {
	int ret;
	DBG(READ_STATE, "<1>read_state = %s\n",
	    read_state[run_state.read_state]);
	DBG(FLOW, "<1>" "step8\n");
	if (event_header.size <= size) {
	    ret = copy_to_user(ubuf, event_header.p, event_header.size);
	    if (ret != 0)
		return -EINVAL;
	    ubuf += event_header.size;
	    size -= event_header.size;
	    result += event_header.size;
	    run_state.read_state = 2;
	} else {
	    ret = copy_to_user(ubuf, event_header.p, size);
	    if (ret != 0)
		return -EINVAL;
	    event_header.p += size;
	    event_header.size -= size;
	    result += size;
	}
	DBG(FLOW, "<1>" "step9\n");
    }

    DBG(FLOW, "<1>" "step10\n");

    if (run_state.read_state == 2) {
	DBG(FLOW, "<1>" "step11\n");
	DBG(READ_STATE, "<1>read_state = %s\n",
	    read_state[run_state.read_state]);

	if (debug_flag & DEBUG_DATA) {
	    int i;
	    unsigned int *lp = (unsigned int *) e->start;
	    for (i = 0; i < 10; i++) {
		printk("<1> data %d %08x\n", i, lp[i]);
	    }
	}
	DBG(FLOW, "<1>" "step12\n");

	if (e->size <= size) {
	    int ret = copy_to_user(ubuf, e->start, e->size);
	    if (ret != 0)
		return -EINVAL;
	    size -= e->size;
	    ubuf += e->size;
	    result += e->size;
	    run_state.read_state = 3;

	    dequeue_event(e);
	    plx9054.dmabuf[e->bufno].in_use -= 1;
	} else {
	    int ret = copy_to_user(ubuf, e->start, size);
	    if (ret != 0)
		return -EINVAL;
	    e->start += size;
	    e->size -= size;
	    result += size;
	}
	DBG(FLOW, "<1>" "step13\n");
    }

    if (run_state.read_state == 3) {
	DBG(FLOW, "<1>" "step14\n");
	DBG(READ_STATE, "<1>read_state = %s\n",
	    read_state[run_state.read_state]);
	if (event_footer.size <= size) {
	    int ret =
		copy_to_user(ubuf, event_footer.p, event_footer.size);
	    if (ret != 0)
		return -EINVAL;
	    ubuf += event_footer.size;
	    size -= event_footer.size;
	    result += event_footer.size;

	    run_state.event_number++;
	    run_state.read_state = 0;
	} else {
	    int ret = copy_to_user(ubuf, event_footer.p, size);
	    if (ret != 0)
		return -EINVAL;
	    event_footer.p += size;
	    event_footer.size -= size;
	    result += size;
	}
    }
    DBG(FLOW, "<1>" "step15\n");

    up(&sem_critical);		// restore_flags(flags);

    DBG(FLOW, "<1>" "step16\n");
    //    DBG( YAMADA, "<1>" "step16\n");

    if (plx9054.buffer_full) {
/*       if( debug_flag & DEBUG_YAMADA ){ */
/* 	printk("<1>" "plx9054 buffer full\n"); */
/*       } */

	down(&sem_critical);
/* 	if( debug_flag & DEBUG_YAMADA ){ */
/* 	  printk("<1>" "Down Semaphore\n"); */
/* 	} */
	if (start_dma() == 0) {
	    DBG(DMABUF,
		"<1>we read event, but there is no space to DMA %d bytes\n",
		plx9054.next_dma_size);
	}
	up(&sem_critical);
/* 	if( debug_flag & DEBUG_YAMADA ){ */
/* 	  printk("<1>" "Up Semaphore\n"); */
/* 	} */

    }
    DBG(FLOW, "<1>" "step17\n");

    return result;
}


static void create_copper_proc(void)
{
    struct proc_dir_entry *pentry_file;
    copper.pentry = create_proc_entry("copper", S_IFDIR, 0);
    ADD_COPPER_READER(FF_STA);
    ADD_COPPER_READER(FINESSE_STA);
    if (pentry_file)
	pentry_file->write_proc = write_copper_FINESSE_STA;

    ADD_COPPER_READER(LEF_READSEL);
    ADD_COPPER_READER(LEF_TOTAL);
    ADD_COPPER_READER(LEF_AB);
    ADD_COPPER_READER(LEF_CD);
    ADD_COPPER_READER(LEF_STA);

    ADD_COPPER_READER(WEA_COUNTER);
    ADD_COPPER_READER(WEB_COUNTER);
    ADD_COPPER_READER(WEC_COUNTER);
    ADD_COPPER_READER(WED_COUNTER);

    ADD_COPPER_READER(LWRA_DMA);
    ADD_COPPER_READER(LWRB_DMA);
    ADD_COPPER_READER(LWRC_DMA);
    ADD_COPPER_READER(LWRD_DMA);

    ADD_COPPER_READER(CONF_WA_AF);
    ADD_COPPER_READER(CONF_WB_AF);
    ADD_COPPER_READER(CONF_WC_AF);
    ADD_COPPER_READER(CONF_WD_AF);

    ADD_COPPER_READER(CONF_WA_FF);
    ADD_COPPER_READER(CONF_WB_FF);
    ADD_COPPER_READER(CONF_WC_FF);
    ADD_COPPER_READER(CONF_WD_FF);

    ADD_COPPER_READER(LEF_WA_FF);
    if (pentry_file)
	pentry_file->write_proc = write_copper_LEF_WA_FF;
    ADD_COPPER_READER(LEF_WB_FF);
    if (pentry_file)
	pentry_file->write_proc = write_copper_LEF_WB_FF;
    ADD_COPPER_READER(LEF_WC_FF);
    if (pentry_file)
	pentry_file->write_proc = write_copper_LEF_WC_FF;
    ADD_COPPER_READER(LEF_WD_FF);
    if (pentry_file)
	pentry_file->write_proc = write_copper_LEF_WD_FF;

    ADD_COPPER_READER(LEF_WA_AF);
    ADD_COPPER_READER(LEF_WB_AF);
    ADD_COPPER_READER(LEF_WC_AF);
    ADD_COPPER_READER(LEF_WD_AF);

    ADD_COPPER_READER(one);
    ADD_COPPER_READER(bufstat);
    ADD_COPPER_READER(event_number);

    pentry_file = create_proc_entry("debug_flag", 0, copper.pentry);
    if (pentry_file) {
	pentry_file->read_proc = read_copper_debug_flag;
	pentry_file->write_proc = write_copper_debug_flag;
    }


}

static void remove_copper_proc(void)
{
    printk("<1> stage1\n");
    DEL_COPPER_READER(FINESSE_STA);
    DEL_COPPER_READER(FF_STA);
    DEL_COPPER_READER(LEF_TOTAL);
    DEL_COPPER_READER(LEF_READSEL);
    DEL_COPPER_READER(LEF_AB);
    DEL_COPPER_READER(LEF_CD);
    DEL_COPPER_READER(LEF_STA);
    DEL_COPPER_READER(WEA_COUNTER);
    DEL_COPPER_READER(WEB_COUNTER);
    DEL_COPPER_READER(WEC_COUNTER);
    DEL_COPPER_READER(WED_COUNTER);

    printk("<1> stage2\n");

    DEL_COPPER_READER(LWRA_DMA);
    DEL_COPPER_READER(LWRB_DMA);
    DEL_COPPER_READER(LWRC_DMA);
    DEL_COPPER_READER(LWRD_DMA);

    DEL_COPPER_READER(LWRA_DMA);
    DEL_COPPER_READER(LWRB_DMA);
    DEL_COPPER_READER(LWRC_DMA);
    DEL_COPPER_READER(LWRD_DMA);

    DEL_COPPER_READER(CONF_WA_AF);
    DEL_COPPER_READER(CONF_WB_AF);
    DEL_COPPER_READER(CONF_WC_AF);
    DEL_COPPER_READER(CONF_WD_AF);

    DEL_COPPER_READER(CONF_WA_FF);
    DEL_COPPER_READER(CONF_WB_FF);
    DEL_COPPER_READER(CONF_WC_FF);
    DEL_COPPER_READER(CONF_WD_FF);

    DEL_COPPER_READER(LEF_WA_FF);
    DEL_COPPER_READER(LEF_WB_FF);
    DEL_COPPER_READER(LEF_WC_FF);
    DEL_COPPER_READER(LEF_WD_FF);

    DEL_COPPER_READER(LEF_WA_AF);
    DEL_COPPER_READER(LEF_WB_AF);
    DEL_COPPER_READER(LEF_WC_AF);
    DEL_COPPER_READER(LEF_WD_AF);

    DEL_COPPER_READER(one);
    DEL_COPPER_READER(bufstat);
    DEL_COPPER_READER(event_number);

    printk("<1> stage3\n");

    remove_proc_entry("debug_flag", copper.pentry);
    printk("<1> stage4\n");
    remove_proc_entry("copper", 0);
    printk("<1> stage5\n");
}

static void create_proc_entries(void)
{
    create_plx9054_proc();
    create_copper_proc();
}

static void remove_proc_entries(void)
{
    printk("<1> removing plx9054 proc entries\n");
    remove_plx9054_proc();
    printk("<1> removing copper proc entries\n");
    remove_copper_proc();
    printk("<1> done\n");
}

static int find_chips(void)
{
    int bus_no;
    int i;
    int found = 0;
    struct pci_dev *pdev;
    // unsigned int addr;
    // unsigned int irq;

    for (bus_no = 1; bus_no < 6; bus_no++) {
	pdev = pci_find_slot(bus_no, PCI_DEVFN(8, 0));	/* COPPER is at X,8,0, TTRX is at X,5,0 */
	if (pdev == NULL) {
	    printk("<1>Couldn't find PLX9054 at bus %d\n", bus_no);
	} else {
	    printk("<1>PLX9054 at bus %d\n", bus_no);
	    found = 1;
	    break;
	}
    }

    if (found == 0)
	return -1;

    printk("<1>PLX9054 found %p\n", pdev);

    {
	/* enable bus master */
	pci_read_config_dword(pdev, PCI_COMMAND, &i);
	pci_write_config_dword(pdev, PCI_COMMAND, i | PCI_COMMAND_MASTER);
	pci_read_config_dword(pdev, PCI_COMMAND, &i);
	printk("<1>PCI_COMMAND %x\n", i);
    }

    for (i = 0; i < 6; i++) {
	pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0 + i * 4,
			      &plx9054.bar[i]);
	if (1) {
	    printk("<1>BAR %d %08x\n", i, plx9054.bar[i]);
	}
    }

    plx9054.regaddr = plx9054.bar[0] & PCI_BASE_ADDRESS_MEM_MASK;
    copper.regaddr = plx9054.bar[2] & PCI_BASE_ADDRESS_MEM_MASK;

    plx9054.regsize = 0x100;
    copper.regsize = 0x200;

    /* access protection from other driver */
    request_mem_region(plx9054.regaddr, plx9054.regsize, "plx9054");
    request_mem_region(copper.regaddr, copper.regsize, "copper");

    plx9054.reg = ioremap(plx9054.regaddr, plx9054.regsize);
    copper.reg = ioremap(copper.regaddr, copper.regsize);

    pci_read_config_byte(pdev, PCI_INTERRUPT_LINE,
			 &plx9054.irq_pci_register);

    plx9054.irq_pdev = pdev->irq;

    printk("<1>PLX9054 pci %08x reg %p\n", plx9054.regaddr, plx9054.reg);
    printk("<1>COPPER  pci %08x reg %p\n", copper.regaddr, copper.reg);
    printk("<1>IRQ %d in PCI register\n", plx9054.irq_pci_register);
    printk("<1>IRQ %d in pdev structure\n", plx9054.irq_pdev);

    {
	printk("<1> PLX9054 %08x\n", *(unsigned int *) (plx9054.reg));
    }

    /* We must validate these values, but now I have no time */

    plx9054.intr = 0;

    if (pdev->irq) {
	int result = request_irq(pdev->irq, plx9054_intr,
				 IRQF_DISABLED /* |IRQF_SHARED */, "plx9054", &plx9054);
	if (result < 0) {
	    printk("<1> plx9054 IRQ request failed\n");
	}
    }

    return 0;
}

static void clean_chips(void)
{
    if (plx9054.reg) {
	release_mem_region(plx9054.regaddr, plx9054.regsize);
	iounmap(plx9054.reg);
    }

    if (copper.reg) {
	release_mem_region(copper.regaddr, copper.regsize);
	iounmap(copper.reg);
    }

    if (plx9054.irq_pdev)
	free_irq(plx9054.irq_pdev, &plx9054);
}

static void allocate_buffer(void)
{
    unsigned char *p;
    int i;

    for (i = 0; i < NDMABUF; i++) {
	p = (unsigned char
	     *) (__get_free_pages(GFP_KERNEL, DMA_PAGES));
	plx9054.dmabuf[i].in_use = 0;
	plx9054.dmabuf[i].wpos = 0;
	plx9054.dmabuf[i].vmaddr = p;
	plx9054.dmabuf[i].pciaddr = virt_to_bus(p);
#if 0
	printk("<1> buffer[%d] = %08x(vm) / %08x(pci)\n",
	       i, p, plx9054.dmabuf[i].pciaddr);
#endif
	if (p == NULL) {
	    printk("<1> buffer allocation failed..\n");
	}
    }
    plx9054.wbuf = 0;
    plx9054.rbuf = 0;

    /* clear_dma_buffer is not needed. */
}

static void free_buffer(void)
{
    int i;
    for (i = 0; i < NDMABUF; i++) {
	if (plx9054.dmabuf[i].vmaddr)
	    free_pages((int) (plx9054.dmabuf[i].vmaddr), DMA_PAGES);
    }
}

static struct file_operations copper_fops = {
  owner:THIS_MODULE,
  ioctl:copper_ioctl,
  open:copper_open,
  release:copper_close,
  read:copper_read,
  poll:copper_poll,
};

static struct file_operations plx9054_fops = {
  owner:THIS_MODULE,
  ioctl:plx9054_ioctl,
  open:plx9054_open,
  release:plx9054_close,
};

int init_module()
{
    int error = 0;
    printk("<1> %s\n", build_id);

    /* find PLX9054 and map registers of PLX9054 and copper in the localbus */
    if (-1 == find_chips()) {
	return -ENOENT;
    }


    printk("<1>" COPPER_DEVICE_NAME " init\n");

    error =
	register_chrdev(COPPER_major, COPPER_DEVICE_NAME, &copper_fops);
    if (error) {
	printk("<1>" COPPER_DEVICE_NAME " register_chrdev error %d\n",
	       error);
    }

    printk("<1>" PLX9054_DEVICE_NAME " init\n");
    error =
	register_chrdev(PLX9054_major, PLX9054_DEVICE_NAME, &plx9054_fops);
    if (error) {
	printk("<1>" PLX9054_DEVICE_NAME " register_chrdev error %d\n",
	       error);
    }

/* allocates buffer for DMA */
    allocate_buffer();
    allocate_event_queue();
    create_proc_entries();

    // init_waitqueue_head(&waitq);

    if (plx9054_workq == NULL)
	plx9054_workq = create_singlethread_workqueue("plx9054");

    return 0;
}

void cleanup_module()
{
    int error = 0;

    printk("<1>" COPPER_DEVICE_NAME " cleanup\n");
    printk("<1>" PLX9054_DEVICE_NAME " cleanup\n");

    if (plx9054_workq)
	destroy_workqueue(plx9054_workq);

    remove_proc_entries();
    free_event_queue();
    free_buffer();		/* free buffers for DMA */
    clean_chips();		/* clean memory map and irq */

    error = unregister_chrdev(PLX9054_major, PLX9054_DEVICE_NAME);
    if (error) {
	printk("<1>" PLX9054_DEVICE_NAME "unregister_chrdev error %d\n",
	       error);
    }

    error = unregister_chrdev(COPPER_major, COPPER_DEVICE_NAME);
    if (error) {
	printk("<1>" COPPER_DEVICE_NAME "unregister_chrdev error %d\n",
	       error);
    }

}
