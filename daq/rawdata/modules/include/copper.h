/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _COPPER_H_
#define _COPPER_H_

#define COPPER_DRIVER_HEADER_MAGIC  0x7FFF0008
#define COPPER_DRIVER_FOOTER_MAGIC  0x7FFF0009

struct copper_header {
  int magic;
  int event_number;
  int subsys;
  int crate;
  int slot;
  unsigned int ttrx[2];
};

struct copper_footer {
  int chksum_xor;
  int magic;
};

#ifndef _LINUX_IOCTL_H
#include <linux/ioctl.h>
#endif

int peek_next_event_size(void);
int shift_next_event_size(void);

/* register positions */
#define COPPER_FF_STA   0x00000000  // COPPER Event FIFO Status
#define COPPER_CONF_WA_AE 0x00000010  // COPPER Event FIFO Ach Almost Empty
#define COPPER_CONF_WB_AE 0x00000014  // COPPER Event FIFO Bch Almost Empty
#define COPPER_CONF_WC_AE 0x00000018  // COPPER Event FIFO Cch Almost Empty
#define COPPER_CONF_WD_AE 0x0000001C  // COPPER Event FIFO Dch Almost Empty
#define COPPER_CONF_WA_FF 0x00000020  // COPPER Event FIFO Ach Defined Quantity
#define COPPER_CONF_WB_FF 0x00000024  // COPPER Event FIFO Bch Defined Quantity
#define COPPER_CONF_WC_FF 0x00000028  // COPPER Event FIFO Cch Defined Quantity
#define COPPER_CONF_WD_FF 0x0000002C  // COPPER Event FIFO Dch Defined Quantity
#define COPPER_CONF_WA_AF 0x00000030  // COPPER Event FIFO Ach Almost Full
#define COPPER_CONF_WB_AF 0x00000034  // COPPER Event FIFO Bch Almost Full
#define COPPER_CONF_WC_AF 0x00000038  // COPPER Event FIFO Cch Almost Full
#define COPPER_CONF_WD_AF 0x0000003C  // COPPER Event FIFO Dch Almost Full
#define COPPER_FF_RST   0x00000050  // COPPER Event FIFO & FINESSE Soft Reset
#define COPPER_FINESSE_STA  0x00000054  // FINESSE Status
#define COPPER_ALMFULL_ENB  0x00000058  // COPPER Almost Full Flag Enable
#define COPPER_VERSION    0x0000005C  // COPPER firmware version
#define COPPER_TRGCM    0x00000060  // COPPER Trigger Command
#define COPPER_TRG_LENG   0x00000064  // COPPER Trigger Interval
#define COPPER_LEF_READSEL  0x00000068  // Length FIFO Read Select
#define COPPER_LEF_TOTAL  0x0000006C  // Length FIFO Total Data
#define COPPER_LEF_AB   0x00000080  // COPPER Length FIFO A&Bch Data
#define COPPER_LEF_CD   0x00000084  // COPPER Length FIFO C&Dch Data
#define COPPER_LEF_STA    0x00000088  // COPPER Length FIFO Status
#define COPPER_LEF_WA_FF  0x00000090  // COPPER Length FIFO Ach Defined Quantity
#define COPPER_LEF_WB_FF  0x00000094  // COPPER Length FIFO Bch Defined Quantity
#define COPPER_LEF_WC_FF  0x00000098  // COPPER Length FIFO Cch Defined Quantity
#define COPPER_LEF_WD_FF  0x0000009C  // COPPER Length FIFO Dch Defined Quantity
#define COPPER_LEF_WA_AF  0x000000A0  // COPPER Length FIFO Ach Almost Full
#define COPPER_LEF_WB_AF  0x000000A4  // COPPER Length FIFO Bch Almost Full
#define COPPER_LEF_WC_AF  0x000000A8  // COPPER Length FIFO Cch Almost Full
#define COPPER_LEF_WD_AF  0x000000AC  // COPPER Length FIFO Dch Almost Full
#define COPPER_INT_STA    0x000000B0  // COPPER Interrupt Status
#define COPPER_INT_MASK   0x000000B4  // COPPER Interrupt Mask
#define COPPER_FF_RW    0x000000B8  // COPPER Event FIFO Single Read/Write Control
#define COPPER_INT_FACTOR 0x000000BC  // COPPER Interruption Factor Reg.
#define COPPER_FF_WA    0x00000100  // COPPER Event FIFO Ach Single Write
#define COPPER_FF_WB    0x00000104  // COPPER Event FIFO Bch Single Write
#define COPPER_FF_WC    0x00000108  // COPPER Event FIFO Cch Single Write
#define COPPER_FF_WD    0x0000010C  // COPPER Event FIFO Dch Single Write
#define COPPER_FF_RA    0x00000110  // COPPER Event FIFO Ach Single Read
#define COPPER_FF_RB    0x00000114  // COPPER Event FIFO Bch Single Read
#define COPPER_FF_RC    0x00000118  // COPPER Event FIFO Cch Single Read
#define COPPER_FF_RD    0x0000011C  // COPPER Event FIFO Dch Single Read
#define COPPER_FF_DMA   0x00400000  // Event FIFO DMA Read Reg.

