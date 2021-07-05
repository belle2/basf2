/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/////////////////////////////////////////////////////////////////////// 
//		Name:fileconfig.c				     //	
//		Purpose: configurate the parameters on CDC FEE 	     //	
//		Author: Sun Dehui				     //			
//		History: Nakao (mgtreg.c)			     //	
//		Version:   0.1                                       //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include "cprfin_fngeneric.h"
#include "para.h"

char *ARGV0  = 0;
char *DEVICE = 0;
unsigned int adr;
/* ---------------------------------------------------------------------- *\
   usage
\* ---------------------------------------------------------------------- */
static void
usage()
{
  printf("usage: %s -(a,b,c,d) filename\n",ARGV0);
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
  int i, j;
  FILE *fp;
  int ch;
/* ---------------------------------------------------------------------- *\
   list of address
\* ---------------------------------------------------------------------- */
  static struct { char *name; int adrs; } regs[] = {
    { "window",   WINDOW},
    { "csr",      CSR   },
    { "delay",    DELAY },
    { "ver",      VER   },
  };
/* ---------------------------------------------------------------------- *\
   check the command
\* ---------------------------------------------------------------------- */
  use[0] = use[1] = use[2] = use[3] = 0;
  fd[0]  = fd[1]  = fd[2]  = fd[3]  = -1;
  ARGV0=argv[0]; 
  while (argc >= 2 && argv[1][0] == '-' && argv[1][1] != 0) {
    for (i = 1; argv[1][i]; i++) {
      DEVICE = 0;
      if (! strchr("abcd", argv[1][i])) {
        argc = -1;
        break;
      }
      
      j = argv[1][i] - 'a';
      if (use[j]) {
        fprintf(stderr, "FINESSE %c is specified twice\n", 'a'+j);
        exit(1);
      }
      use[j] = 1;
    }
    argv++, argc--;
  }

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
      }
   }
 
/* ---------------------------------------------------------------------- *\
   trans the configure file
\* ---------------------------------------------------------------------- */

       if(! (fp = fopen(argv[1],"r"))) {
          printf("cannot open file: %s\n",argv[1]);
          exit(1);
       }
       int count = 0; 
       
       for(i=0; i<4; i++){
         if(!use[i]) continue;
	 wrmgt(fd[i], CSR ,0x09); // ACK  for start of file
         while((ch = getc(fp)) != EOF){
           wrmgt(fd[i], CONFILE, ch);
           count++;
           if(count%1000000==0){
             printf("%d bytes written\n", count);
           }
         }
         printf("export %s to  HSLB:%c successfully\n",argv[1],'a'+i);
	 wrmgt(fd[i], CSR, 0x08);  //ACK  for end of file
         rewind(fp);
       }
       fclose(fp);
} 

