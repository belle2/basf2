#ifndef _EVQ_H_
#define _EVQ_H_
/* tail queue item for event reading */
struct event {
  unsigned char* start;
  int size;
  int bufno;
  TAILQ_ENTRY(event) tq;     /* Tail queue. */
};

#ifdef __KERNEL__
struct event* peek_event(void);
#endif

extern void clear_event_queue(void);
extern int count_event_queue(void);
extern int count_empty_queue(void);
extern void dequeue_event(struct event* e);
extern int enqueue_events(unsigned long* start, int size, int bufno);
extern void free_event_queue(void);
extern int allocate_event_queue(void);
#endif