// 2005/01/17 Append
#define COPPER_EWRP_A   0x00000140  // COPPER Event FIFO Ach Read/Write Pointer
#define COPPER_EWRP_B   0x00000144  // COPPER Event FIFO Bch Read/Write Pointer
#define COPPER_EWRP_C   0x00000148  // COPPER Event FIFO Cch Read/Write Pointer
#define COPPER_EWRP_D   0x0000014C  // COPPER Event FIFO Dch Read/Write Pointer

#define COPPER_WEA_COUNTER  0x00000150  // FINESSE Ach WE Counter
#define COPPER_WEB_COUNTER  0x00000154  // FINESSE Bch WE Counter
#define COPPER_WEC_COUNTER  0x00000158  // FINESSE Cch WE Counter
#define COPPER_WED_COUNTER  0x0000015C  // FINESSE Dch WE Counter

#define COPPER_LWRA_DMA   0x00000160  // COPPER Length FIFO Ach Read/Write Pointer for DMA
#define COPPER_LWRB_DMA   0x00000164  // COPPER Length FIFO Bch Read/Write Pointer for DMA
#define COPPER_LWRC_DMA   0x00000168  // COPPER Length FIFO Cch Read/Write Pointer for DMA
#define COPPER_LWRD_DMA   0x0000016C  // COPPER Length FIFO Dch Read/Write Pointer for DMA

#define COPPER_DMA_TRANS  0x00000170  // COPPER DMA Counter
#define COPPER_DMA_TS_ENB 0x00000174  // COPPER DMA Counter Control

#define COPPER_RW_DEBUG   0x000000FC  // Memory for Debug

/* alias */

#define COPPER_LEF_WA_THR COPPER_LEF_WA_FF
#define COPPER_LEF_WB_THR COPPER_LEF_WB_FF
#define COPPER_LEF_WC_THR COPPER_LEF_WC_FF
#define COPPER_LEF_WD_THR COPPER_LEF_WD_FF

/* ioctl cmds */

#define COPPER_MAGIC    0xdf

