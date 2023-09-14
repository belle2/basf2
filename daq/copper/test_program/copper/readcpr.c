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
#include <sys/select.h>
#include <sys/errno.h>
#include "copper.h"

unsigned long buffer[1024 * 1024];
int timeout;
int terminated = 0;

void show_event(unsigned long *head, int len)
{
    int i, j;
    for (i = 0; i + 1 < (len / 4); i += 8) {
	printf("%08d %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", i,
	       buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3],
	       buffer[i + 4], buffer[i + 5], buffer[i + 6], buffer[i + 7]);
    }
    printf("%08d", i);
    for (j = 0; j < (len / 4) - i; j += 4) {
	printf(" %08lx", buffer[i + j]);
    }
    printf("\n");
}

unsigned long xor(unsigned long *start, int wordlen)
{
    unsigned long ret = 0;
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
    int ch;

    while ((ch = getopt(argc, argv, "c:")) != -1) {
	switch (ch) {
	case 'c':
		maxevent = strtol(optarg, 0, 0);
		break;
	}
    }

    fd = open("/dev/copper/copper", O_RDONLY);

    for (i=0; i<100; i++) {
	unsigned int val;
        reset_fifo_and_finesse(fd);
	ret = ioctl(fd, CPRIOGET_FF_STA, &val, sizeof(val));
	printf("FF_STA = %08x\n", val);
	if ((val & 0xFF) == 0x23)
	    break;
    }

#if 1
    ret = ioctl(fd, CPRIO_INIT_RUN, 0);
    fprintf(stderr, "CPRIO_INIT_RUN ret = %d\n", ret);
#endif

    signal(SIGALRM, do_alarm);


    if (fd < 0) {
	perror("open");
	exit(0);
    }

    while (1) {
#if 0
	/* If you want to test select() */
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	FD_ZERO(&efds);
	FD_SET(fd, &efds);
	ret = select(1 + fd, &rfds, NULL, &efds, NULL);
	fprintf(stderr, "select returned %d\n", ret);

	if (FD_ISSET(fd, &rfds))
	    fprintf(stderr, "fd %d is readable\n", fd);

	if (FD_ISSET(fd, &efds))
	    fprintf(stderr, "fd %d is error\n", fd);
#endif

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
		fprintf(stderr, "bad copper evn = %lx should be %x\n",
			buffer[1], event);
	    }
	}

	if (1 || event < 10) {
#if 1
	    printf("xor = %08lx\n", xor((unsigned long *) buffer, ret / 4));
	    show_event(buffer, ret);
#else
	    fwrite(buffer, ret, 1, stdout);
#endif
	}

	if (maxevent && maxevent <= event) {
	    break;
	}

	event++;
    }

    if (ret > 0) {
	fprintf(stderr, "last event\n");
	show_event(buffer, ret);
    }
}
