#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

char buffer_in[512 * 1024];
char buffer_out[8 * 1024 * 1024];

int lap_count = 1000;
int total_bytes = 0;

double
tv_diff(struct timeval tv1, struct timeval tv0)
{
    return
	(double)(tv1.tv_sec - tv0.tv_sec) +
	(double)(tv1.tv_usec - tv0.tv_usec)/(double)1000000.0;
}


main()
{
    struct timeval t0, t1;
    int nevent = 0;
    int fd = open("/dev/copper/copper", O_RDONLY); 
    int ret;

    assert(fd > 0);

    setvbuf(stdout, buffer_out, sizeof(buffer_out), _IOFBF);

    while (1) {
	int event_size = read(fd, buffer_in, sizeof(buffer_in));
	assert(event_size > 0);

	ret = fwrite(buffer_in, event_size, 1, stdout);
	assert(ret == 1);

	total_bytes += event_size;

	if (nevent == 0)
	    fprintf(stderr, "event_size = %d bytes\n", event_size);

	if (nevent % lap_count == 0) {
	    double tdiff;

	    gettimeofday(&t1, NULL);

	    tdiff = tv_diff(t1, t0);

	    fprintf(stderr, "event# = %d, laptime = %f freq = %fHz %fMB %fMB/s\n",
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
