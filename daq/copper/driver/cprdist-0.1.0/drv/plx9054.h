#ifndef _PLX9054_H_
#define _PLX9054_H_

#include <linux/ioctl.h>
/*
###############################################################################
# PLX9054 Internal Registers Address Definition
###############################################################################
*/

/*****  Local Configuration Registers  ***************************************/

#define PLX9054_LAS0RR    0x00  // Local Address Space 0 Range Register for PCI-to-Local Bus
#define PLX9054_LAS0BA    0x04  // Local Address Space 0 Local Base Address
#define PLX9054_MARBR   0x08  // Mode/DMA Arbitration
#define PLX9054_BIGEND    0x0C  // Big/Little Endian Descriptor
#define PLX9054_LMISC   0x0D  // Local Miscellaneous Control
#define PLX9054_PROT_AREA 0x0E  // Serial EEPROM Write-Protected Address Boundary
#define PLX9054_EROMRR    0x10  // Expansion ROM Range
#define PLX9054_EROMBA    0x14  // Expansion ROM Local Base Address
#define PLX9054_LBRD0   0x18  // Local Address Space 0/Expansion ROM Bus Region Descriptor
#define PLX9054_DMRR    0x1C  // Local Range Register for PCI Initiator-to-PCI
#define PLX9054_DMLBAM    0x20  // Local Bus Base Address Register for PCI Initiator-to-PCI Memory
#define PLX9054_DMLBAI    0x24  // Local Base Address Register for PCI Initiator-to-PCI I/O Configuration
#define PLX9054_DMPBAM    0x28  // PCI Base Address (Remap) Register for PCI Initiator-to-PCI Memory
#define PLX9054_DMCFGA    0x2C  // PCI Configuration Address Register for PCI Initiator-to-PCI I/O Configuration
#define PLX9054_LAS1RR    0xF0  // Local Address Space 1 Range Register for PCI-to-Local Bus
#define PLX9054_LAS1BA    0xF4  // Local Address Space 1 Local Base Address
#define PLX9054_LBRD1   0xF8  // Local Address Space 1 Bus Region Descriptor
#define PLX9054_DMDAC   0xFC  // PCI Initiator PCI Dual Address Cycle


/*****  Runtime Registers  ***************************************************/

#define PLX9054_MBOX0   0x40  // Mailbox Register 0
#define PLX9054_MBOX1   0x44  // Mailbox Register 1
#define PLX9054_MBOX2   0x48  // Mailbox Register 2
#define PLX9054_MBOX3   0x4C  // Mailbox Register 3
#define PLX9054_MBOX4   0x50  // Mailbox Register 4
#define PLX9054_MBOX5   0x54  // Mailbox Register 5
#define PLX9054_MBOX6   0x58  // Mailbox Register 6
#define PLX9054_MBOX7   0x5C  // Mailbox Register 7
#define PLX9054_P2LDBELL  0x60  // PCI-to-Local Doorbell
#define PLX9054_L2PDBELL  0x64  // Local-to-PCI Doorbell
#define PLX9054_INTCSR    0x68  // Interrupt Control/Status
#define PLX9054_CNTRL   0x6C  // Serial EEPROM Control, PCI Command Codes, User I/O Control, and Init Control
#define PLX9054_PCIHIDR   0x70  // PCI Hardcoded Configuration ID
#define PLX9054_PCIHREV   0x74  // PCI Hardcoded Revision ID

#define PLX9054_INT_DIS   0x0F000000
#define PLX9054_INT_EN    0x0F040900
#define PLX9054_INT_DMA   0x0F040100

/*****  DMA Registers  *******************************************************/

