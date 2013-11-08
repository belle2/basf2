/* cprfin-jig.h */


#ifndef CPRFIN_JIG_H
#define CPRFIN_JIG_H


/********************************************************************************/
/***  JIG command list                                                        ***/
/***  only used to select data pattern                                        ***/
/********************************************************************************/

#define JIGCMD_DATA_EXTERNAL    0
#define JIGCMD_DATA_INCREMENT   1
#define JIGCMD_DATA_RANDOM    2
#define JIGCMD_DATA_CPU_ACCESS    3
#define JIGCMD_SELECT_FIFO0   (0|JIGCMD_DATA_CPU_ACCESS)
#define JIGCMD_SELECT_FIFO1   (1|JIGCMD_DATA_CPU_ACCESS)
#define JIGCMD_SELECT_FIFO2   (2|JIGCMD_DATA_CPU_ACCESS)
#define JIGCMD_SELECT_FIFO3   (3|JIGCMD_DATA_CPU_ACCESS)

/********************************************************************************/
/***  JIG register map                                                       ***/
/********************************************************************************/

#define JIG_LED             0x00000000      // JIG LED Offset Address
#define JIG_DIPSW           0x00000004      // JIG Dip-Switch Offset Address
#define JIG_START           0x00000008      // JIG START Offset Address
#define JIG_CMD             0x0000000C      // JIG Command Offset Address
#define JIG_FIFO0           0x00000010      // JIG Write FIFO High High Offset Address
#define JIG_FIFO1           0x00000014      // JIG Write FIFO High Low Offset Address
#define JIG_FIFO2           0x00000018      // JIG Write FIFO Low  High Offset Address
#define JIG_FIFO3           0x0000001C      // JIG Write FIFO Low  Low & Action Offset Address
#define JIG_OUTD            0x00000020      // JIG Output Data Control Offset Address
#define JIG_REV             0x00000024      // JIG PLD Revision Offset Address
#define JIG_DIPLEDH         0x00000028      // JIG DIP_LEDH  Offset Address
#define JIG_DIPLEDM         0x0000002C      // JIG DIP_LEDH  Offset Address
#define JIG_DIPLEDL         0x00000030      // JIG DIP_LEDH  Offset Address

#define JIG_LEDTEST_ENB     0x0000003C      // LED Test Enable
#define JIG_LED1_TEST       0x00000040      // LED1_TEST Data
#define JIG_LED2_TEST       0x00000044      // LED2_TEST Data
#define JIG_LED3_TEST       0x00000048      // LED3_TEST Data
#define JIG_LED4_TEST       0x0000004C      // LED4_TEST Data
#define JIG_LED5_TEST       0x00000050      // LED5_TEST Data

#define JIG_MODE_EXT        0x00000000      // Select External Data to Finesse Source
#define JIG_MODE_INC        0x01000000      // Select FPGA Increment Data to Finesse Source
#define JIG_MODE_RND        0x02000000      // Select FPGA Random Data to Finesse Source
#define JIG_MODE_CPU        0x03000000      // Select CPU Data to Finesse Source
#define JIG_MODE_FPGA       0x00000000      // Select FPGA Increment/Random Data to Finesse Source

#define JIG_CMD_START       0x01000000      // Start Finesse
#define JIG_CMD_STOP        0x00000000      // Stop Finessse


#define GENERIC_FPGA_VER

#define JIGREG_N_TOTAL_REG  0x1FC
/********************************************************************************/
/***  JIG ioctl                                                              ***/
/********************************************************************************/

#ifndef _LINUX_IOCTL_H
#include <linux/ioctl.h>
#endif


#define CPRFIN_JIG_MAGIC    0xe2

