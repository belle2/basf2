/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define FINESSE_GENERIC_REG_CSR      480
#define FINESSE_GENERIC_REG_FIFO_EMU 484
#define FINESSE_GENERIC_REG_FPGA_VER 488
#define FINESSE_GENERIC_REG_SERIAL_L 492
#define FINESSE_GENERIC_REG_SERIAL_H 496
#define FINESSE_GENERIC_REG_TYPE_L   500
#define FINESSE_GENERIC_REG_TYPE_H   504
#define FINESSE_GENERIC_REG_RESERVED 508

enum finesse_generic_info {
        FINESSE_GENERIC_INFO_NOP,
        FINESSE_GENERIC_INFO_CSR,
        FINESSE_GENERIC_INFO_FIFO_EMU,
        FINESSE_GENERIC_INFO_FPGA_VER,
        FINESSE_GENERIC_INFO_SERIAL,
        FINESSE_GENERIC_INFO_TYPE,
        FINESSE_GENERIC_INFO_RESERVED,
};

#include "copper.h"
#include "plx9054.h"
#include "cprfin.h"
#include "cprfin-jig.h"

void
change_jig_regs(int * jigfd, char * key, int setcmd, int getcmd, int val)
{
    int i;
    int ret0, ret1, ret2;
    int val0, val1;

    for (i=0; i<4; i++) {
	if (jigfd[i] == -1)
	    continue;
	ret0 = ioctl(jigfd[i], getcmd, &val0, sizeof(val0));
	ret1 = ioctl(jigfd[i], setcmd, &val,  sizeof(val));
	ret2 = ioctl(jigfd[i], getcmd, &val1, sizeof(val1));
	printf("%10s[%d] %x(%d)->%x(%d)->%x(%d)\n",
		key, i, val0, ret0, val, ret1, val1, ret2);
    }
}

#define	SHOW_BIT(n)	printf(#n " = %d\n", csr.bit.n)

void
show_intcsr(plx9054_intcsr csr)
{
    SHOW_BIT(mbox3);
    SHOW_BIT(mbox2);
    SHOW_BIT(mbox1);
    SHOW_BIT(mbox0);
    SHOW_BIT(target_failed);
    SHOW_BIT(dma1_failed);
    SHOW_BIT(dma0_failed);
    SHOW_BIT(master_failed);
    SHOW_BIT(bist_int_active);
    SHOW_BIT(dma1_int_active);
    SHOW_BIT(dma0_int_active);
    SHOW_BIT(local_doorbell_int_active);
    SHOW_BIT(dma1_int_enable);
    SHOW_BIT(dma0_int_enable);
    SHOW_BIT(local_doorbell_int_enable);
    SHOW_BIT(local_int_output_enable);
    SHOW_BIT(local_int_active);
    SHOW_BIT(pci_abort);
    SHOW_BIT(pci_doorbell);
    SHOW_BIT(retry_abort_enable);
    SHOW_BIT(local_int_input_enable);
    SHOW_BIT(pci_abort_int_enable);
    SHOW_BIT(pci_doorbell_int_enable);
    SHOW_BIT(pci_int_enable);
    SHOW_BIT(pci_error);
    SHOW_BIT(pci_error_enable);
    SHOW_BIT(pm_int);
    SHOW_BIT(pm_int_enable);
    SHOW_BIT(mbox_enable);
    SHOW_BIT(gen_pci_error);
    SHOW_BIT(slave_local_err_enable);
    SHOW_BIT(master_local_err_enable);
}

void
check_pci_error(int plxfd)
{
    int ret;
    plx9054_intcsr intcsr;
    ret = ioctl(plxfd, PLXIOGET_INTCSR, &intcsr, sizeof(intcsr));
    if (intcsr.bit.pci_error)
	printf("pci_error (%08x)\n", intcsr.val);
    intcsr.bit.pci_error = 1;
    ret = ioctl(plxfd, PLXIOSET_INTCSR, &intcsr, sizeof(intcsr));
}

#define	SETJIG(n, m)	change_jig_regs(jigfd, #n, FNIOSET_##n, FNIOGET_##n, m)