#define PLX9054_DMAMODE0  0x80  // DMA Channel 0 Mode
#define PLX9054_DMAPADR0  0x84  // DMA Channel 0 PCI Address
#define PLX9054_DMALADR0  0x88  // DMA Channel 0 Local Address
#define PLX9054_DMASIZ0   0x8C  // DMA Channel 0 Transfer Size
#define PLX9054_DMADPR0   0x90  // DMA Channel 0 Descriptor Pointer
#define PLX9054_DMAMODE1  0x94  // DMA Channel 1 Mode
#define PLX9054_DMAPADR1  0x98  // DMA Channel 1 PCI Address
#define PLX9054_DMALADR1  0x9C  // DMA Channel 1 Local Address
#define PLX9054_DMASIZ1   0xA0  // DMA Channel 1 Transfer Size
#define PLX9054_DMADPR1   0xA4  // DMA Channel 1 Descriptor Pointer
#define PLX9054_DMACSR0   0xA8  // DMA Channel 0 Command/Status
#define PLX9054_DMACSR1   0xA9  // DMA Channel 1 Command/Status
#define PLX9054_DMAARB    0xAC  // DMA Arbitration
#define PLX9054_DMATHR    0xB0  // DMA Threshold
#define PLX9054_DMADAC0   0xB4  // DMA Channel 0 PCI Dual Address Cycle Address
#define PLX9054_DMADAC1   0xB8  // DMA Channel 1 PCI Dual Address Cycle Address


/*****  Messaging Queue Registers  *******************************************/

#define PLX9054_OPQIS   0x30  // Outbound Post Queue Interrupt Status
#define PLX9054_OPQIM   0x34  // Outbound Post Queue Interrupt Mask
#define PLX9054_IQP   0x40  // Inbound Queue Port
#define PLX9054_OQP   0x44  // Outbound Queue Port
#define PLX9054_MQCR    0xC0  // Messaging Queue Configuration
#define PLX9054_QBAR    0xC4  // Queue Base Address
#define PLX9054_IFHPR   0xC8  // Inbound Free Head Pointer
#define PLX9054_IFTPR   0xCC  // Inbound Free Tail Pointer
#define PLX9054_IPHPR   0xD0  // Inbound Post Head Pointer
#define PLX9054_IPTPR   0xD4  // Inbound Post Tail Pointer
#define PLX9054_OFHPR   0xD8  // Outbound Free Head Pointer
#define PLX9054_OFTPR   0xDC  // Outbound Free Tail Pointer
#define PLX9054_OPHPR   0xE0  // Outbound Post Head Pointer
#define PLX9054_OPTPR   0xE4  // Outbound Post Tail Pointer
#define PLX9054_QSR   0xE8  // Queue Status/Control

#define PLX9054_INTCSR_PCI_ERROR_ENABLE     (1<<6)
#define PLX9054_INTCSR_PCI_ERROR_ACTIVE     (1<<7)
#define PLX9054_INTCSR_PCI_ERROR_CLEAR      (1<<7)

#define PLX9054_INTCSR_PCI_INTERRUPT_ENABLE   (1<<10)

#define PLX9054_INTCSR_LOCAL_INTERRUPT_INPUT_ENABLE (1<<11)
#define PLX9054_INTCSR_LOCAL_INPUT_INTERRUPT_ACTIVE (1<<15)

#define PLX9054_INTCSR_LOCAL_DMA0_INTERRUPT_ENABLE  (1<<18)
#define PLX9054_INTCSR_LOCAL_DMA1_INTERRUPT_ENABLE  (1<<19)
#define PLX9054_INTCSR_LOCAL_DMA0_INTERRUPT_ACTIVE  (1<<21)
#define PLX9054_INTCSR_LOCAL_DMA1_INTERRUPT_ACTIVE  (1<<22)

