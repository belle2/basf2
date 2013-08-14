/* cprfin-jig.c */

#define	MODULE

#include <linux/autoconf.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include "cprfin.h"
#include "cprfin-jig.h"

/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

#define DRIVER_NAME "cprfin-jig"


MODULE_AUTHOR("yamagata CRC,KEK");
MODULE_DESCRIPTION("cprfin");
MODULE_LICENSE("GPL");


/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

static char CPRFIN_opened[4] = { 0, 0, 0, 0 };


static int
cprfinjig_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	const int slot = MINOR(inode->i_rdev);

	if (_IOC_DIR(cmd) &  _IOC_READ ){
		const int error = !access_ok(VERIFY_WRITE, (void*)arg, _IOC_SIZE(cmd));
		if (error) return -EFAULT;
	}


	if       ( _IOC_TYPE(cmd) == CPRFIN_MAGIC     ){

		const unsigned int val = cprfin_ioctl_finesse_generic_info(slot, _IOC_NR(cmd));
		copy_to_user((unsigned int*)arg, &val, sizeof(unsigned int));

	} else if( _IOC_TYPE(cmd) == CPRFIN_JIG_MAGIC ){

		if( _IOC_DIR(cmd) & _IOC_READ || _IOC_DIR(cmd) & _IOC_WRITE ){

			int error;
			unsigned int val, reg[2];
			const unsigned int regid = _IOC_NR(cmd);

			/* cprfin_finesse_reg return only highest 8bit */

			if     ( _IOC_DIR(cmd) & _IOC_READ  ){
				val = cprfin_finesse_reg(slot, regid);
				copy_to_user( (unsigned int*)arg, &val, sizeof(unsigned int) );
				return 0;
			}
			else if( _IOC_DIR(cmd) & _IOC_WRITE ){
				val = *(unsigned int *)arg;
				*(unsigned int*)(cprfin_finesse_base_addr(slot) + regid)
					= (val << 24);
				return 0;
			}

		} else {
#if 0
			/* currently not implemented */
			cprfinjig_command(slot, _IOC_NR(cmd));
			return 0;
#else
			return -EINVAL;
#endif
		}

	} else {

		return -EINVAL;

	}

	return 0;
}


static int
cprfinjig_open(struct inode *inode, struct file *filp)
{
	const int slot = MINOR(inode->i_rdev);

	// MOD_INC_USE_COUNT;

	if( slot<0 || slot > 4 ) return -ENXIO;

	if( CPRFIN_opened[slot] ){
		printk(DRIVER_NAME ": open(): multiple open\n");
		return -EBUSY;
	}

	if( !cprfin_exist_finesse(slot) ){
		printk(DRIVER_NAME ": open(): no such device or address\n");
		return -ENXIO;
	}

	CPRFIN_opened[slot]++;

	return 0;
}


static int
cprfinjig_close(struct inode *inode, struct file *filp)
{
	const int slot = MINOR(inode->i_rdev);

	// MOD_DEC_USE_COUNT;

	CPRFIN_opened[slot]--;

	return 0;
}


struct file_operations cprfinjig_fops = {
	owner:          THIS_MODULE,
	ioctl:          cprfinjig_ioctl,
	open:           cprfinjig_open,
	release:        cprfinjig_close,
};


/********************************************************************************/
/***                                                                          ***/
/********************************************************************************/

static unsigned int CPRFIN_major = 200;
static unsigned int CPRFIN_type  = 0; /* 0 = jig, 5 = AMT3 */
// MODULE_PARM(CPRFIN_type, "i");


int init_modile(void);
void cleanup_module(void);


int
init_module(void)
{
	int error = 0;
	int major;

	error = cprfin_init(CPRFIN_type);
	if( error ) goto ERROR;


	error = register_chrdev(CPRFIN_major, DRIVER_NAME, &cprfinjig_fops);
	if( error ){
		printk(DRIVER_NAME ": cannot get major %d\n", CPRFIN_major);
		CPRFIN_major = -1;
		goto ERROR;
	}

	return 0;


ERROR:
	cleanup_module();

	return error;
}


void
cleanup_module(void)
{
	if( CPRFIN_major>=0 ){
		unregister_chrdev(CPRFIN_major, DRIVER_NAME);
	}

	cprfin_term();
}