#define CPRIOGET_FF_STA   _IOR(COPPER_MAGIC, COPPER_FF_STA, sizeof(unsigned int))
#define CPRIOGET_CONF_WA_AE _IOR(COPPER_MAGIC, COPPER_CONF_WA_AE, sizeof(unsigned int))
#define CPRIOGET_CONF_WB_AE _IOR(COPPER_MAGIC, COPPER_CONF_WB_AE, sizeof(unsigned int))
#define CPRIOGET_CONF_WC_AE _IOR(COPPER_MAGIC, COPPER_CONF_WC_AE, sizeof(unsigned int))
#define CPRIOGET_CONF_WD_AE _IOR(COPPER_MAGIC, COPPER_CONF_WD_AE, sizeof(unsigned int))
#define CPRIOGET_CONF_WA_FF _IOR(COPPER_MAGIC, COPPER_CONF_WA_FF, sizeof(unsigned int))
#define CPRIOGET_CONF_WB_FF _IOR(COPPER_MAGIC, COPPER_CONF_WB_FF, sizeof(unsigned int))
#define CPRIOGET_CONF_WC_FF _IOR(COPPER_MAGIC, COPPER_CONF_WC_FF, sizeof(unsigned int))
#define CPRIOGET_CONF_WD_FF _IOR(COPPER_MAGIC, COPPER_CONF_WD_FF, sizeof(unsigned int))
#define CPRIOGET_CONF_WA_AF _IOR(COPPER_MAGIC, COPPER_CONF_WA_AF, sizeof(unsigned int))
#define CPRIOGET_CONF_WB_AF _IOR(COPPER_MAGIC, COPPER_CONF_WB_AF, sizeof(unsigned int))
#define CPRIOGET_CONF_WC_AF _IOR(COPPER_MAGIC, COPPER_CONF_WC_AF, sizeof(unsigned int))
#define CPRIOGET_CONF_WD_AF _IOR(COPPER_MAGIC, COPPER_CONF_WD_AF, sizeof(unsigned int))
#define CPRIOGET_FF_RST   _IOR(COPPER_MAGIC, COPPER_FF_RST, sizeof(unsigned int))
#define CPRIOGET_FINESSE_STA  _IOR(COPPER_MAGIC, COPPER_FINESSE_STA, sizeof(unsigned int))
#define CPRIOGET_ALMFULL_ENB  _IOR(COPPER_MAGIC, COPPER_ALMFULL_ENB, sizeof(unsigned int))
#define CPRIOGET_VERSION  _IOR(COPPER_MAGIC, COPPER_VERSION, sizeof(unsigned int))
#define CPRIOGET_TRGCM    _IOR(COPPER_MAGIC, COPPER_TRGCM, sizeof(unsigned int))
#define CPRIOGET_TRG_LENG _IOR(COPPER_MAGIC, COPPER_TRG_LENG, sizeof(unsigned int))
#define CPRIOGET_LEF_READSEL  _IOR(COPPER_MAGIC, COPPER_LEF_READSEL, sizeof(unsigned int))
#define CPRIOGET_LEF_TOTAL  _IOR(COPPER_MAGIC, COPPER_LEF_TOTAL, sizeof(unsigned int))
#define CPRIOGET_LEF_AB   _IOR(COPPER_MAGIC, COPPER_LEF_AB, sizeof(unsigned int))
#define CPRIOGET_LEF_CD   _IOR(COPPER_MAGIC, COPPER_LEF_CD, sizeof(unsigned int))
#define CPRIOGET_LEF_STA  _IOR(COPPER_MAGIC, COPPER_LEF_STA, sizeof(unsigned int))
#define CPRIOGET_LEF_WA_FF  _IOR(COPPER_MAGIC, COPPER_LEF_WA_FF, sizeof(unsigned int))
#define CPRIOGET_LEF_WB_FF  _IOR(COPPER_MAGIC, COPPER_LEF_WB_FF, sizeof(unsigned int))
#define CPRIOGET_LEF_WC_FF  _IOR(COPPER_MAGIC, COPPER_LEF_WC_FF, sizeof(unsigned int))
#define CPRIOGET_LEF_WD_FF  _IOR(COPPER_MAGIC, COPPER_LEF_WD_FF, sizeof(unsigned int))
#define CPRIOGET_LEF_WA_AF  _IOR(COPPER_MAGIC, COPPER_LEF_WA_AF, sizeof(unsigned int))
#define CPRIOGET_LEF_WB_AF  _IOR(COPPER_MAGIC, COPPER_LEF_WB_AF, sizeof(unsigned int))
#define CPRIOGET_LEF_WC_AF  _IOR(COPPER_MAGIC, COPPER_LEF_WC_AF, sizeof(unsigned int))
#define CPRIOGET_LEF_WD_AF  _IOR(COPPER_MAGIC, COPPER_LEF_WD_AF, sizeof(unsigned int))
#define CPRIOGET_INT_STA  _IOR(COPPER_MAGIC, COPPER_INT_STA, sizeof(unsigned int))
#define CPRIOGET_INT_MASK _IOR(COPPER_MAGIC, COPPER_INT_MASK, sizeof(unsigned int))
#define CPRIOGET_FF_RW    _IOR(COPPER_MAGIC, COPPER_FF_RW, sizeof(unsigned int))
#define CPRIOGET_INT_FACTOR _IOR(COPPER_MAGIC, COPPER_INT_FACTOR, sizeof(unsigned int))
#define CPRIOGET_FF_WA    _IOR(COPPER_MAGIC, COPPER_FF_WA, sizeof(unsigned int))
#define CPRIOGET_FF_WB    _IOR(COPPER_MAGIC, COPPER_FF_WB, sizeof(unsigned int))
#define CPRIOGET_FF_WC    _IOR(COPPER_MAGIC, COPPER_FF_WC, sizeof(unsigned int))
#define CPRIOGET_FF_WD    _IOR(COPPER_MAGIC, COPPER_FF_WD, sizeof(unsigned int))
#define CPRIOGET_FF_RA    _IOR(COPPER_MAGIC, COPPER_FF_RA, sizeof(unsigned int))
#define CPRIOGET_FF_RB    _IOR(COPPER_MAGIC, COPPER_FF_RB, sizeof(unsigned int))
#define CPRIOGET_FF_RC    _IOR(COPPER_MAGIC, COPPER_FF_RC, sizeof(unsigned int))
#define CPRIOGET_FF_RD    _IOR(COPPER_MAGIC, COPPER_FF_RD, sizeof(unsigned int))
#define CPRIOGET_FF_DMA   _IOR(COPPER_MAGIC, COPPER_FF_DMA, sizeof(unsigned int))

