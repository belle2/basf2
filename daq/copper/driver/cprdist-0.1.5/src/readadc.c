#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <assert.h>
#include <sys/errno.h>
#include "copper.h"

unsigned long buffer[10 * 1024 * 1024];
int timeout;
int terminated = 0;

void show_event(unsigned long *head, int len)
{
    int i, j;
    for (i = 0; i + 1 < (len / 4); i += 8) {
	printf("%08d %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
	       buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3],
	       buffer[i + 4], buffer[i + 5], buffer[i + 6], buffer[i + 7]);
    }
    printf("%08d", i);
    for (j = 0; j < (len / 4) - i; j += 4) {
	printf(" %08x", buffer[i + j]);
    }
    printf("\n");
}

unsigned long xor(unsigned long *start, int wordlen)
{
    unsigned long ret = 0;
    int i;
    while (wordlen--) {
	ret ^= *(start++);
    }
    return ret;
}

void do_alarm(int arg)
{
    fprintf(stderr, "SIGALRM\n");
    timeout = 1;
}

void do_term(int arg)
{
    fprintf(stderr, "SIGTERM\n");
    terminated = 1;
}

void reset_fifo_and_finesse(int cprfd)
{
    int val;
    val = 0x1F;
    ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
    val = 0;
    ioctl(cprfd, CPRIOSET_FF_RST, &val, sizeof(val));
}

main(int argc, char ** argv)
{
    int event = 0;
    int ret, i = 0;
    int fd;
    int maxevent = 0;
    fd_set rfds, efds;
    int ch;
    int wff = 0;
    int ff = 0;

    while ((ch = getopt(argc, argv, "F:W:c:")) != -1) {
	switch (ch) {
	case 'c':
	    maxevent = strtol(optarg, 0, 0);
	    break;
	case 'W':
	    wff = strtol(optarg, 0, 0);
	    break;
	case 'F':
	    ff = strtol(optarg, 0, 0);
	    break;
	}
    }

    fd = open("/dev/copper/copper", O_RDONLY);

#if 0
    for (i=0; i<100; i++) {
	unsigned int val;
        reset_fifo_and_finesse(fd);
	ret = ioctl(fd, CPRIOGET_FF_STA, &val, sizeof(val));
	printf("FF_STA = %08x\n", val);
	if ((val & 0x0F0F0F0F) == 0x03030303) {
	    /* almost empty && empty in slotA,B,C,D */
	    break;
	}
    }
#endif

#if 0
    ret = ioctl(fd, CPRIO_INIT_RUN, 0);
    fprintf(stderr, "CPRIO_INIT_RUN ret = %d\n", ret);
#endif

    /* set WX_FF */
    if (wff != 0) {
	ret = ioctl(fd, CPRIOSET_LEF_WA_FF, &wff, sizeof(wff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_LEF_WB_FF, &wff, sizeof(wff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_LEF_WB_FF, &wff, sizeof(wff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_LEF_WB_FF, &wff, sizeof(wff));
	assert(ret == 0);
    }

    /* set CONF_WX_FF */
    if (ff != 0) {

	ret = ioctl(fd, CPRIOSET_CONF_WA_FF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_FF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_FF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_FF, &ff, sizeof(ff));
	assert(ret == 0);

    	ff *= 2;

	ret = ioctl(fd, CPRIOSET_CONF_WA_AF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_AF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_AF, &ff, sizeof(ff));
	assert(ret == 0);
	ret = ioctl(fd, CPRIOSET_CONF_WB_AF, &ff, sizeof(ff));
	assert(ret == 0);
    }


    signal(SIGALRM, do_alarm);

    if (fd < 0) {
	perror("open");
	exit(0);
    }

    while (1) {
	while (1) {
	    timeout = 0;
	    alarm(5);
	    ret = read(fd, (char *) buffer, sizeof(buffer));
	    if (ret > 0) {
		/* we got data */
		break;
	    } else if (ret == 0) {
		/* we got EOF */
		fprintf(stderr, "We got EOF\n");
		close(fd);
		exit(0);
	    } else {
		/* ret < 0 means error */
		perror("read");
		if (ret == -1 && errno == EINTR) {
		    if (timeout) {
			ioctl(fd, CPRIO_FORCE_DMA, 0);
		    }
		    if (terminated) {
			ioctl(fd, CPRIO_END_RUN, 0);
		    }
		} else {
		    exit(0);
		}
	    }
	}

	i++;

	if (i % 1000 == 0) {
	    write(2, ".", 1);
	}

	{
	    char *p = (char *) buffer;
	    struct copper_header *header = (struct copper_header *) p;
	    struct copper_footer *footer =
		(struct copper_footer *) (p + ret -
					  sizeof(struct copper_footer));

	    if (header->magic != COPPER_DRIVER_HEADER_MAGIC) {
		fprintf(stderr, "bad fotter %x\n", header->magic);
		break;
	    }

	    if (footer->magic != COPPER_DRIVER_FOOTER_MAGIC) {
		fprintf(stderr, "bad fotter %x\n", footer->magic);
		break;
	    }

	    if (header->event_number != event) {
		fprintf(stderr, "bad copper evn = %x should be %x\n",
			buffer[1], event);
	    }
	}

	printf("%8d # of data = %4d,%4d,%4d,%4d, xor=%08x\n",
	    event,
	    buffer[9], buffer[10], buffer[11], buffer[12],
	    xor((unsigned long *) buffer, ret / 4));
#if 0
	show_event(buffer, ret);
#endif
	event++;
	if (maxevent && maxevent < event)
	    exit(0);
    }

    if (ret > 0) {
	fprintf(stderr, "last event\n");
	show_event(buffer, ret);
    }
}
