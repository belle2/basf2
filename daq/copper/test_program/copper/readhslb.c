/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "cprfin_fngeneric.h"
#include "para.h"
#include "copper.h"

unsigned long buffer[1024 * 1024];

/* ---------------------------------------------------------------------- *\
   rdmgt
\* ---------------------------------------------------------------------- */
int
rdmgt(int fd, int adr)
{
  int val = 0;
  if (ioctl(fd, FNGENERICIO_GET(adr), &val) < 0) {
    fprintf(stderr,"cannot read  %s\n",
            strerror(errno));
    exit(1);
  }
  return val;
}
/* ---------------------------------------------------------------------- *\
   wrmgt
\* ---------------------------------------------------------------------- */
void
wrmgt(int fd, int adr, int val)
{
  if (ioctl(fd, FNGENERICIO_SET(adr), val) < 0) {
    fprintf(stderr,"cannot write %s\n",
             strerror(errno));
    exit(1);
  }
}
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
    int fd[4];
    
    int j=1;
    int k=0;
    int val=7;
    int use_slot[4];
    use_slot[0] = 0; // configure
    use_slot[1] = 1; // accordingly
    use_slot[2] = 0; // to your
    use_slot[3] = 1; // system
    time_t   lt;
    int err_check,err_ini[4];
    cprfd = open("/dev/copper/copper", O_RDONLY);
    printf("cprfd= %d\n",cprfd);
    ioctl(cprfd,CPRIOSET_LEF_WA_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WB_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WC_FF,&j);
    ioctl(cprfd,CPRIOSET_LEF_WD_FF,&j); 
    if (cprfd < 0) {
        perror("open");
        exit(0);
    }

  val = (use_slot[0]<<0) | (use_slot[1]<<1) | (use_slot[2]<<2) | (use_slot[3]<<3);
  ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &val, sizeof(val));



    fd[0] = open("/dev/copper/fngeneric:a", O_RDWR);
    fd[1] = open("/dev/copper/fngeneric:b", O_RDWR);
    fd[2] = open("/dev/copper/fngeneric:c", O_RDWR);
    fd[3] = open("/dev/copper/fngeneric:d", O_RDWR); /* 201002261156 modified by sundehui  */

    printf("HSLB = %d:%d:%d:%d\n", fd[0], fd[1], fd[2], fd[3]);
     for( k = 0 ; k < 4 ; k ++ ){
                if ( !use_slot[k] ) continue;
                lt=time(NULL);/*system   time   and   date*/
                printf(ctime(&lt));   /*english   format   output*/
                err_ini[k] = rdmgt(fd[k], ERROR);
                printf("The number of errors of link %c is %d when the application starts\n",'a' + k, err_ini[k]);
     }
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
			perror("test");
	} while (ret == -1 && errno == EINTR);

	if (ret == 0) { /* EOF indicates end of this run */
		break;
	}

        i++;

        if(i % 1==0){
            for( k = 0 ; k < 4 ; k ++ ){
               if ( !use_slot[k] ) continue;
               err_check = rdmgt(fd[k], ERROR);
               if ( !use_slot[k] ) continue;
               if(err_check != err_ini[k])
               {
                  printf("\nthe number of the events is %d\n",i);
                  lt=time(NULL);/*system   time   and   date*/
                  printf(ctime(&lt));   /*english   format   output*/
                  printf("a error occorred, the number of error is %d now\n",err_check);
                  err_ini[k] = err_check;
               }
            }
        }
        if (i % 5000 == 0)
        {
            write(2, ".", 1);
        }

        if (i % 500000 == 0)
            printf("\nThe number of events is %d \n",i);

	{
	    char * p = (char *)buffer;
	    struct copper_header * header = (struct copper_header *)p;
	    struct copper_footer * footer =
		(struct copper_footer *)(p + ret - sizeof(struct copper_footer));

	    if (header->magic != COPPER_DRIVER_HEADER_MAGIC) {
		printf("bad fotter %x\n", header->magic);
                printf("xor = %08x\n", xor((unsigned long *)buffer, ret/4));
                show_event(buffer, ret);
		break;
	    }

	    if (footer->magic != COPPER_DRIVER_FOOTER_MAGIC) {
		printf("bad fotter %x\n", footer->magic);
               printf("xor = %08x\n", xor((unsigned long *)buffer, ret/4));
               show_event(buffer, ret);
		break;
	    }

	    if (header->event_number != event) {
		printf("bad copper evn = %x should be %x\n", buffer[1], event);
	    }
	}

        if (event < 20) { /*  modified by sun */
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
