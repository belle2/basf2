#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "copper.h"

main(int argc, char ** argv)
{
    int i;
    int cmd[] = {
	CPRIOSET_LEF_WA_AF,
	CPRIOSET_LEF_WB_AF,
	CPRIOSET_LEF_WC_AF,
	CPRIOSET_LEF_WD_AF,
	0,
    };
    int fd = open("/dev/copper/copper_ctl", O_RDWR);
    int v = 511;

    if (fd == -1) {
	perror("open /dev/copper/copper_ctl:");
	exit(1);
    }
    if (argc > 1) {
	v = strtol(argv[1], 0, 0);
    }

    for (i=0; cmd[i]; i++) {
	int ret = ioctl(fd, cmd[i], &v, sizeof(v));
	if (ret) {
	    perror("ioctl");
	    exit(1);
	}
	printf("set %c to %d\n", 'A'+i, v);
    }
    exit(0);
}
