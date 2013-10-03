#ifndef _B2DAQ_run_status_h
#define _B2DAQ_run_status_h

const int run_status_revision = 1;

struct run_status {
  uint32 serial;
  uint32 exp_no;
  uint32 run_no;
  uint32 start_time;
  uint32 end_time;
  uint32 event_number;
  uint32 event_total;
};

#endif
