#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
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

unsigned long find_offset()
{
    FILE *fp = fopen("/proc/bus/pci/03/08.0", "r");
    unsigned long word[0x100];
    int n;
    if (fp == NULL)
        return 0;

    n = fread(word, sizeof(unsigned long), 0x100, fp);
    if (n < 0x10)
        return 0;
    assert(word[0] == 0x905410b5);
    printf ( "words = %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x %8.8x\n",
	     word[0],word[1],word[2],word[3],word[4],word[5],word[6],
	     word[7] );
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
//    ret = mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset); // COPPER overall
    offset += 0x00100000; // finesse slot A
//    offset += 0x00103000;   // finesse slot A
    printf("%x\n", offset);
    ret = mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset); // A
    return ret;
}

void doit()
{
    int i;
    int ret;
    int event = 0;
    unsigned char *finA = do_mmap();
    if (finA == MAP_FAILED)
        return;

    for (i=0; i<0x200; i+=4) {
	printf("%4d 0x%08x\n", i, *(unsigned int *)(finA+i));
    }

    return;
}

main(int argc, char **argv)
{
    doit();
    exit(0);
}
