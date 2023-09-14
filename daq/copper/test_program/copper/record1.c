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
#include <string.h>
#include <time.h>   // for getdate and gettime
#include <unistd.h> // for getdate and gettime

FILE *fpw;  // for file write

unsigned long buffer[1024 * 1024];


void closefile( )
{
    printf("Have closed the file\n");
    fclose(fpw);
    exit(1);
}


void
show_event(unsigned long * head, int len)
{
    int i;
    for (i=0; i<(len/4); i+= 8) {
        printf("%08d %08lx %08lx %08lx %08lx %08lx %08lx %08lx %08lx\n", i,
            buffer[i], buffer[i+1], buffer[i+2], buffer[i+3],
            buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
    }
}


unsigned long
xor(unsigned long * start, int wordlen)
{
    unsigned long ret = 0;
    while (wordlen--) {
        ret ^= *(start++);
    }
    return ret;
}

int cprfd;

main()
{   
    FILE *fpr;  // for file read modified by Jingzhou Zhao
    char SaveFile[100];// SaveFile
    char timebuf[100];
    time_t t;//             

    int event=0;
    int ret, i = 0;
    int amtfd[4];
    int iii;
    int istop;
    int istart;    
    int j=1;
    int card_test[4];
    int val=7;
    int use_slot_A = 0; // configure
    int use_slot_B = 1; // accordingly
    int use_slot_C = 0; // to your
    int use_slot_D = 1; // system

    time(&t);
    strftime(timebuf,sizeof(timebuf),"RUN%Y%m%d%H%M%S",localtime(&t));
    strcpy(SaveFile,timebuf);
    strcat(SaveFile,".dat");
    printf("File Name: %s\n",SaveFile);
 
    if((fpr=fopen(SaveFile,"r"))!=NULL) {
        printf("File: %s exists on the disk!",SaveFile);
        fclose(fpr);
        exit(0);
    }
    else {
        fpw=fopen(SaveFile,"wb");
    }



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

    val = (use_slot_A<<0) | (use_slot_B<<1) | (use_slot_C<<2) | (use_slot_D<<3);
    ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &val, sizeof(val));

    amtfd[0] = open("/dev/copper/fngeneric:a", O_RDWR);
    amtfd[1] = open("/dev/copper/fngeneric:b", O_RDWR);
    amtfd[2] = open("/dev/copper/fngeneric:c", O_RDWR);
    amtfd[3] = open("/dev/copper/fngeneric:d", O_RDWR); /* 201002261156 modified by sundehui  */

    printf("HSLB on COPPER:  %d:%d:%d:%d\n", amtfd[0], amtfd[1], amtfd[2], amtfd[3]);


    while (1) {
/*        printf("before redo.\n"); */
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
	    errno = 0;
	    ret = read(cprfd, (char *)buffer, sizeof(buffer));
	    if (ret < 0) perror("READ");
	} while (ret == -1 && errno == EINTR);

	if (ret == 0)  { /* EOF indicates end of this run */
	    break;
       	}
        for( iii=0;iii<4;iii++) card_test[iii]=0; 
        i++;

        if (i % 1000 == 0){ write(2, ".", 1);}

        if (i% 80000 == 0){ printf("\n event number = %i \n ",i);}

	char * p = (char *)buffer;
	struct copper_header * header = (struct copper_header *)p;
	struct copper_footer * footer =
	    (struct copper_footer *)(p + ret - sizeof(struct copper_footer)); 
	 int t;
       unsigned  short temph,templ;
        if(buffer[ 9] != 0) card_test[0] = 1;
        if(buffer[10] != 0) card_test[1] = 1;
        if(buffer[11] != 0) card_test[2] = 1;
        if(buffer[12] != 0) card_test[3] = 1; // by zjz and lza
       istart = 13;
        for (iii=0;iii<4;iii++) {
         if (card_test[iii]== 0) 
           continue;
         else
           istop = istart + buffer[9 + iii] ; 
	 if(card_test[0]==1 & iii==0) fprintf(fpw,"Board A: %08lx /*** Data length=%08lx, Trigger#=%08lx ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
         if(card_test[1]==1 & iii==1){ fprintf(fpw,"Board B: %08lx /*** Data length=%08lx, Trigger#=%08lx ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
                                       printf("Board B: %08lx /*** Data length=%08lx, Trigger#=%08lx ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);}
         if(card_test[2]==1 & iii==2) fprintf(fpw,"Board C: %08lx /*** Data length=%08lx, Trigger#=%08lx ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
         if(card_test[3]==1 & iii==3) fprintf(fpw,"Board D: %08lx /*** Data length=%08lx, Trigger#=%08lx ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
         printf("istart= %d ,istop = %d\n",istart, istop);
         for (t= istart + 4; t<istop-1; t= t + 1)
          {
 
           if((t-istart-3)%48==0)
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
              fprintf(fpw,"%04x,%04x\n", temph,templ);
             printf(".\n");
             }
           else
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
              fprintf(fpw,"%04x,%04x,", temph,templ);
               printf("*");
            }
          }
        istart = istop; 
       }
/*
         for (t=17; t<(ret/4 - sizeof(struct copper_footer)/4 - 2); t= t + 1)
          {
 
 
           if(t%16==0)
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
              fprintf(fpw,"%04x,%04x\n", temph,templ);
             }
           else
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
              fprintf(fpw,"%04x,%04x,", temph,templ);
            }
          }
 */
/*
   	 for (t=16; t<(ret/4 - sizeof(struct copper_footer)/4 - 2); t+= 1) 
	  {
            
                
           if((t)%32==0)
       	      fprintf(fpw,"%08x\n", buffer[t]);
           else
              fprintf(fpw,"%08x,", buffer[t]);
          }  */

	if (header->magic != COPPER_DRIVER_HEADER_MAGIC)  {
	    printf("bad fotter %x\n", header->magic);
	    break;
	}

	if (footer->magic != COPPER_DRIVER_FOOTER_MAGIC)  {
	    printf(" Event : %i  received, Footer: %x,  xor = %08lx\n", event, footer->magic, xor((unsigned long *)buffer, ret/4));
            printf("ret= %x\n",ret);
    	    printf("bad fotter %x\n", footer->magic);
            show_event(buffer, ret);
            break;
	}

	if (header->event_number != event) { 
	    printf("bad copper evn = %lx should be %x\n", buffer[1], event);
        }


	event++;
        
        if (event <= 10)  {/*   modified by sun */
            printf(" Event : %i  received, xor = %08lx\n", event,  xor((unsigned long *)buffer, ret/4));
            printf("ret= %x\n",ret);
            show_event(buffer, ret);
        }     /*  modified by sun */

//	fwrite(buffer,ret, 1, fpw);//////////////////////////////////////////Modify by Jingzhou Zhao
        
        if( event >4){
            fclose(fpw);
            exit(1);
	};
        
        signal(SIGINT,closefile); 
        ///////////////////////////////////////////////////////////////////////////////////////
    }

    if (ret > 0) {
        printf("last event\n");
        show_event(buffer, ret);
	fwrite(buffer,ret,1,fpw);//////////////////////////////////////////Modify by Jingzhou Zhao
    }
	
    fclose(fpw);//////////////////////////////////////////Modify by Jingzhou Zhao
}

