///////////////////////////////////////////////////////////////////////// 
//Name:
//Purpose:
//Author:
//History:

//              Author:    Sun Dehui                                   //

//              Copyright: IHEP                                        // 
//              Version:   0.1                                         //
/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "cprfin_fngeneric.h"
#include "mgtf.h"
struct event {
        char temp0;
        char temp1;
        char temp2;
	char temp3;
	char temp4;
	char temp5;
        char flag;
        char temp6;
        };
char buffer[8];
char *ARGV0  = 0;
char *DEVICE = 0;
unsigned int adr;
int counter = 0;
/* ---------------------------------------------------------------------- *\
   usage
\* ---------------------------------------------------------------------- */
static void
usage()
{
  printf("usage: %S -(a,b,c,d) [option] value/filename\n",ARGV0);
  printf("option:\n");
  printf("list: list the avalid option\n");
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
  int buf1,buf2,buf3,buf4;
/* ---------------------------------------------------------------------- *\
   list of address
\* ---------------------------------------------------------------------- */
  static struct { char *name; int adrs; } regs[] = {
    { "window",   WINDOW},
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
   trans the file or set the regs
\* ---------------------------------------------------------------------- */
   while(argc > 1){
    	if(strcmp(argv[1],"file")==0)
	{
       		if(! (fp = fopen(argv[2],"rb"))) 
		{
          		printf("cannot open file: %s\n",argv[2]);
          		exit(1);
        	}
		else printf("%s\n",argv[2]);
	wrmgt(fd[4], TRANS, 0x9);
        wrmgt(fd[3], TRANS, 0x9);
   int ret;
   while(1){
   	  do {
        ret = fread( buffer, sizeof(char),8,fp);
          if (ret < 0)
	  {
              perror("read");
	      exit(0);
	  }
        } while (ret == -1 && errno == EINTR);
     if(ret == 0)
	{
	 wrmgt(fd[4], TRANS, 0x8);
         wrmgt(fd[3], TRANS, 0x8);
	printf("trans file successfully\n"); 
	break;
	}
	struct event *p = (struct event *) buffer;
	counter++;
	if( counter % 50000 == 0)
	    write( 2 , "." , 1);
     if ( ( p->flag & 0x1c ) == 0x08 || ( p->flag & 0x1c )==0x0c ) 
	{
		for(i = 0; i < 8; i++)
		{
			wrmgt(fd[4], 0x00, buffer[i]&0x000000ff);
			wrmgt(fd[3], 0x00, buffer[i]&0x000000ff);
		}

	}
     else if( ( p->flag & 0x60 ) == 0x40) 
		{
		for(i = 0; i < 8; i++)
			wrmgt(fd[4], 0x00, buffer[i]&0x000000ff);
		}
     else if(( p->flag & 0x60 ) == 0x20)
		{
		for(i = 0; i < 8; i++)
			wrmgt(fd[3], 0x00, buffer[i]&0x000000ff);
		}
     else
	{
		fprintf(stderr,"err of file reading\n");
		exit(0);
	}
   }	
/*       int count; 
       for(i=0; i<4; i++){
         if(!use[i]) continue;
           while((ch = getc(fp)) != EOF){
           wrmgt(fd[i], 0x02, ch);
           count++;
           if(count%1000000==0){
             printf("%d bytes written\n", count);
           }
         }
         printf("export %s to  HSLB:%c successfully\n",argv[2],'a'+i);
         rewind(fp);
       }*/
       fclose(fp); 
     } else {
         int val;
         if(argc <3) {
           usage();
           exit(1);
         } 
         for(i=0; i<sizeof(regs)/sizeof(regs[0]);i++) {
           if(!strcmp(regs[i].name, argv[1])) {
              adr = regs[i].adrs;
              goto aferr;}
         }
        printf("invalid address name: %s\n",argv[1]);
aferr:  val = strtoul(argv[2], 0, 16);
	for(i= 0; i<4; i++) {
	  if(!use[i]) continue;
          wrmgt(fd[i], adr, val);
	  printf("written %x to address %x on HSLB:%c\n",val,adr,'a'+ i); 
        }
     }
     argv += 2;
     argc -= 2;
   }
 //  int tr=0;
 //  for(i=0; i<4; i++) {
 //    if(!use[i]) continue;
 //    wrmgt(fd[i], TRANS, tr);}
}
