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

#define COPPER_FF_RW            0x000000B8      // COPPER Event FIFO Single Read/Write Control

#define COPPER_FF_RA            0x00000110      // COPPER Event FIFO Ach Single Read
#define COPPER_FF_RB            0x00000114      // COPPER Event FIFO Bch Single Read
#define COPPER_FF_RC            0x00000118      // COPPER Event FIFO Cch Single Read
#define COPPER_FF_RD            0x0000011C      // COPPER Event FIFO Dch Single Read

int slotA = 0;
int slotB = 0;
int slotC = 0;
int slotD = 0;
int duplimit = 50;

int ninterest = 0;
int interest[1024];

int maxevent = 0;

void do_reset(void *start)
{
    unsigned long *lp = (unsigned long *) (start + COPPER_FF_RST);
    *lp = 0x1F;
    *lp = 0;
}

void
do_display_word(char * title, unsigned long offset, unsigned long value, int may_be_eof)
{
    int j;

    if (ninterest == 0) {
        if (may_be_eof) {
            printf("%s %4d %08x (%ddup, may be end of the FIFO)\n",
                title, offset, value, may_be_eof);
        } else {
            printf("%s %4d %08x\n",
                title, offset, value);
        }
    } else {
        for (j = 0; j < ninterest; j++) {
            if (interest[j] == offset) {
                printf("%s %4d %08x\n", title, offset, value);
            }
        }
    }
}

int do_read_fifo(char *title, unsigned char *fifop)
{
    int ret = 0;
    int ndup = 0;
    int offset = 0;
    unsigned long word, lastword = 0;

    while (1) {
        word = *(unsigned long *) fifop;
        if (word == lastword) {
            ndup++;
            if (ndup > duplimit)
                return ret;
        } else {
            ndup = 0;
            lastword = word;
        }

	if (ndup == offset && (word >> 16) == 0xFF55) {
	} else {
		do_display_word(title, offset, word, ndup);
		ret = 1;
	}
        offset++;
    }
    return ret;
}

int do_read_all_fifo(unsigned char *start)
{
    int ret = 0;
    if (slotA) {
        ret += do_read_fifo("slotA", start + COPPER_FF_RA);
    }
    if (slotB) {
        ret += do_read_fifo("slotB", start + COPPER_FF_RB);
    }
    if (slotC) {
        ret += do_read_fifo("slotC", start + COPPER_FF_RC);
    }
    if (slotD) {
        ret += do_read_fifo("slotD", start + COPPER_FF_RD);
    }

    return ret;
}

void enable_single_read(unsigned char *start)
{
    *(unsigned long *) (start + COPPER_FF_RW) = 1;
}

void do_scan(void *start)
{
    unsigned long *lp = start;
    int i;

    for (i = 0; i * 4 < 0x100; i++) {
        printf("COPPER REG %3d %08x\n", i, lp[i]);
    }

}

unsigned long find_offset()
{
    FILE *fp = fopen("/proc/bus/pci/01/08.0", "r");
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

unsigned char *do_mmap()
{
    int fd;
    int offset;
    unsigned char *ret;
    fd = open("/dev/mem", O_RDWR);
    /* offset = 0xfa800000; */
    offset = find_offset();
    printf("%x\n", offset);
    ret = mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    return ret;
}

void doit()
{
    int ret;
    int event = 0;
    unsigned char *copperreg = do_mmap();
    if (copperreg == MAP_FAILED)
        return;

    /* do_scan(copperreg); */
    do_reset(copperreg);
    enable_single_read(copperreg);
    while (1) {
        ret = do_read_all_fifo(copperreg);
	if (ret > 0) {
	    printf("==========================\n", ret);
	    event++;
	}
        usleep(10 * 1000);
        if (maxevent && event > maxevent)
	    break;
    }
    return;
}

main(int argc, char **argv)
{
    int ch;
    while ((ch = getopt(argc, argv, "c:hs:n:i:")) != -1) {
        switch (ch) {
	case 'c':
	    maxevent = strtol(optarg, 0, 0);
	    break;
        case 'h':
            fprintf(stderr,
                    "[-h] [-s [abcd]] [-n dumplimit] [-i interest offset]\n"
                    "\t-s ab : will read only slotA and B (ABCD are read by default)\n"
                    "\t-n NUMBER : treat NUMBER times of repetation of same word as end of FIFO\n"
                    "\t-i offset : offset number of interest word\n"
                    "\t\t: -i 1 -i 2 -i 255 will show word[1], word[2], word[255]\n");
            exit(0);
            break;
        case 's':
            if (strchr(optarg, 'a') || strchr(optarg, 'A'))
                slotA = 1;
            if (strchr(optarg, 'b') || strchr(optarg, 'B'))
                slotB = 1;
            if (strchr(optarg, 'c') || strchr(optarg, 'C'))
                slotC = 1;
            if (strchr(optarg, 'd') || strchr(optarg, 'D'))
                slotD = 1;
            break;
        case 'n':
            duplimit = strtol(optarg, 0, 0);
            break;
        case 'i':
            interest[ninterest] = strtol(optarg, 0, 0);
            ninterest++;
            break;
        }
    }

    if (slotA + slotB + slotC + slotD == 0) {
        slotA = 1;
        slotB = 1;
        slotC = 1;
        slotD = 1;
    }
    printf("%d %d %d %d\n", slotA, slotB, slotC, slotD);
    doit();
    exit(0);
}