typedef union {
  unsigned int val;
  struct {
    unsigned int master_local_err_enable: 1;
    unsigned int slave_local_err_enable: 1;
    unsigned int gen_pci_error: 1;
    unsigned int mbox_enable: 1;

    unsigned int pm_int_enable: 1;
    unsigned int pm_int: 1;
    unsigned int pci_error_enable: 1;
    unsigned int pci_error: 1;

    unsigned int pci_int_enable: 1;
    unsigned int pci_doorbell_int_enable: 1;
    unsigned int pci_abort_int_enable: 1;
    unsigned int local_int_input_enable: 1;

    unsigned int retry_abort_enable: 1;     /* enable retry abort interrupt     */
    unsigned int pci_doorbell: 1;           /* pci doorbell interrupt is active */
    unsigned int pci_abort: 1;              /*    pci abort interrupt is active */
    unsigned int local_int_active: 1;       /*  local interrupt input is active */

    unsigned int local_int_output_enable: 1;
    unsigned int local_doorbell_int_enable: 1;
    unsigned int dma0_int_enable: 1;
    unsigned int dma1_int_enable: 1;

    unsigned int local_doorbell_int_active: 1;
    unsigned int dma0_int_active: 1;
    unsigned int dma1_int_active: 1;
    unsigned int bist_int_active: 1;

    unsigned int master_failed: 1;
    unsigned int dma0_failed: 1;
    unsigned int dma1_failed: 1;
    unsigned int target_failed: 1;

    unsigned int mbox0: 1;
    unsigned int mbox1: 1;
    unsigned int mbox2: 1;
    unsigned int mbox3: 1;
  } bit;
} plx9054_intcsr;

typedef union {
  unsigned int val;
  struct {
    unsigned int dma0_enable: 1;
    unsigned int dma0_start: 1;
    unsigned int dma0_abort: 1;
    unsigned int dma0_clear_int: 1;
    unsigned int dma0_done: 1;
    unsigned int dma0_rsv: 3;

    unsigned int dma1_enable: 1;
    unsigned int dma1_start: 1;
    unsigned int dma1_abort: 1;
    unsigned int dma1_clear_int: 1;
    unsigned int dma1_done: 1;
    unsigned int dma1_rsv: 3;

    unsigned int rsv: 16;
  } bit;
} plx9054_dmacsr;


#define PLX9054_MAGIC ('P'|0x80)

