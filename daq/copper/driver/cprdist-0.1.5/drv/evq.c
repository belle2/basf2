#include "tailq.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/pci.h>
#include <asm/uaccess.h>

#define	printf(...)	printk("<1>" __VA_ARGS__)

#define	assert(A)	((A) || printk("<1>" #A))

#define	NTQ		1024	/* event queue for reading */

#include "evq.h"

static struct evqent {
    struct event ent[NTQ];
    struct evqent * next;
} *res = NULL;

static TAILQ_HEAD(tailhead, event) ready, empty;

void
clear_event_queue(void)
{
    struct event * e;
    while (e = ready.tqh_first) {
	TAILQ_REMOVE(&ready, e, tq);
	TAILQ_INSERT_TAIL(&empty, e, tq);
    }

}

struct evqent *
extend_event_queue(void)
{
    int i;
    struct evqent *p, *n;

    // printf("evq extend %d start\n", NTQ);

    n = res;
    p = (struct evqent *)kmalloc(sizeof(struct evqent), GFP_ATOMIC);
    if (p == NULL)
	return 0;

    res = p;
    p->next = n;

    for (i=0; i<NTQ; i++) {
	TAILQ_INSERT_TAIL(&empty, &(p->ent[i]), tq);
    }
    // printf("evq extend end\n");
}

int
allocate_event_queue(void)
{
    int i;
    TAILQ_INIT(&ready);
    TAILQ_INIT(&empty);
#if 1
    for (i=0; i<32; i++) {
	if (0 == extend_event_queue())
	    return 0;
    }
    return 1;
#else
    return (0 != extend_event_queue());
#endif
}

void
free_event_queue(void)
{
    struct evqent * n;
    while (res) {
	// printf("free_event_queue res=%p\n", res);
	n = res->next;
	kfree(res);
	res = n;
    }
}


/*
 * size should be in word 
 */
int
enqueue_events(unsigned long * start, int size, int bufno)
{
    int i=0;
    int found_event = 0;
    struct event * e;
    struct hdr {
	unsigned int magic;
	unsigned int total;
	unsigned int size[4];
    } * p;

    extern int debug_flag;

#define DEBUG_BAD_EVENT         (1<<10)

    while (size > 0) {
	p = (struct hdr *)start;
	if (p->magic != 0xFFFFFAFA ||
	    p->total != p->size[0] + p->size[1] + p->size[2] + p->size[3] + sizeof(struct hdr)/sizeof(int) + 1 ||
	    start[p->total - 1] != 0xFFFFF5F5) {

	    if ((debug_flag & DEBUG_BAD_EVENT) || (found_event > 0)) {
		printf("bad event found %d in remaining %d word\n", found_event, size);
		printf("magic = %08x\n", p->magic);
		printf("total = %08x\n", p->total);
		printf("s0 = %08x\n", p->size[0]);
		printf("s1 = %08x\n", p->size[1]);
		printf("s2 = %08x\n", p->size[2]);
		printf("s3  = %08x\n", p->size[3]);
	    }

	    if (found_event > 0 && size < 100) {
		int j;
		for (j=0; j<100; j++) {
		    printf("%3d %08x\n", j, start[j]);
		}
	    }

	    break;
	}

	found_event ++;

	e = empty.tqh_first;
	if (e == NULL) {
	    printf("event queue overflow (%d)\n", i);
	    return i;
	}

	TAILQ_REMOVE(&empty, e, tq);
	TAILQ_INSERT_TAIL(&ready, e, tq);
	if (empty.tqh_first == NULL) {
	    if (extend_event_queue() == NULL) {
		printf("extend failed\n");
	    }
	}

	e->start = (unsigned char *)p;
	e->size  = p->total * sizeof(int);
	e->bufno = bufno;

	i++;

	start += p->total;
	size  -= p->total;
    }


    //printf("%d events enqueue success\n", i);
    return i;
}

struct event *
peek_event(void)
{
    return ready.tqh_first;
}

void
dequeue_event(struct event * e)
{
    if (e) {
	TAILQ_REMOVE(&ready, e, tq);
	TAILQ_INSERT_TAIL(&empty, e, tq);
    }
}

void
show_first_event(void)
{
    int i;
    struct event * e = peek_event();
    printf(" *** \n");
    for (i=0; i < e->size ; i+= 4) { 
	printf("%08lx\n", *(unsigned long *)(e->start + i));
    }
}

void
show_all_events(void)
{
    int i, j=0;
    struct event * e = peek_event();

    while (e) {
	printf(" *** %d ***\n", j++);
	printf(" start = %p\n", e->start);
	printf(" size  = %08x\n", e->size);
	for (i=0; i < e->size ; i+= 4) { 
	    printf("%08lx\n", *(unsigned long *)(e->start + i));
	}
	e = e->tq.tqe_next;
    }
}


int
count_event_queue(void)
{
    int i = 0;
    struct event * e = ready.tqh_first;
    while (e) {
	e = e->tq.tqe_next;
	++i;
    }
    return i;
}

int
count_empty_queue(void)
{
    int i = 0;
    struct event * e = empty.tqh_first;
    while (e) {
	e = e->tq.tqe_next;
	++i;
    }
    return i;
}

#ifndef __KERNEL__
main(int argc, char ** argv)
{
    int fd;
    int ret;
    struct stat s;
    char * buf;
    int events;
    struct event * e;
    int read_count;

    if (argc < 2)
	exit(0);
    ret = stat(argv[1], &s);
    printf("stat (ret=%d) size=%d\n", ret, s.st_size);

    if (ret != 0)
	exit(0);

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
	perror("open");
	exit(0);
    }

    allocate_event_queue();

    buf = malloc(s.st_size);
    ret = read(fd, buf, s.st_size);
    if (ret != s.st_size)
	exit(0);
    printf("data file reading is done, check start....\n");

    clear_event_queue();

    events = enqueue_events((unsigned long *)buf, s.st_size / sizeof(int), 0);
    printf("enqueue returned %d\n", events);

    read_count = events;

    while (e = peek_event()) {
	printf("read_count=%d start = %x size = %d\n", read_count, e->start, e->size);
	dequeue_event(e);
	read_count --;
    }
    printf("read_count = %d\n", read_count);

    free_event_queue();
}
#endif
