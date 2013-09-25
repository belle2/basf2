#ifndef _B2DAQ_run_status_h
#define _B2DAQ_run_status_h

const int run_status_revision = 1;

struct run_status {
  uint64 serial;
  uint64 exp_no;
  uint64 run_no;
  uint64 start_time;
  uint64 end_time;
  uint64 total_triggers;
};

#endif
