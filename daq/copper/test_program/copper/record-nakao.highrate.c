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
    /* printf("%08d %08x %08x %08x %08x %08x %08x %08x %08x\n", i, */
    /*     buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], */
    /*     buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]); */
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

int
main(int argc, char **argv)
{   
  FILE *fpr;  // for file read modified by Jingzhou Zhao
  char SaveFile[100];// SaveFile
  int irun;
  char timebuf[100];
  time_t t;//             

  int event=0;
  int ret, i = 0;
  fd_set rfds, efds;
  int amtfd[4];
  int iii;
  int istop;
  int istart;    
  int j=1;
  int card_test[4];
  int use_slot = 0; /* bit mask */
  char *program = argv[0];
  int tot_recvd_byte = 0;
  while (argc > 1) {
    if (argv[1][0] == '-') {
      int pos;
      for (pos=1; argv[1][pos] != 0; pos++) {
	int opt =  argv[1][pos];
	if (opt >= 'a' && opt <= 'd') {
	  use_slot |= (1 << (opt-'a'));
	} else {
	  printf("%s: unknown option %c\n", program, opt);
	  exit(1);
	}
      }
    }
    argc--, argv++;
  }
  if (! use_slot) {
    printf("usage: %s -[abcd] ...\n", program);
    exit(1);
  } else {
    int slot;
    printf("%s: slot", program);
    for (slot=0; slot<4; slot++) {
      if (use_slot & (1<<slot)) printf(" %c", 'A'+slot);
    }
    printf("\n");
  }

  time(&t);
  strftime(timebuf,sizeof(timebuf),"RUN%Y%m%d%H%M%S",localtime(&t));
  strcpy(SaveFile,timebuf);
  strcat(SaveFile,".dat");
  printf("File Name: %s\n",SaveFile);
 
#ifdef PRINT
  if((fpr=fopen(SaveFile,"r"))!=NULL) {
    printf("File: %s exists on the disk!",SaveFile);
    fclose(fpr);
    exit(0);
  }
  else {
    fpw=fopen(SaveFile,"wb");
  }
#endif


  cprfd = open("/dev/copper/copper", O_RDONLY);
  printf("cprfd= %d\n",cprfd);

  int set_regval = 100;
  ioctl(cprfd,CPRIOSET_LEF_WA_FF,&set_regval);
  ioctl(cprfd,CPRIOSET_LEF_WB_FF,&set_regval);
  ioctl(cprfd,CPRIOSET_LEF_WC_FF,&set_regval);
  ioctl(cprfd,CPRIOSET_LEF_WD_FF,&set_regval); 
  if (cprfd < 0) {
    perror("open");
    exit(0);
  }

  ret = ioctl(cprfd, CPRIOSET_FINESSE_STA, &use_slot, sizeof(use_slot));
  amtfd[0] = open("/dev/copper/fngeneric:a", O_RDWR);
  amtfd[1] = open("/dev/copper/fngeneric:b", O_RDWR);
  amtfd[2] = open("/dev/copper/fngeneric:c", O_RDWR);
  amtfd[3] = open("/dev/copper/fngeneric:d", O_RDWR); /* 201002261156 modified by sundehui  */

  printf("HSLB on COPPER:  %d:%d:%d:%d\n", amtfd[0], amtfd[1], amtfd[2], amtfd[3]);

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
      errno = 0;
      ret = read(cprfd, (char *)buffer, sizeof(buffer));
      if (ret < 0) perror("READ");
    } while (ret == -1 && errno == EINTR);

    tot_recvd_byte += ret;
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
#ifdef PRINT
      if(card_test[0]==1 & iii==0){
	fprintf(fpw,"Board A: %08x /*** Data length=%08x, Trigger#=%08x ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
      }
      if(card_test[1]==1 & iii==1){
	fprintf(fpw,"Board B: %08x /*** Data length=%08x, Trigger#=%08x ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
	printf("Board B: %08x /*** Data length=%08x, Trigger#=%08x ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
      }
      if(card_test[2]==1 & iii==2){
	fprintf(fpw,"Board C: %08x /*** Data length=%08x, Trigger#=%08x ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
      }
      if(card_test[3]==1 & iii==3){
	fprintf(fpw,"Board D: %08x /*** Data length=%08x, Trigger#=%08x ***/\n", buffer[istart + 1], buffer[istart + 2], buffer[istart + 3]);
      }
      printf("istart= %d ,istop = %d\n",istart, istop);
#endif
      for (t= istart + 4; t<istop-1; t= t + 1)
	{
 
	  if((t-istart-3)%48==0)
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
#ifdef PRINT
              fprintf(fpw,"%04x,%04x\n", temph,templ);
	      printf(".\n");
#endif
	    }
	  else
            {
              templ = buffer[t]&0xffff;
              temph = (buffer[t]>>16)&0xffff;
#ifdef PRINT
              fprintf(fpw,"%04x,%04x,", temph,templ);
	      printf("*");
#endif
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
      printf(" Event : %i  received, Footer: %x,  xor = %08x\n", event, footer->magic, xor((unsigned long *)buffer, ret/4));
      printf("ret= %x\n",ret);
      printf("bad fotter %x\n", footer->magic);
      show_event(buffer, ret);
      break;
    }

    if (header->event_number != event) { 
      printf("bad copper evn = %x should be %x\n", buffer[1], event);
    }


    event++;

    if( event % 10000 == 0 ){
      time_t timer;
      struct tm *t_st;
      time(&timer);
      t_st = localtime(&timer);

      printf(" Event : %i  received, xor = %08x total received %.2lf[MB] %s", event,  xor((unsigned long *)buffer, ret/4), tot_recvd_byte/1.e6, asctime(t_st));
    }
    if (event <= 10)  {/*   modified by sun */
#ifdef PRINT
      printf(" Event : %i  received, xor = %08x\n", event,  xor((unsigned long *)buffer, ret/4));
      printf("ret= %x\n",ret);
      show_event(buffer, ret);
#endif
    }     /*  modified by sun */

    //	fwrite(buffer,ret, 1, fpw);//////////////////////////////////////////Modify by Jingzhou Zhao

        
    /* if( event >4){ */
    /*     fclose(fpw); */
    /*     exit(1); */
    /* }; */
        
    signal(SIGINT,closefile); 
    ///////////////////////////////////////////////////////////////////////////////////////
  }

  if (ret > 0) {
    printf("last event\n");
    show_event(buffer, ret);
    fwrite(buffer,ret,1,fpw);//////////////////////////////////////////Modify by Jingzhou Zhao
  }

#ifdef PRINT	
  fclose(fpw);//////////////////////////////////////////Modify by Jingzhou Zhao
#endif
  return 0;
}