#define CPRIOSET_FF_STA   _IOW(COPPER_MAGIC, COPPER_FF_STA, sizeof(unsigned int))
#define CPRIOSET_CONF_WA_AE _IOW(COPPER_MAGIC, COPPER_CONF_WA_AE, sizeof(unsigned int))
#define CPRIOSET_CONF_WB_AE _IOW(COPPER_MAGIC, COPPER_CONF_WB_AE, sizeof(unsigned int))
#define CPRIOSET_CONF_WC_AE _IOW(COPPER_MAGIC, COPPER_CONF_WC_AE, sizeof(unsigned int))
#define CPRIOSET_CONF_WD_AE _IOW(COPPER_MAGIC, COPPER_CONF_WD_AE, sizeof(unsigned int))
#define CPRIOSET_CONF_WA_FF _IOW(COPPER_MAGIC, COPPER_CONF_WA_FF, sizeof(unsigned int))
#define CPRIOSET_CONF_WB_FF _IOW(COPPER_MAGIC, COPPER_CONF_WB_FF, sizeof(unsigned int))
#define CPRIOSET_CONF_WC_FF _IOW(COPPER_MAGIC, COPPER_CONF_WC_FF, sizeof(unsigned int))
#define CPRIOSET_CONF_WD_FF _IOW(COPPER_MAGIC, COPPER_CONF_WD_FF, sizeof(unsigned int))
#define CPRIOSET_CONF_WA_AF _IOW(COPPER_MAGIC, COPPER_CONF_WA_AF, sizeof(unsigned int))
#define CPRIOSET_CONF_WB_AF _IOW(COPPER_MAGIC, COPPER_CONF_WB_AF, sizeof(unsigned int))
#define CPRIOSET_CONF_WC_AF _IOW(COPPER_MAGIC, COPPER_CONF_WC_AF, sizeof(unsigned int))
#define CPRIOSET_CONF_WD_AF _IOW(COPPER_MAGIC, COPPER_CONF_WD_AF, sizeof(unsigned int))
#define CPRIOSET_FF_RST   _IOW(COPPER_MAGIC, COPPER_FF_RST, sizeof(unsigned int))
#define CPRIOSET_FINESSE_STA  _IOW(COPPER_MAGIC, COPPER_FINESSE_STA, sizeof(unsigned int))
#define CPRIOSET_ALMFULL_ENB  _IOW(COPPER_MAGIC, COPPER_ALMFULL_ENB, sizeof(unsigned int))
#define CPRIOSET_TRGCM    _IOW(COPPER_MAGIC, COPPER_TRGCM, sizeof(unsigned int))
#define CPRIOSET_TRG_LENG _IOW(COPPER_MAGIC, COPPER_TRG_LENG, sizeof(unsigned int))
#define CPRIOSET_LEF_READSEL  _IOW(COPPER_MAGIC, COPPER_LEF_READSEL, sizeof(unsigned int))
#define CPRIOSET_LEF_TOTAL  _IOW(COPPER_MAGIC, COPPER_LEF_TOTAL, sizeof(unsigned int))
#define CPRIOSET_LEF_AB   _IOW(COPPER_MAGIC, COPPER_LEF_AB, sizeof(unsigned int))
#define CPRIOSET_LEF_CD   _IOW(COPPER_MAGIC, COPPER_LEF_CD, sizeof(unsigned int))
#define CPRIOSET_LEF_STA  _IOW(COPPER_MAGIC, COPPER_LEF_STA, sizeof(unsigned int))
#define CPRIOSET_LEF_WA_FF  _IOW(COPPER_MAGIC, COPPER_LEF_WA_FF, sizeof(unsigned int))
#define CPRIOSET_LEF_WB_FF  _IOW(COPPER_MAGIC, COPPER_LEF_WB_FF, sizeof(unsigned int))
#define CPRIOSET_LEF_WC_FF  _IOW(COPPER_MAGIC, COPPER_LEF_WC_FF, sizeof(unsigned int))
#define CPRIOSET_LEF_WD_FF  _IOW(COPPER_MAGIC, COPPER_LEF_WD_FF, sizeof(unsigned int))
#define CPRIOSET_LEF_WA_AF  _IOW(COPPER_MAGIC, COPPER_LEF_WA_AF, sizeof(unsigned int))
#define CPRIOSET_LEF_WB_AF  _IOW(COPPER_MAGIC, COPPER_LEF_WB_AF, sizeof(unsigned int))
#define CPRIOSET_LEF_WC_AF  _IOW(COPPER_MAGIC, COPPER_LEF_WC_AF, sizeof(unsigned int))
#define CPRIOSET_LEF_WD_AF  _IOW(COPPER_MAGIC, COPPER_LEF_WD_AF, sizeof(unsigned int))
#define CPRIOSET_INT_STA  _IOW(COPPER_MAGIC, COPPER_INT_STA, sizeof(unsigned int))
#define CPRIOSET_INT_MASK _IOW(COPPER_MAGIC, COPPER_INT_MASK, sizeof(unsigned int))
#define CPRIOSET_FF_RW    _IOW(COPPER_MAGIC, COPPER_FF_RW, sizeof(unsigned int))
#define CPRIOSET_INT_FACTOR _IOW(COPPER_MAGIC, COPPER_INT_FACTOR, sizeof(unsigned int))
#define CPRIOSET_FF_WA    _IOW(COPPER_MAGIC, COPPER_FF_WA, sizeof(unsigned int))
#define CPRIOSET_FF_WB    _IOW(COPPER_MAGIC, COPPER_FF_WB, sizeof(unsigned int))
#define CPRIOSET_FF_WC    _IOW(COPPER_MAGIC, COPPER_FF_WC, sizeof(unsigned int))
#define CPRIOSET_FF_WD    _IOW(COPPER_MAGIC, COPPER_FF_WD, sizeof(unsigned int))
#define CPRIOSET_FF_RA    _IOW(COPPER_MAGIC, COPPER_FF_RA, sizeof(unsigned int))
#define CPRIOSET_FF_RB    _IOW(COPPER_MAGIC, COPPER_FF_RB, sizeof(unsigned int))
#define CPRIOSET_FF_RC    _IOW(COPPER_MAGIC, COPPER_FF_RC, sizeof(unsigned int))
#define CPRIOSET_FF_RD    _IOW(COPPER_MAGIC, COPPER_FF_RD, sizeof(unsigned int))
#define CPRIOSET_FF_DMA   _IOW(COPPER_MAGIC, COPPER_FF_DMA, sizeof(unsigned int))

