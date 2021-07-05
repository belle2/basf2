/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/////////////////////////////////////////////////////////////////////////////////// 
//              Name:fileconfig.c                                                 // 
//              Purpose: configurate and readback the parameters on CDC FEE       // 
//              Author: Sun Dehui                                                 //                 
//              History: Nakao (regmgt.c)                                         //
//			 write only version  2010.10.12                           // 
//              Version:   0.2                                                    //
////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "cprfin_fngeneric.h"
#include "para.h"
static int FirmwareVersion = 0x000a;
char *ARGV0  = 0;
char *DEVICE = 0;
unsigned int adr;
static struct  { char *name; int adrs; } regs[]= {
    { "window",   WINDOW},
    { "csr",      CONTROL   },
    { "delay",    DELAY },
    { "ver",      FEE_FWV },
    { "feeserial",  FEE_SERIAL  },
     };//regs[] = {
//    { "window",   WINDOW},
//    { "csr",      CSR   },
//    { "delay",    DELAY },
//    { "ver",      VER   },
//    { "file",     CONFILE},
//  };
/* ---------------------------------------------------------------------- *\
   usage
\* ---------------------------------------------------------------------- */
static void
usage()
{ 
  int m;
  printf("usage: %s -(a,b,c,d) -[r/w] <parameter(char)> [<value(hex)>]\n",ARGV0);
  printf("valid parameter options : ");
  for(m = 0 ; m < sizeof(regs)/sizeof(regs[0]);m++)
     printf("  %s  ",regs[m].name);
  printf("\n");
}


/* ---------------------------------------------------------------------- *\
   rdmgt
\* ---------------------------------------------------------------------- */
int
rdmgt(int fd, int adr)
{
  int val = 0;
  if (ioctl(fd, FNGENERICIO_GET(adr), &val) < 0) {
    fprintf(stderr, "%s: cannot read %s: %s\n",
            ARGV0, DEVICE, strerror(errno));
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
    fprintf(stderr, "%s: cannot write %s: %s\n",
            ARGV0, DEVICE, strerror(errno));
    exit(1);
  }
}

