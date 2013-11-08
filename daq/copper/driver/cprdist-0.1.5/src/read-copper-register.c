#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <assert.h>

#include "copper.h"

const int nreg = 0x400;

char * reg_name[0x400];

#define	DICITEM(X)	reg_name[COPPER_##X/4] = #X

void
setup_reg_name()
{
    int i;
    for (i=0; i<nreg; i++) {
	reg_name[i] = "";
    }

    DICITEM(FF_STA);
    DICITEM(CONF_WA_AE);
    DICITEM(CONF_WB_AE);
    DICITEM(CONF_WC_AE);
    DICITEM(CONF_WD_AE);
    DICITEM(CONF_WA_FF);
    DICITEM(CONF_WB_FF);
    DICITEM(CONF_WC_FF);
    DICITEM(CONF_WD_FF);
    DICITEM(CONF_WA_AF);
    DICITEM(CONF_WB_AF);
    DICITEM(CONF_WC_AF);
    DICITEM(CONF_WD_AF);
    DICITEM(FF_RST);
    DICITEM(FINESSE_STA);
    DICITEM(ALMFULL_ENB);
    DICITEM(VERSION);
    DICITEM(TRGCM);
    DICITEM(TRG_LENG);
    DICITEM(LEF_READSEL);
    DICITEM(LEF_TOTAL);
    DICITEM(LEF_AB);
    DICITEM(LEF_CD);
    DICITEM(LEF_STA);
    DICITEM(LEF_WA_FF);
    DICITEM(LEF_WB_FF);
    DICITEM(LEF_WC_FF);
    DICITEM(LEF_WD_FF);
    DICITEM(LEF_WA_AF);
    DICITEM(LEF_WB_AF);
    DICITEM(LEF_WC_AF);
    DICITEM(LEF_WD_AF);
    DICITEM(INT_STA);
    DICITEM(INT_MASK);
    DICITEM(FF_RW);
    DICITEM(INT_FACTOR);
    DICITEM(FF_WA);
    DICITEM(FF_WB);
    DICITEM(FF_WC);
    DICITEM(FF_WD);
    DICITEM(FF_RA);
    DICITEM(FF_RB);
    DICITEM(FF_RC);
    DICITEM(FF_RD);
    DICITEM(EWRP_A);
    DICITEM(EWRP_B);
    DICITEM(EWRP_C);
    DICITEM(EWRP_D);
    DICITEM(WEA_COUNTER);
    DICITEM(WEB_COUNTER);
    DICITEM(WEC_COUNTER);
    DICITEM(WED_COUNTER);
    DICITEM(LWRA_DMA);
    DICITEM(LWRB_DMA);
    DICITEM(LWRC_DMA);
    DICITEM(LWRD_DMA);
    DICITEM(DMA_TRANS);
    DICITEM(DMA_TS_ENB);
    DICITEM(RW_DEBUG);
}

uint32_t *map32(const char * filename)
{
    int fd;
    int offset;
    unsigned char *ret;
    fd = open(filename, O_RDONLY);
    return mmap(0, 0x200, PROT_READ, MAP_SHARED, fd, offset);
}

main(int argc, char **argv)
{
    int i;
    uint32_t * p = map32("/sys/bus/pci/devices/0000:03:08.0/resource2");

    setup_reg_name();

    for (i=0; i<100; i++) {
	printf("%3d %-16s %08x\n", i, reg_name[i], p[i]);
    }
    exit(0);
}
