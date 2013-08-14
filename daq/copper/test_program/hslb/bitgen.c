#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

int
main()
{
    unsigned int i = 0x00,j = 0;
    FILE *fp;
    char file[] = "test.dat";
    if (!(fp = fopen(file,"wb"))){
	printf("err\n");
	exit(1);
    }
    for (j = 0; j < 100; j++)
	{
	for (i = 0; i <= 0xFF; i++)
	   {
	    putc(i,fp); 
	    }
	i=0;
    }
    fclose(fp);
}
