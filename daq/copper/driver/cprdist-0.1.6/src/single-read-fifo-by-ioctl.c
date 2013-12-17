#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "copper.h"

int slotA = 0;
int slotB = 0;
int slotC = 0;
int slotD = 0;
int duplimit = 50;

int ninterest = 0;
int interest[1024];

int maxevent = 0;

int cprfd = -1;

void do_reset()
{
    int ret;
    int val;
    val = 0x1F;
    ret = ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
    assert(ret == 0);
    val = 0;
    ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
    assert(ret == 0);
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

int do_read_fifo(char slot)
{
    int ret = 0;
    int ndup = 0;
    int offset = 0;
    unsigned long word, lastword = 0;

    char title[6] = { 's', 'l', 'o', 't', 0, 0 };

    unsigned int cmd;

    title[4] = slot;

    switch (slot) {
    case 'A': cmd = CPRIOGET_FF_RA; break;
    case 'B': cmd = CPRIOGET_FF_RB; break;
    case 'C': cmd = CPRIOGET_FF_RC; break;
    case 'D': cmd = CPRIOGET_FF_RD; break;
    }

    while (1) {
	int word;
	{
	    int ret;
	    ret = ioctl(cprfd, cmd, &word, sizeof(word));
	    assert(ret == 0);
	}
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

int do_read_all_fifo()
{
    int ret = 0;
    if (slotA) {
        ret += do_read_fifo('A');
    }
    if (slotB) {
        ret += do_read_fifo('B');
    }
    if (slotC) {
        ret += do_read_fifo('C');
    }
    if (slotD) {
        ret += do_read_fifo('D');
    }

    return ret;
}

void enable_single_read()
{
    int val = 1;
    int ret = ioctl(cprfd, CPRIOSET_FF_RW, &val, sizeof(val));
    assert(ret == 0);
    ret = ioctl(cprfd, CPRIO_DISABLE_INTERRUPT);
    assert(ret == 0);
}

void doit()
{
    int ret;
    int event = 0;

    cprfd = open("/dev/copper/copper", O_RDONLY);
    assert(cprfd != -1);

    do_reset();
    enable_single_read();
    while (1) {
        ret = do_read_all_fifo();
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
                    "[-h] [-s [abcd]] [-n dumplimit] [-i interest offset] [-c maxevent]\n"
                    "\t-c maxevent : exit after reading specified events\n"
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
