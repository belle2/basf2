/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <assert.h>

#define COPPER_FF_RST           0x00000050      // COPPER Event FIFO & FINESSE Soft Reset

void
do_reset(void * start)
{
        volatile unsigned long * lp = (unsigned long *)(start + COPPER_FF_RST);
        *lp = 0x1F;
	sleep(1);
        *lp = 0;
}

void
do_scan(void * start)
{
        unsigned long * lp = start;
        int i;

        for (i=0; i*4<0x100; i++) {
                printf("COPPER REG %3d %08x\n", i, lp[i]);
        }

}

unsigned long
find_offset()
{
        FILE * fp = fopen("/proc/bus/pci/01/08.0", "r");
        unsigned long word[0x100];
        int n;
        if (fp == NULL)
                return 0;

        n = fread(word, sizeof(unsigned long), 0x100, fp);
        if (n < 0x10)
                return 0;
        assert(word[0] == 0x905410b5);
        return word[6];
}
unsigned char *
do_mmap()
{
        int fd;
        int offset;
        unsigned char * ret;
        fd = open("/dev/mem", O_RDWR);
        /* offset = 0xfa800000; */
        offset = find_offset();
        printf("%x\n", offset);
        ret = mmap(0, 0x200, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset);
        return ret;
}

void
doit()
{
        unsigned char * copperreg = do_mmap();
        if (copperreg == MAP_FAILED)
                return;

        do_scan(copperreg);
        do_reset(copperreg);
        return;
}

main(int argc, char ** argv)
{
        doit();
        exit(0);
}