#define PLXIOGET_LAS0RR   _IOR(PLX9054_MAGIC, PLX9054_LAS0RR, sizeof(unsigned int))
#define PLXIOGET_LAS0BA   _IOR(PLX9054_MAGIC, PLX9054_LAS0BA, sizeof(unsigned int))
#define PLXIOGET_MARBR    _IOR(PLX9054_MAGIC, PLX9054_MARBR,  sizeof(unsigned int))
#define PLXIOGET_BIGEND   _IOR(PLX9054_MAGIC, PLX9054_BIGEND, sizeof(unsigned int))
#define PLXIOGET_LMISC    _IOR(PLX9054_MAGIC, PLX9054_LMISC,  sizeof(unsigned int))
#define PLXIOGET_PROT_AREA  _IOR(PLX9054_MAGIC, PLX9054_PROT_AREA,  sizeof(unsigned int))
#define PLXIOGET_EROMRR   _IOR(PLX9054_MAGIC, PLX9054_EROMRR, sizeof(unsigned int))
#define PLXIOGET_EROMBA   _IOR(PLX9054_MAGIC, PLX9054_EROMBA, sizeof(unsigned int))
#define PLXIOGET_LBRD0    _IOR(PLX9054_MAGIC, PLX9054_LBRD0,  sizeof(unsigned int))
#define PLXIOGET_DMRR   _IOR(PLX9054_MAGIC, PLX9054_DMRR, sizeof(unsigned int))
#define PLXIOGET_DMLBAM   _IOR(PLX9054_MAGIC, PLX9054_DMLBAM, sizeof(unsigned int))
#define PLXIOGET_DMLBAI   _IOR(PLX9054_MAGIC, PLX9054_DMLBAI, sizeof(unsigned int))
#define PLXIOGET_DMPBAM   _IOR(PLX9054_MAGIC, PLX9054_DMPBAM, sizeof(unsigned int))
#define PLXIOGET_DMCFGA   _IOR(PLX9054_MAGIC, PLX9054_DMCFGA, sizeof(unsigned int))
#define PLXIOGET_LAS1RR   _IOR(PLX9054_MAGIC, PLX9054_LAS1RR, sizeof(unsigned int))
#define PLXIOGET_LAS1BA   _IOR(PLX9054_MAGIC, PLX9054_LAS1BA, sizeof(unsigned int))
#define PLXIOGET_LBRD1    _IOR(PLX9054_MAGIC, PLX9054_LBRD1,  sizeof(unsigned int))
#define PLXIOGET_DMDAC    _IOR(PLX9054_MAGIC, PLX9054_DMDAC,  sizeof(unsigned int))
#define PLXIOGET_MBOX0    _IOR(PLX9054_MAGIC, PLX9054_MBOX0,  sizeof(unsigned int))
#define PLXIOGET_MBOX1    _IOR(PLX9054_MAGIC, PLX9054_MBOX1,  sizeof(unsigned int))
#define PLXIOGET_MBOX2    _IOR(PLX9054_MAGIC, PLX9054_MBOX2,  sizeof(unsigned int))
#define PLXIOGET_MBOX3    _IOR(PLX9054_MAGIC, PLX9054_MBOX3,  sizeof(unsigned int))
#define PLXIOGET_MBOX4    _IOR(PLX9054_MAGIC, PLX9054_MBOX4,  sizeof(unsigned int))
#define PLXIOGET_MBOX5    _IOR(PLX9054_MAGIC, PLX9054_MBOX5,  sizeof(unsigned int))
#define PLXIOGET_MBOX6    _IOR(PLX9054_MAGIC, PLX9054_MBOX6,  sizeof(unsigned int))
#define PLXIOGET_MBOX7    _IOR(PLX9054_MAGIC, PLX9054_MBOX7,  sizeof(unsigned int))
#define PLXIOGET_P2LDBELL _IOR(PLX9054_MAGIC, PLX9054_P2LDBELL, sizeof(unsigned int))
#define PLXIOGET_L2PDBELL _IOR(PLX9054_MAGIC, PLX9054_L2PDBELL, sizeof(unsigned int))
#define PLXIOGET_INTCSR   _IOR(PLX9054_MAGIC, PLX9054_INTCSR, sizeof(unsigned int))
#define PLXIOGET_CNTRL    _IOR(PLX9054_MAGIC, PLX9054_CNTRL,  sizeof(unsigned int))
#define PLXIOGET_PCIHIDR  _IOR(PLX9054_MAGIC, PLX9054_PCIHIDR,  sizeof(unsigned int))
#define PLXIOGET_PCIHREV  _IOR(PLX9054_MAGIC, PLX9054_PCIHREV,  sizeof(unsigned int))
#define PLXIOGET_DMAMODE0 _IOR(PLX9054_MAGIC, PLX9054_DMAMODE0, sizeof(unsigned int))
#define PLXIOGET_DMAPADR0 _IOR(PLX9054_MAGIC, PLX9054_DMAPADR0, sizeof(unsigned int))
#define PLXIOGET_DMALADR0 _IOR(PLX9054_MAGIC, PLX9054_DMALADR0, sizeof(unsigned int))
#define PLXIOGET_DMASIZ0  _IOR(PLX9054_MAGIC, PLX9054_DMASIZ0,  sizeof(unsigned int))
#define PLXIOGET_DMADPR0  _IOR(PLX9054_MAGIC, PLX9054_DMADPR0,  sizeof(unsigned int))
#define PLXIOGET_DMAMODE1 _IOR(PLX9054_MAGIC, PLX9054_DMAMODE1, sizeof(unsigned int))
#define PLXIOGET_DMAPADR1 _IOR(PLX9054_MAGIC, PLX9054_DMAPADR1, sizeof(unsigned int))
#define PLXIOGET_DMALADR1 _IOR(PLX9054_MAGIC, PLX9054_DMALADR1, sizeof(unsigned int))
#define PLXIOGET_DMASIZ1  _IOR(PLX9054_MAGIC, PLX9054_DMASIZ1,  sizeof(unsigned int))
#define PLXIOGET_DMADPR1  _IOR(PLX9054_MAGIC, PLX9054_DMADPR1,  sizeof(unsigned int))
#define PLXIOGET_DMACSR0  _IOR(PLX9054_MAGIC, PLX9054_DMACSR0,  sizeof(unsigned int))
#define PLXIOGET_DMACSR1  _IOR(PLX9054_MAGIC, PLX9054_DMACSR1,  sizeof(unsigned int))
#define PLXIOGET_DMAARB   _IOR(PLX9054_MAGIC, PLX9054_DMAARB, sizeof(unsigned int))
#define PLXIOGET_DMATHR   _IOR(PLX9054_MAGIC, PLX9054_DMATHR, sizeof(unsigned int))
#define PLXIOGET_DMADAC0  _IOR(PLX9054_MAGIC, PLX9054_DMADAC0,  sizeof(unsigned int))
#define PLXIOGET_DMADAC1  _IOR(PLX9054_MAGIC, PLX9054_DMADAC1,  sizeof(unsigned int))
#define PLXIOGET_OPQIS    _IOR(PLX9054_MAGIC, PLX9054_OPQIS,  sizeof(unsigned int))
#define PLXIOGET_OPQIM    _IOR(PLX9054_MAGIC, PLX9054_OPQIM,  sizeof(unsigned int))
#define PLXIOGET_IQP    _IOR(PLX9054_MAGIC, PLX9054_IQP,  sizeof(unsigned int))
#define PLXIOGET_OQP    _IOR(PLX9054_MAGIC, PLX9054_OQP,  sizeof(unsigned int))
#define PLXIOGET_MQCR   _IOR(PLX9054_MAGIC, PLX9054_MQCR, sizeof(unsigned int))
#define PLXIOGET_QBAR   _IOR(PLX9054_MAGIC, PLX9054_QBAR, sizeof(unsigned int))
#define PLXIOGET_IFHPR    _IOR(PLX9054_MAGIC, PLX9054_IFHPR,  sizeof(unsigned int))
#define PLXIOGET_IFTPR    _IOR(PLX9054_MAGIC, PLX9054_IFTPR,  sizeof(unsigned int))
#define PLXIOGET_IPHPR    _IOR(PLX9054_MAGIC, PLX9054_IPHPR,  sizeof(unsigned int))
#define PLXIOGET_IPTPR    _IOR(PLX9054_MAGIC, PLX9054_IPTPR,  sizeof(unsigned int))
#define PLXIOGET_OFHPR    _IOR(PLX9054_MAGIC, PLX9054_OFHPR,  sizeof(unsigned int))
#define PLXIOGET_OFTPR    _IOR(PLX9054_MAGIC, PLX9054_OFTPR,  sizeof(unsigned int))
#define PLXIOGET_OPHPR    _IOR(PLX9054_MAGIC, PLX9054_OPHPR,  sizeof(unsigned int))
#define PLXIOGET_OPTPR    _IOR(PLX9054_MAGIC, PLX9054_OPTPR,  sizeof(unsigned int))
#define PLXIOGET_QSR    _IOR(PLX9054_MAGIC, PLX9054_QSR,  sizeof(unsigned int))

