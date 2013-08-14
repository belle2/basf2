#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

char buffer[512 * 1024];

int lap_count = 1000;
int total_bytes = 0;

double
tv_diff(struct timeval tv1, struct timeval tv0)
{
    return
	(double)(tv1.tv_sec - tv0.tv_sec) +
	(double)(tv1.tv_usec - tv0.tv_usec)/(double)1000000.0;
}


main(int argc, char ** argv)
{
    struct timeval t0, t1;
    int nevent = 0;
    int fd = open("/dev/copper/copper", O_RDONLY); 
    int ch;

    assert(fd > 0);

    while (-1 != (ch = getopt(argc, argv, "l:"))) {
	switch (ch) {
	case 'l':
	    lap_count = strtol(optarg, 0, 0);
	}
    }

    while (1) {
	int event_size = read(fd, buffer, sizeof(buffer));
	assert(event_size > 0);

	total_bytes += event_size;

	if (nevent == 0)
	    printf("event_size = %d bytes\n", event_size);

	if (nevent % lap_count == 0) {
	    double tdiff;

	    gettimeofday(&t1, NULL);

	    tdiff = tv_diff(t1, t0);

	    printf("event# = %d, laptime = %f freq = %fHz %fMB %fMB/s\n",
		nevent,
		tdiff,
		(double) lap_count / tdiff,
		total_bytes / 1000000.0,
		total_bytes / (1000000.0 * tdiff)
		);

	    t0 = t1;
	    total_bytes = 0;
	}

	nevent ++;

    }
}
