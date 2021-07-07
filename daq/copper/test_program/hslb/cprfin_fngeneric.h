/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* cprfin_fngeneric.h */


#ifndef CPRFIN_FNGENERIC_H
#define CPRFIN_FNGENERIC_H


#ifdef __KERNEL__


#define DRIVER_NAME "cprfin_fngeneric"
#define DRIVER_VERSION_NUMBER 2009042003

MODULE_AUTHOR("T.Higuchi IPNS,KEK");
MODULE_DESCRIPTION("cprfin_fngeneric for kernel 2.4.30-20050518a");
MODULE_LICENSE("GPL");

#endif /* __KERNEL__ */



/********************************************************************************/
/***  FNGENERIC ioctl                                                         ***/
/********************************************************************************/

#include <linux/ioctl.h>

#define CPRFIN_FNGENERIC_MAGIC    0xe3

#define FNGENERICIO_GET(addr)  _IOR(CPRFIN_FNGENERIC_MAGIC, addr, sizeof(unsigned int))
#define FNGENERICIO_SET(addr)  _IOW(CPRFIN_FNGENERIC_MAGIC, addr, sizeof(unsigned int))


#endif /* CPRFIN_FNGENERIC_H */