#define PLXIOSET_LAS0RR   _IOW(PLX9054_MAGIC, PLX9054_LAS0RR, sizeof(unsigned int))
#define PLXIOSET_LAS0BA   _IOW(PLX9054_MAGIC, PLX9054_LAS0BA, sizeof(unsigned int))
#define PLXIOSET_MARBR    _IOW(PLX9054_MAGIC, PLX9054_MARBR,  sizeof(unsigned int))
#define PLXIOSET_BIGEND   _IOW(PLX9054_MAGIC, PLX9054_BIGEND, sizeof(unsigned int))
#define PLXIOSET_LMISC    _IOW(PLX9054_MAGIC, PLX9054_LMISC,  sizeof(unsigned int))
#define PLXIOSET_PROT_AREA  _IOW(PLX9054_MAGIC, PLX9054_PROT_AREA,  sizeof(unsigned int))
#define PLXIOSET_EROMRR   _IOW(PLX9054_MAGIC, PLX9054_EROMRR, sizeof(unsigned int))
#define PLXIOSET_EROMBA   _IOW(PLX9054_MAGIC, PLX9054_EROMBA, sizeof(unsigned int))
#define PLXIOSET_LBRD0    _IOW(PLX9054_MAGIC, PLX9054_LBRD0,  sizeof(unsigned int))
#define PLXIOSET_DMRR   _IOW(PLX9054_MAGIC, PLX9054_DMRR, sizeof(unsigned int))
#define PLXIOSET_DMLBAM   _IOW(PLX9054_MAGIC, PLX9054_DMLBAM, sizeof(unsigned int))
#define PLXIOSET_DMLBAI   _IOW(PLX9054_MAGIC, PLX9054_DMLBAI, sizeof(unsigned int))
#define PLXIOSET_DMPBAM   _IOW(PLX9054_MAGIC, PLX9054_DMPBAM, sizeof(unsigned int))
#define PLXIOSET_DMCFGA   _IOW(PLX9054_MAGIC, PLX9054_DMCFGA, sizeof(unsigned int))
#define PLXIOSET_LAS1RR   _IOW(PLX9054_MAGIC, PLX9054_LAS1RR, sizeof(unsigned int))
#define PLXIOSET_LAS1BA   _IOW(PLX9054_MAGIC, PLX9054_LAS1BA, sizeof(unsigned int))
#define PLXIOSET_LBRD1    _IOW(PLX9054_MAGIC, PLX9054_LBRD1,  sizeof(unsigned int))
#define PLXIOSET_DMDAC    _IOW(PLX9054_MAGIC, PLX9054_DMDAC,  sizeof(unsigned int))
#define PLXIOSET_MBOX0    _IOW(PLX9054_MAGIC, PLX9054_MBOX0,  sizeof(unsigned int))
#define PLXIOSET_MBOX1    _IOW(PLX9054_MAGIC, PLX9054_MBOX1,  sizeof(unsigned int))
#define PLXIOSET_MBOX2    _IOW(PLX9054_MAGIC, PLX9054_MBOX2,  sizeof(unsigned int))
#define PLXIOSET_MBOX3    _IOW(PLX9054_MAGIC, PLX9054_MBOX3,  sizeof(unsigned int))
#define PLXIOSET_MBOX4    _IOW(PLX9054_MAGIC, PLX9054_MBOX4,  sizeof(unsigned int))
#define PLXIOSET_MBOX5    _IOW(PLX9054_MAGIC, PLX9054_MBOX5,  sizeof(unsigned int))
#define PLXIOSET_MBOX6    _IOW(PLX9054_MAGIC, PLX9054_MBOX6,  sizeof(unsigned int))
#define PLXIOSET_MBOX7    _IOW(PLX9054_MAGIC, PLX9054_MBOX7,  sizeof(unsigned int))
#define PLXIOSET_P2LDBELL _IOW(PLX9054_MAGIC, PLX9054_P2LDBELL, sizeof(unsigned int))
#define PLXIOSET_L2PDBELL _IOW(PLX9054_MAGIC, PLX9054_L2PDBELL, sizeof(unsigned int))
#define PLXIOSET_INTCSR   _IOW(PLX9054_MAGIC, PLX9054_INTCSR, sizeof(unsigned int))
#define PLXIOSET_CNTRL    _IOW(PLX9054_MAGIC, PLX9054_CNTRL,  sizeof(unsigned int))
#define PLXIOSET_PCIHIDR  _IOW(PLX9054_MAGIC, PLX9054_PCIHIDR,  sizeof(unsigned int))
#define PLXIOSET_PCIHREV  _IOW(PLX9054_MAGIC, PLX9054_PCIHREV,  sizeof(unsigned int))
#define PLXIOSET_DMAMODE0 _IOW(PLX9054_MAGIC, PLX9054_DMAMODE0, sizeof(unsigned int))
#define PLXIOSET_DMAPADR0 _IOW(PLX9054_MAGIC, PLX9054_DMAPADR0, sizeof(unsigned int))
#define PLXIOSET_DMALADR0 _IOW(PLX9054_MAGIC, PLX9054_DMALADR0, sizeof(unsigned int))
#define PLXIOSET_DMASIZ0  _IOW(PLX9054_MAGIC, PLX9054_DMASIZ0,  sizeof(unsigned int))
#define PLXIOSET_DMADPR0  _IOW(PLX9054_MAGIC, PLX9054_DMADPR0,  sizeof(unsigned int))
#define PLXIOSET_DMAMODE1 _IOW(PLX9054_MAGIC, PLX9054_DMAMODE1, sizeof(unsigned int))
#define PLXIOSET_DMAPADR1 _IOW(PLX9054_MAGIC, PLX9054_DMAPADR1, sizeof(unsigned int))
#define PLXIOSET_DMALADR1 _IOW(PLX9054_MAGIC, PLX9054_DMALADR1, sizeof(unsigned int))
#define PLXIOSET_DMASIZ1  _IOW(PLX9054_MAGIC, PLX9054_DMASIZ1,  sizeof(unsigned int))
#define PLXIOSET_DMADPR1  _IOW(PLX9054_MAGIC, PLX9054_DMADPR1,  sizeof(unsigned int))
#define PLXIOSET_DMACSR0  _IOW(PLX9054_MAGIC, PLX9054_DMACSR0,  sizeof(unsigned int))
#define PLXIOSET_DMACSR1  _IOW(PLX9054_MAGIC, PLX9054_DMACSR1,  sizeof(unsigned int))
#define PLXIOSET_DMAARB   _IOW(PLX9054_MAGIC, PLX9054_DMAARB, sizeof(unsigned int))
#define PLXIOSET_DMATHR   _IOW(PLX9054_MAGIC, PLX9054_DMATHR, sizeof(unsigned int))
#define PLXIOSET_DMADAC0  _IOW(PLX9054_MAGIC, PLX9054_DMADAC0,  sizeof(unsigned int))
#define PLXIOSET_DMADAC1  _IOW(PLX9054_MAGIC, PLX9054_DMADAC1,  sizeof(unsigned int))
#define PLXIOSET_OPQIS    _IOW(PLX9054_MAGIC, PLX9054_OPQIS,  sizeof(unsigned int))
#define PLXIOSET_OPQIM    _IOW(PLX9054_MAGIC, PLX9054_OPQIM,  sizeof(unsigned int))
#define PLXIOSET_IQP    _IOW(PLX9054_MAGIC, PLX9054_IQP,  sizeof(unsigned int))
#define PLXIOSET_OQP    _IOW(PLX9054_MAGIC, PLX9054_OQP,  sizeof(unsigned int))
#define PLXIOSET_MQCR   _IOW(PLX9054_MAGIC, PLX9054_MQCR, sizeof(unsigned int))
#define PLXIOSET_QBAR   _IOW(PLX9054_MAGIC, PLX9054_QBAR, sizeof(unsigned int))
#define PLXIOSET_IFHPR    _IOW(PLX9054_MAGIC, PLX9054_IFHPR,  sizeof(unsigned int))
#define PLXIOSET_IFTPR    _IOW(PLX9054_MAGIC, PLX9054_IFTPR,  sizeof(unsigned int))
#define PLXIOSET_IPHPR    _IOW(PLX9054_MAGIC, PLX9054_IPHPR,  sizeof(unsigned int))
#define PLXIOSET_IPTPR    _IOW(PLX9054_MAGIC, PLX9054_IPTPR,  sizeof(unsigned int))
#define PLXIOSET_OFHPR    _IOW(PLX9054_MAGIC, PLX9054_OFHPR,  sizeof(unsigned int))
#define PLXIOSET_OFTPR    _IOW(PLX9054_MAGIC, PLX9054_OFTPR,  sizeof(unsigned int))
#define PLXIOSET_OPHPR    _IOW(PLX9054_MAGIC, PLX9054_OPHPR,  sizeof(unsigned int))
#define PLXIOSET_OPTPR    _IOW(PLX9054_MAGIC, PLX9054_OPTPR,  sizeof(unsigned int))
#define PLXIOSET_QSR    _IOW(PLX9054_MAGIC, PLX9054_QSR,  sizeof(unsigned int))

#define PLXIOCLR_DMA    _IO(PLX9054_MAGIC, 1)
#endif  /* _PLX9054_H_ */
