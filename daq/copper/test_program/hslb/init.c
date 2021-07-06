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
static int FirmwareVersion = 0x000a;
static int HardwareVersion = 0x000a;
char *ARGV0  = 0;
char *DEVICE = 0;
unsigned int adr;
static struct  { char *name; int adrs; } regs[]= {
    { "trigger",   CONTROL},
    { "trghold",      CONTROL   },
    { "checkerr",    CONTROL },
    { "checkfee",    CONTROL   },
    { "link",  CONTROL  },
    { "linkrst",  CONTROL  },
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
  printf("usage: %s -(a,b,c,d)  <parameter(char)>\n",ARGV0);
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
  int val;
  int feetype,feehwv,feefwv;
  time_t   lt;
  int err_check,err_ini;

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
        if ( ( tem =  rdmgt( fd[i] , 0x71 ) )  != FirmwareVersion ) {
         fprintf(stderr,"HLSB Firmware version %.4x is not suitable for this applicaton\n",tem);
         exit(1);
       }
        if ( ( tem =  rdmgt( fd[i] , 0x70 ) )  != HardwareVersion ) {
         fprintf(stderr,"HLSB Hardware version %.4x is not suitable for this applicaton\n",tem);
         exit(1);
        } 
      }
   }
   if( !strcmp( argv[1] , "link" ) )
   {
     for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x01);
       wrmgt(fd[i], CONTROL, 0x0a);
       //printf("HSLB:%c\n",'a' + i);
      }
      exit(0); 
