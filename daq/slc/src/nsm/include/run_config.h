#ifndef _B2DAQ_run_config_h
#define _B2DAQ_run_config_h

const int run_config_revision = 1;

struct run_config {
  uint32 nused;
  byte8  used[1024];
  int32  version;
  char   run_type[256];
  char   operators[256];
  int32 trigger_mode;
  int32 dummy_rate;
  int32 trigger_limit;
};

#endif