main()
{
    int jigfd[4];
    int cprfd = open("/dev/copper/copper", O_RDONLY);
    int plxfd = open("/dev/copper/plx9054", O_RDONLY);
    int ret;
    int rev = -1;
    int cmd;
    int start;
    int i;
    int val;

    printf("cprfd = %d\n", cprfd);
    printf("plxfd = %d\n", plxfd);

    ret = ioctl(cprfd, CPRIO_INIT_RUN, 0);
    printf("CPRIO_INIT_RUN ret = %d\n", ret);

    check_pci_error(plxfd);

    jigfd[0] = open("/dev/copper/jig:a",  O_RDONLY);
    jigfd[1] = open("/dev/copper/jig:b",  O_RDONLY);
    jigfd[2] = open("/dev/copper/jig:c",  O_RDONLY);
    jigfd[3] = open("/dev/copper/jig:d",  O_RDONLY);

    ret = ioctl(cprfd, CPRIOGET_VERSION, &val, sizeof(val));
    printf("ret = %d version=%x(%d)\n", ret, val, val);

    /* set clear flag on copper fifo (necessary) */
    val = 0x1F;
    ret = ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
    printf("ret = %d set reset\n", ret);

    /* negate clear flag on copper fifo (necessary) */
    val = 0;
    ret = ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
    printf("ret = %d clear reset\n", ret);

    /* negate COPPER_FF_RW flag (necessary) */
    val = 0;
    ret = ioctl(cprfd, CPRIOSET_FF_RW, &val, sizeof(val));
    printf("ret = %d clear FF_RW\n", ret);

    /* check copper FF_STA */
    {
	char * p = getenv("FINESSE_SLOT");
	int val = 0;
	if (p) {
	    if (strchr(p, 'A') || strchr(p, 'a')) val |= 1;
	    if (strchr(p, 'B') || strchr(p, 'b')) val |= 2;
	    if (strchr(p, 'C') || strchr(p, 'c')) val |= 4;
	    if (strchr(p, 'D') || strchr(p, 'd')) val |= 8;
	} else {
	    val = 0xF;
	}
	ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &val, sizeof(val));
    }

    /* use LEF_TOTAL */
    val = 1; /* use LEF_TOTAL(1) or LEF_AB(0) */
    ret = ioctl(cprfd, CPRIOSET_LEF_READSEL, &val, sizeof(val));

    /* set length fifo interrupt theshold */
    {
	static const int regs[] = {
	    CPRIOSET_LEF_WA_THR, CPRIOSET_LEF_WB_THR,
	    CPRIOSET_LEF_WC_THR, CPRIOSET_LEF_WD_THR,
	};
	for (i=0; i<4; i++) {
	    val = 1;
	    ret = ioctl(cprfd, regs[i], &val, sizeof(val));
	    printf("CPRIOSET_LEF %d\n", ret);
	}
    }

    /* enable length fifo threshold interrupt */
    val = 0x7;	/*	1(event fifo almost full)
			2(length fifo almost full)
			4(length fifo threshold)	*/
    ret  = ioctl(cprfd, CPRIOSET_INT_MASK, &val, sizeof(val));
    ret += ioctl(cprfd, CPRIOGET_INT_MASK, &val, sizeof(val));
    printf("INT_MASK = %x/%d\n", val, ret);

    check_pci_error(plxfd);

    /* check jig revision (not necessary) */
    for (i=0; i<4; i++) {
	if (jigfd[i] == -1)
	    continue;
	ret = ioctl(jigfd[i], FNIOGET_JIG_REV, &rev, sizeof(rev));
	printf("ret = %d JIG_REV = %d\n", ret, rev);
    }

    check_pci_error(plxfd);

    /* check jig FPGA version (not necessary) */
    for (i=0; i<4; i++) {
	if (jigfd[i] == -1)
	    continue;
	ret = ioctl(jigfd[i], FNIOGETGENFPGAVER, &rev, sizeof(rev));
	printf("ret = %d FPGAVER = %d\n", ret, rev);
    }

    check_pci_error(plxfd);

    /* set data pattern from jig */
    SETJIG(JIG_CMD, JIGCMD_DATA_INCREMENT);

    check_pci_error(plxfd);

    /* set data size from jig */
    {
	char * p = getenv("FINESSE_MODE");
	int finesse_mode = 0;
	if (p)
	    finesse_mode = strtol(p, 0, 0);
	SETJIG(JIG_OUTD, finesse_mode * 0x10);
    }

    check_pci_error(plxfd);

    /* start jig */
    SETJIG(JIG_START, 1);

    check_pci_error(plxfd);

    for (i=0; i<4; i++) {
	if (jigfd[i] == -1)
	    continue;
	close(jigfd[i]);
    }
}
