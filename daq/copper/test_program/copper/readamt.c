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
#include <errno.h>
#include <sys/select.h>
#include "copper.h"

unsigned long buffer[1024 * 1024];

void
show_event(unsigned long * head, int len)
{
    int i;
    for (i=0; i<(len/4); i+= 8) {
        printf("%08d %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
            buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
            buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
    }
}

unsigned long
xor(unsigned long * start, int wordlen)
{
    unsigned long ret = 0;
    int i;
    while (wordlen--) {
        ret ^= *(start++);
    }
    return ret;
}

int cprfd;

main()
{
    int event=0;
    int ret, i = 0;
    fd_set rfds, efds;
    int amtfd[4];
    
    int j=1;

    int val=7;
  int use_slot_A = 0; // configure
  int use_slot_B = 0; // accordingly
  int use_slot_C = 1; // to your
  int use_slot_D = 0; // system

    cprfd = open("/dev/copper/copper", O_RDONLY);
    printf("cprfd= %d\n",cprfd);
/*    ioctl(cprfd,CPRIOSET_LEF_WA_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WB_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WC_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WD_FF,&j); */
    if (cprfd < 0) {
        perror("open");
        exit(0);
    }

  val = (use_slot_A<<0) | (use_slot_B<<1) | (use_slot_C<<2) | (use_slot_D<<3);
  ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &val, sizeof(val));



    amtfd[0] = open("/dev/copper/amt3:a", O_RDWR);
    amtfd[1] = open("/dev/copper/amt3:b", O_RDWR);
    amtfd[2] = open("/dev/copper/amt3:c", O_RDWR);
    amtfd[3] = open("/dev/copper/amt3:d", O_RDWR); /* 201002261156 modified by sundehui  */

    printf("amt = %d:%d:%d:%d\n", amtfd[0], amtfd[1], amtfd[2], amtfd[3]);

    while (1) {
/*        printf("before redo.\n"); */
	int redo;
/*        printf("                         after redo.\n"); */
#if 0
	/* If you want to test select() */
        FD_ZERO(&rfds); FD_SET(cprfd, &rfds);
        printf("after Rfds.\n");
        FD_ZERO(&efds); FD_SET(cprfd, &efds);
        printf("                         after Efds.\n");
        ret = select(1+fd, &rfds, NULL, &efds, NULL);
        printf("select returned %d\n", ret);

        if (FD_ISSET(cprfd, &rfds))
            printf("fd %d is readable\n", fd);

        if (FD_ISSET(cprfd, &efds))
            printf("fd %d is error\n", fd);
#endif

	do {
		ret = read(cprfd, (char *)buffer, sizeof(buffer));
		if (ret < 0)
			perror("read");
	} while (ret == -1 && errno == EINTR);

	if (ret == 0) { /* EOF indicates end of this run */
		break;
	}

        i++;

        if (i % 1000 == 0)
        {
            write(2, ".", 1);
        }

	{
	    char * p = (char *)buffer;
	    struct copper_header * header = (struct copper_header *)p;
	    struct copper_footer * footer =
		(struct copper_footer *)(p + ret - sizeof(struct copper_footer));

	    if (header->magic != COPPER_DRIVER_HEADER_MAGIC) {
		printf("bad fotter %x\n", header->magic);
		break;
	    }

	    if (footer->magic != COPPER_DRIVER_FOOTER_MAGIC) {
		printf("bad fotter %x\n", footer->magic);
		break;
	    }

	    if (header->event_number != event) {
		printf("bad copper evn = %x should be %x\n", buffer[1], event);
	    }
	}

        if (event < 10) { /*  modified by sun */
            printf("xor = %08x\n", xor((unsigned long *)buffer, ret/4));
            show_event(buffer, ret);
        }     /*  modified by sun */
        event++;
    }

    if (ret > 0) {
        printf("last event\n");
        show_event(buffer, ret);
    }
}