#define FNIOGET_JIG_LED   _IOR(CPRFIN_JIG_MAGIC, JIG_LED,   sizeof(unsigned int))
#define FNIOGET_JIG_DIPSW _IOR(CPRFIN_JIG_MAGIC, JIG_DIPSW, sizeof(unsigned int))
#define FNIOGET_JIG_START _IOR(CPRFIN_JIG_MAGIC, JIG_START, sizeof(unsigned int))
#define FNIOGET_JIG_CMD   _IOR(CPRFIN_JIG_MAGIC, JIG_CMD,   sizeof(unsigned int))
#define FNIOGET_JIG_FIFO0 _IOR(CPRFIN_JIG_MAGIC, JIG_FIFO0, sizeof(unsigned int))
#define FNIOGET_JIG_FIFO1 _IOR(CPRFIN_JIG_MAGIC, JIG_FIFO1, sizeof(unsigned int))
#define FNIOGET_JIG_FIFO2 _IOR(CPRFIN_JIG_MAGIC, JIG_FIFO2, sizeof(unsigned int))
#define FNIOGET_JIG_FIFO3 _IOR(CPRFIN_JIG_MAGIC, JIG_FIFO3, sizeof(unsigned int))
#define FNIOGET_JIG_OUTD  _IOR(CPRFIN_JIG_MAGIC, JIG_OUTD,  sizeof(unsigned int))
#define FNIOGET_JIG_REV   _IOR(CPRFIN_JIG_MAGIC, JIG_REV,   sizeof(unsigned int))
#define FNIOGET_JIG_DIPLEDH _IOR(CPRFIN_JIG_MAGIC, JIG_DIPLEDH, sizeof(unsigned int))
#define FNIOGET_JIG_DIPLEDM _IOR(CPRFIN_JIG_MAGIC, JIG_DIPLEDM, sizeof(unsigned int))
#define FNIOGET_JIG_DIPLEDL _IOR(CPRFIN_JIG_MAGIC, JIG_DIPLEDL, sizeof(unsigned int))
#define FNIOGET_JIG_LEDTEST_ENB _IOR(CPRFIN_JIG_MAGIC, JIG_LEDTEST_ENB, sizeof(unsigned int))
#define FNIOGET_JIG_LED1_TEST _IOR(CPRFIN_JIG_MAGIC, JIG_LED1_TEST, sizeof(unsigned int))
#define FNIOGET_JIG_LED2_TEST _IOR(CPRFIN_JIG_MAGIC, JIG_LED2_TEST, sizeof(unsigned int))
#define FNIOGET_JIG_LED3_TEST _IOR(CPRFIN_JIG_MAGIC, JIG_LED3_TEST, sizeof(unsigned int))
#define FNIOGET_JIG_LED4_TEST _IOR(CPRFIN_JIG_MAGIC, JIG_LED4_TEST, sizeof(unsigned int))
#define FNIOGET_JIG_LED5_TEST _IOR(CPRFIN_JIG_MAGIC, JIG_LED5_TEST, sizeof(unsigned int))

#define FNIOSET_JIG_LED   _IOW(CPRFIN_JIG_MAGIC, JIG_LED,   sizeof(unsigned int))
#define FNIOSET_JIG_DIPSW _IOW(CPRFIN_JIG_MAGIC, JIG_DIPSW, sizeof(unsigned int))
#define FNIOSET_JIG_START _IOW(CPRFIN_JIG_MAGIC, JIG_START, sizeof(unsigned int))
#define FNIOSET_JIG_CMD   _IOW(CPRFIN_JIG_MAGIC, JIG_CMD,   sizeof(unsigned int))
#define FNIOSET_JIG_FIFO0 _IOW(CPRFIN_JIG_MAGIC, JIG_FIFO0, sizeof(unsigned int))
#define FNIOSET_JIG_FIFO1 _IOW(CPRFIN_JIG_MAGIC, JIG_FIFO1, sizeof(unsigned int))
#define FNIOSET_JIG_FIFO2 _IOW(CPRFIN_JIG_MAGIC, JIG_FIFO2, sizeof(unsigned int))
#define FNIOSET_JIG_FIFO3 _IOW(CPRFIN_JIG_MAGIC, JIG_FIFO3, sizeof(unsigned int))
#define FNIOSET_JIG_OUTD  _IOW(CPRFIN_JIG_MAGIC, JIG_OUTD,  sizeof(unsigned int))
#define FNIOSET_JIG_REV   _IOW(CPRFIN_JIG_MAGIC, JIG_REV,   sizeof(unsigned int))
#define FNIOSET_JIG_DIPLEDH _IOW(CPRFIN_JIG_MAGIC, JIG_DIPLEDH, sizeof(unsigned int))
#define FNIOSET_JIG_DIPLEDM _IOW(CPRFIN_JIG_MAGIC, JIG_DIPLEDM, sizeof(unsigned int))
#define FNIOSET_JIG_DIPLEDL _IOW(CPRFIN_JIG_MAGIC, JIG_DIPLEDL, sizeof(unsigned int))
#define FNIOSET_JIG_LEDTEST_ENB _IOW(CPRFIN_JIG_MAGIC, JIG_LEDTEST_ENB, sizeof(unsigned int))
#define FNIOSET_JIG_LED1_TEST _IOW(CPRFIN_JIG_MAGIC, JIG_LED1_TEST, sizeof(unsigned int))
#define FNIOSET_JIG_LED2_TEST _IOW(CPRFIN_JIG_MAGIC, JIG_LED2_TEST, sizeof(unsigned int))
#define FNIOSET_JIG_LED3_TEST _IOW(CPRFIN_JIG_MAGIC, JIG_LED3_TEST, sizeof(unsigned int))
#define FNIOSET_JIG_LED4_TEST _IOW(CPRFIN_JIG_MAGIC, JIG_LED4_TEST, sizeof(unsigned int))
#define FNIOSET_JIG_LED5_TEST _IOW(CPRFIN_JIG_MAGIC, JIG_LED5_TEST, sizeof(unsigned int))

#endif