int
main(int argc, char **argv)
{
  int fd[4];
  int use[4];
  int readback[4];
  int i, j;
  FILE *fp;
  int ch;
  int tem;
  int k = 0, l;
  readback[0] = readback[1] = readback[2] = readback [3] = 0;
  use[0] = use[1] = use[2] = use[3] = 0;
  fd[0]  = fd[1]  = fd[2]  = fd[3]  = -1;
  ARGV0=argv[0];
    for (i = 1; argv[1][i]; i++) {
      DEVICE = 0;
      if (! strchr("abcd", argv[1][i])) {
        argc = -1;
        break;
      }
      
      j = argv[1][i] - 'a';
      if (use[j]) {
        fprintf(stderr, "HSLB %c is specified twice\n", 'a'+j);
        exit(1);
      }
      use[j] = 1;
    }
    argv++, argc--;

  if (argc < 2 || (use[0]+use[1]+use[2]+use[3]) == 0) {
    usage();
    exit(1);
  }
/* ---------------------------------------------------------------------- *\
   check the device
\* ---------------------------------------------------------------------- */   
   for (i=0; i<4; i++) {
     if (! use[i]) continue;
     if (fd[i] < 0) {
       char DEVICE[256];
       sprintf(DEVICE, "/dev/copper/fngeneric:%c", 'a' + i);
       if ((fd[i] = open(DEVICE,O_RDWR)) < 0) {
         fprintf(stderr, "%s: cannot open %s: %s\n",
                  ARGV0, DEVICE, strerror(errno));
         exit(1);
        }
        if ( ( tem =  rdmgt( fd[i] , 0x7d ) )  != FirmwareVersion ) {
         fprintf(stderr,"HLSB Firmware version %.4x is not suitable for this applicaton\n",tem);
         exit(1);
       } 
      }
   } 
/* ---------------------------------------------------------------------- *\
   read or  set the parameters
\* ---------------------------------------------------------------------- */
   if(argv[1][1] == 'r')
   {  
      argv++;
      argc--;
      int val;
      for(k = 1 ; k < argc ; k++)
      {
	for(i=0; i<sizeof(regs)/sizeof(regs[0]);i++) {
           if(!strcmp(regs[i].name, argv[k])) {
              adr = regs[i].adrs;
              break;}
         }
//       printf("i=%d,length of regs = %d ", i ,sizeof(regs)/sizeof(regs[0]));
       if (i == sizeof(regs)/sizeof(regs[0]))
       {
          for(i= 0; i<4; i++) \
          {
             if(!use[i]) continue;
             wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
          }
           usage();
           exit(1);
       }
        for(i= 0; i<4; i++) {
          if(!use[i]) continue;
	  wrmgt(fd[i], adr, 0x02);
        }
      }
      for(i= 0; i<4; i++) {
          if(!use[i]) continue;
          wrmgt(fd[i], CONTROL, 0x07);
     //     printf("HSLB:%c\n",'a' + i);
	  readback[i] = 1; }
   //  printf("befor while\n");
     k = 0;
     while(1)
     {
       
       usleep(1000000);
       k++;
       printf("waited for %d second\n",k);
       if(k == 3){
         printf( "No response from :  ");
         for(i = 0 ; i< 4 ; i++) {
           if(readback[i] == 1) 
             printf(" HSLB:%c ",'a' + i );
         }
         printf("\n");
         break;
       }
       for(i = 0; i < 4 ; i++ ){
	 if(!use[i] || !readback[i]) 
	    continue;
	 val = rdmgt(fd[i], 0x02 );
            printf("value of CSR  on HSLB:%c : %x \n",'a'+ i ,val);
         if(val == 0x11)
	    readback[i] = 0;
        }
       if((readback[0]+readback[1]+readback[2]+readback[3]) == 0)
          break;
       }
  
       while(argc > 1){
         int val;
         if(argc <2) {
           usage();
           exit(1);
         }
         for(i=0; i<sizeof(regs)/sizeof(regs[0]);i++) {
           if(!strcmp(regs[i].name, argv[1])) {
              adr = regs[i].adrs;
              break;}
         }
        for(i= 0; i<4; i++) {
          if(!use[i]) continue;
          val = rdmgt(fd[i], adr);
          printf("value of parameter  %s on HSLB:%c : %x \n",argv[1],'a'+ i ,val);
        }
      argv++;
      argc--; 
      } 
     for(i=0; i<4; i++)
     {
       if(!use[i]) continue;
        printf("written %.2x to address %x on HSLB:%c\n",0x06,CONTROL,'a'+ i);
       wrmgt(fd[i], CONTROL, 0x06);
     }   
   }
   else if(argv[1][1] == 'w')
     {
      argc--;
      argv++;   
      while(argc > 1){
         int val;
         if(argc <3) {
           usage();
           exit(1);
         }
         for(i=0; i<sizeof(regs)/sizeof(regs[0]);i++) {
           if(!strcmp(regs[i].name, argv[1])) {
              adr = regs[i].adrs;
              break;}
         }
         if (i == sizeof(regs)/sizeof(regs[0]))
        { 
           usage();
           exit(1);
         } 
        val = strtoul(argv[2], 0, 16);
	for(i= 0; i<4; i++) {
	  if(!use[i]) continue;
          wrmgt(fd[i], adr, val);
	  printf("written %.2x to address %x on HSLB:%c\n",val,adr,'a'+ i); 
        }
     
     argv += 2;
     argc -= 2;
    }
   
  
     for(i=0; i<4; i++) 
     {
       if(!use[i]) continue;
        printf("written %.2x to address %x on HSLB:%c\n",0x0a,CONTROL,'a'+ i);
       wrmgt(fd[i], CONTROL, 0x0a);
     }
   }
   else 
   {
     usage();
     exit(1);
   }
}