/* alias */

#define CPRIOSET_LEF_WA_THR CPRIOSET_LEF_WA_FF
#define CPRIOSET_LEF_WB_THR CPRIOSET_LEF_WB_FF
#define CPRIOSET_LEF_WC_THR CPRIOSET_LEF_WC_FF
#define CPRIOSET_LEF_WD_THR CPRIOSET_LEF_WD_FF

#define CPRIOGET_LEF_WA_THR CPRIOGET_LEF_WA_FF
#define CPRIOGET_LEF_WB_THR CPRIOGET_LEF_WB_FF
#define CPRIOGET_LEF_WC_THR CPRIOGET_LEF_WC_FF
#define CPRIOGET_LEF_WD_THR CPRIOGET_LEF_WD_FF

/* cmds */

#define CPRIO_RESET_FINESSE _IO(COPPER_MAGIC, 1)
#define CPRIO_INIT_RUN    _IO(COPPER_MAGIC, 2)
#define CPRIO_RESET_COPPER  _IO(COPPER_MAGIC, 3)
#define CPRIO_SET_SUBSYS  _IO(COPPER_MAGIC, 4)
#define CPRIO_SET_CRATE   _IO(COPPER_MAGIC, 5)
#define CPRIO_SET_SLOT    _IO(COPPER_MAGIC, 6)
#define CPRIO_FORCE_DMA     _IO(COPPER_MAGIC, 7)
#define CPRIO_END_RUN     _IO(COPPER_MAGIC, 8)

#endif  /* _COPPER_H_ */
