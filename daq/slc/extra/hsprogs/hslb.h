/* hslb.h */


#ifndef HSLB_H
#define HSLB_H


#ifdef __KERNEL__


#define DRIVER_NAME "hslb"
#define DRIVER_VERSION_NUMBER 2015041501

MODULE_AUTHOR("M.Nakao IPNS,KEK");
MODULE_DESCRIPTION("cprfin_fngeneric for kernel 2.6.18");
MODULE_LICENSE("GPL");

#endif /* __KERNEL__ */



/********************************************************************************/
/***  FNGENERIC ioctl                                                         ***/
/********************************************************************************/

#include <linux/ioctl.h>

#define HSLB_MAGIC         0xe3
#define HSLB_FEE8_MAGIC    0xe4
#define HSLB_FEE32_MAGIC   0xe5
#define HSLB_FEESTREAM_MAGIC   0xe6

#define HSLBIO_GET(addr)    _IOR(HSLB_MAGIC, addr, sizeof(unsigned int))
#define HSLBIO_SET(addr)    _IOW(HSLB_MAGIC, addr, sizeof(unsigned int))

#define HSLBFEE8_GET(addr)  _IOR(HSLB_FEE8_MAGIC, addr, sizeof(unsigned int))
#define HSLBFEE8_SET(addr)  _IOW(HSLB_FEE8_MAGIC, addr, sizeof(unsigned int))

#define HSLBFEE32_GET(addr) _IOR(HSLB_FEE32_MAGIC, addr, sizeof(unsigned int))
#define HSLBFEE32_SET(addr) _IOW(HSLB_FEE32_MAGIC, addr, sizeof(unsigned long long))

#define HSLBADDR_FIN8MIN  0x00
#define HSLBADDR_FIN8MAX  0x7f
#define HSLBADDR_FIN32MIN 0x80
#define HSLBADDR_FIN32MAX 0xff

#define HSLBADDR_FIN32A   0x6f
#define HSLBADDR_FIN32D   0x6e

#define HSLBADDR_D32A     0x6a
#define HSLBADDR_D32B     0x6b
#define HSLBADDR_D32C     0x6c
#define HSLBADDR_D32D     0x6d
#define HSLBADDR_STAT     0x72
#define HSLBADDR_CSR      0x73
#define HSLBADDR_SERIAL   0x00

#endif /* HSLB_H */