/*    k = 0;
    while(1)
     {
       usleep(100000);
       k++;
       printf("waited for %d00ms\n",k);
       if(k == 3){
         printf( "No response from :  ");
         for(i = 0 ; i < 4 ; i++) {
           if(readback[i] == 1)
             printf(" HSLB:%c ",'a' + i );
         }
         printf("\n");
         break;
       }
       for(i = 0; i < 4 ; i++ ){
         if(!use[i] || !readback[i])
            continue;
         val = rdmgt(fd[i], CONTROL );
            printf("value of CSR  on HSLB:%c : %x \n",'a'+ i ,val);
         if(val == 0x11)
           {
            readback[i] = 0;
            printf(" Link:%c is ready\n",'a' + i);
            }
        }
       if((readback[0]+readback[1]+readback[2]+readback[3]) == 0)
          break;
     }
      for(i=0; i<4; i++)
     {
       if(!use[i]) continue;
        printf("written %.2x to address %x on HSLB:%c\n",0x06,CONTROL,'a'+ i);
       wrmgt(fd[i], CONTROL, 0x06);
     }*/
   }
   if( !strcmp( argv[1] , "linkrst" ) )
   {
     for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x02);
       wrmgt(fd[i], CONTROL, 0x0a);
     // printf("HSLB:%c\n",'a' + i);
      }
   }
    else if( !strcmp( argv[1] , "checkerr" ) )
   {
       for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       lt=time(NULL);/*system   time   and   date*/ 
       printf(ctime(&lt));   /*english   format   output*/ 
       err_ini = rdmgt(fd[i], ERROR);
       printf("The number of error is %d when the application starts\n", err_ini);
       while(1)
       {
         sleep(100);
      //   printf("no error occourred\n");
         err_check = rdmgt(fd[i], ERROR);
         lt=time(NULL);/*system   time   and   date*/
         printf(ctime(&lt));   /*english   format   output*/
         if(err_check != err_ini)
         {      
             printf("a error occorred, the number of error is %d now\n",err_check);
             err_ini = err_check;
         }
       }
     // printf("HSLB:%c\n",'a' + i);
     }
   }
   else if( !strcmp( argv[1] , "trghold" ) )
   {
       for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x03);
       wrmgt(fd[i], CONTROL, 0x0a);
     // printf("HSLB:%c\n",'a' + i);
     }
   }
   else if( !strcmp( argv[1] , "realtrg" ) )
   {
       for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x05);
       wrmgt(fd[i], CONTROL, 0x0a);
     // printf("HSLB:%c\n",'a' + i);
     }
   }
   else if( !strcmp( argv[1] , "simtrg" ) )
   {
       for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x06);
       wrmgt(fd[i], CONTROL, 0x0a);
     // printf("HSLB:%c\n",'a' + i);
     }
   }
   else if( !strcmp( argv[1] , "trigger" ) )
   {
       for( i = 0 ; i < 4 ; i ++ ){
       if ( !use[i] ) continue;
       wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
       wrmgt(fd[i], CONTROL, 0x06);
       wrmgt(fd[i], FEE_CONTROL, 0x04);
       wrmgt(fd[i], CONTROL, 0x0a);
     // printf("HSLB:%c\n",'a' + i);
     }
   /*
    while(1)
     {

       usleep(100000);
       k++;
       printf("waited for %d00 ms\n",k);
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
         val = rdmgt(fd[i], CONTROL );
            printf("value of CSR  on HSLB:%c : %x \n",'a'+ i ,val);
         if(val == 0x33)
         {
            readback[i] = 0;
            printf("Link:%c trigger signal is released\n", 'a' + i);
         }
        }
       if((readback[0]+readback[1]+readback[2]+readback[3]) == 0)
          break;
       }
     for(i=0; i<4; i++)
     {
       if(!use[i]) continue;
        printf("written %.2x to address %x on HSLB:%c\n",0x06,CONTROL,'a'+ i);
       wrmgt(fd[i], CONTROL, 0x06);
     } */
   }
   else if( !strcmp( argv[1] , "checkfee" ) )
   {
    for(i= 0; i<4; i++) {
          if(!use[i]) continue;
          wrmgt(fd[i], CONTROL, 0x05); //reset address fifo
          wrmgt(fd[i], CONTROL, 0x06); //resert status register
          wrmgt(fd[i], FEE_TYPE, 0x02);
          wrmgt(fd[i], FEE_SERIAL, 0x02);
          wrmgt(fd[i], FEE_HWV, 0x02);
          wrmgt(fd[i], FEE_FWV, 0x02);
          wrmgt(fd[i], CONTROL, 0x07);
     //     printf("HSLB:%c\n",'a' + i);
          readback[i] = 1;
     }
    while(1)
     {

       usleep(100000);
       k++;
       printf("waited for %d00 ms\n",k);
       if(k == 3){
         printf( "No response from :  ");
         for(i = 0 ; i< 4 ; i++) {
           if(readback[i] == 1)
           {
             printf(" HSLB:%c ",'a' + i );
           }
         }
         printf("\n");
         exit(1);
       }
       for(i = 0; i < 4 ; i++ ){
         if(!use[i] || !readback[i])
            continue;
         val = rdmgt(fd[i], STATUS );
            printf("value of status register  on HSLB:%c : %x \n",'a'+ i ,val);
         if(val == 0x11)
            readback[i] = 0;
        }
       if((readback[0]+readback[1]+readback[2]+readback[3]) == 0)
          break;
     }
     for(i= 0; i<4; i++) {
          if(!use[i]) continue;
          feetype = rdmgt(fd[i], FEE_TYPE);
          switch (feetype)
           {
            case 0x01:
              printf("The type of FEE readout board which is linked to HSLB:%c is CDC\n",'a'+i);
              break;
            case 0x02:
              printf("The type of FEE readout board which is linked to HSLB:%c is ECL\n",'a'+i);
              break;
            default:
              printf("error type number : %x\n",feetype);
           }
          val = rdmgt(fd[i], FEE_SERIAL);
          printf("The serial# of FEE readout board which is linked to HSLB:%c is %x\n",'a'+i,val);
          feefwv = rdmgt(fd[i], FEE_FWV);
          printf("The version of the FEE readout board hardware which is linked to HSLB:%c is %x\n",'a'+i,feefwv);
          feehwv = rdmgt(fd[i], FEE_HWV);
          printf("The version of the FEE readout board firmware which is linked to HSLB:%c is %x\n",'a'+i,feehwv);
     }
     for(i=0; i<4; i++)
     {
       if(!use[i]) continue;
        printf("written %.2x to address %x on HSLB:%c\n",0x06,CONTROL,'a'+ i);
       wrmgt(fd[i], CONTROL, 0x06);
     } 
   }
   else
   {
     usage();
     exit(1);
   }
}  
  
