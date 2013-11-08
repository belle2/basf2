/* cprfin_fngeneric.c */


// #define MODULE
// #define __KERNEL__


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "cprfin.h"
#include "cprfin_fngeneric.h"

#define DEBUG_STR ""


/********************************************************************************/
/*** ioctl() implementation                                                   ***/
/********************************************************************************/

static int
cprfinfngeneric_getreg(const int slot, const off_t offset /* offset in unsigned char */, unsigned int *val)
{
	unsigned int *finesse_base_addr;

	if( offset <0 || offset > 0x7f ) return -EINVAL;

	finesse_base_addr = cprfin_finesse_base_addr(slot);
#if 0
	printk("finesse_base_addr = 0x%08x, offset=0x%08x, rawval=0x%08x, macro_access=0x%08x\n", 
	    finesse_base_addr, offset,
	    *(finesse_base_addr + offset),
		cprfin_finesse_reg(slot, 4 * offset));
#endif
	*val = (*(finesse_base_addr + offset)) >> 24;

	return 0;
}


static int
cprfinfngeneric_setreg(const int slot, const off_t offset /* offset in unsigned char */, const unsigned int val)
{
	unsigned int *finesse_base_addr;

	if( offset <0 || offset > 0x7f ) return -EINVAL;

	finesse_base_addr = cprfin_finesse_base_addr(slot);
	*(finesse_base_addr + offset) = val<<24;

	return 0;
}


/********************************************************************************/
/*** hardware initialization/termination                                      ***/
/********************************************************************************/

static void
cprfinfngeneric_hardware_init(const int slot)
{
	/* do nothing */
}


static void
cprfinfngeneric_hardware_term(const int slot)
{
	/* do nothing */
}


/********************************************************************************/
/*** user code I/F                                                            ***/
/********************************************************************************/

static char CPRFIN_opened[4] = { 0, 0, 0, 0 };


static int
cprfinfngeneric_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	const int slot = MINOR(inode->i_rdev);


	if (_IOC_DIR(cmd) &  _IOC_READ ){
		const int error = !access_ok(VERIFY_WRITE, (void*)arg, _IOC_SIZE(cmd));
		if (error) return -EFAULT;
	}


	if( _IOC_TYPE(cmd) != CPRFIN_FNGENERIC_MAGIC ) return -EINVAL;


	{
		int error;
		const unsigned int dir_r = _IOC_DIR(cmd) & _IOC_READ, dir_w = _IOC_DIR(cmd) & _IOC_WRITE;
		const off_t offset = _IOC_NR(cmd);

		if( !dir_r && !dir_w ) return -EINVAL;
		if( offset <0 || offset > 0x7f ) return -EINVAL;

		if( dir_r ) {
			unsigned int val;
			error = cprfinfngeneric_getreg(slot, offset, &val);
#if 0
printk("fngen ioctl read slot=0x%x, offset=0x%x val=0x%x\n", slot, offset, val);
#endif
			if( error ) return error;
			copy_to_user( (unsigned int*)arg, &val, sizeof(unsigned int) );
			return 0;
		} else {
			error = cprfinfngeneric_setreg(slot, offset, (unsigned int)arg);
			if( error ) return error;
			return 0;
		}
	}
}


static int
cprfinfngeneric_open(struct inode *inode, struct file *filp)
{
	const int slot = MINOR(inode->i_rdev);


	if( slot<0 || slot > 4 ) return -ENXIO;

#if 0
	if( CPRFIN_opened[slot] ){
		printk(DRIVER_NAME ": open(): device busy\n");
		return -EBUSY;
	}
#endif

	if( !cprfin_exist_finesse(slot) ){
		printk(DRIVER_NAME ": open(): no such device or address\n");
		return -ENXIO;
	}

	CPRFIN_opened[slot]++;


	return 0;
}


static int
cprfinfngeneric_close(struct inode *inode, struct file *filp)
{
	const int slot = MINOR(inode->i_rdev);


	CPRFIN_opened[slot]--;


	return 0;
}



/********************************************************************************/
/*** kernel I/F                                                               ***/
/********************************************************************************/

struct file_operations cprfinfngeneric_fops = {
	owner:          THIS_MODULE,
	ioctl:          cprfinfngeneric_ioctl,
	open:           cprfinfngeneric_open,
	release:        cprfinfngeneric_close,
};


static unsigned int CPRFIN_major = 210;
static unsigned int CPRFIN_type  = 10;
// MODULE_PARM(CPRFIN_type, "i");
module_param(CPRFIN_type, int, S_IRUGO);


static int CPRFIN_step_state = 0;


int init_modile(void);
void cleanup_module(void);


int
init_module(void)
{
	int error = 0;
	int major;
	int slot;


	error = cprfin_init(CPRFIN_type);
	if( error ){
		goto ERROR;
	}


	CPRFIN_step_state = 1;


	error = register_chrdev(CPRFIN_major, DRIVER_NAME, &cprfinfngeneric_fops);
	if( error ){
		printk(DRIVER_NAME ": cannot get major %d (error=%d)\n", CPRFIN_major, error);
		goto ERROR;
	}


	for( slot=0; slot<4; slot++ ){
		printk("0x7D of %d = %08x\n", slot, cprfin_finesse_reg(slot, 0x7D));
		printk("500 of %d = %08x\n", slot, cprfin_finesse_reg(slot, 500));
		cprfinfngeneric_hardware_init(slot);
	}
	printk(DRIVER_NAME ": cprfinfngeneric_hardware_init() called in init_module()\n");


	CPRFIN_step_state = 2;


	printk(DRIVER_NAME ": driver version=%d%s: ready\n", DRIVER_VERSION_NUMBER, DEBUG_STR);


	return 0;


ERROR:
	cleanup_module();

	return error;
}


void
cleanup_module(void)
{
	if( CPRFIN_step_state>=2 ){
		int slot;

		unregister_chrdev(CPRFIN_major, DRIVER_NAME);

		for( slot=3; slot>=0; slot-- ){
			cprfinfngeneric_hardware_term(slot);
		}
		printk(DRIVER_NAME ": cprfinfngeneric_hardware_term() called in cleanup_module()\n");
	}

	cprfin_term();
}

