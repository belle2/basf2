/*
  client_data sample to collect status and statistics
 */

const int client_data_revision = 1;

struct client_data {
  int32 is_running;
  int32 run_number;
  int32 run_count;
  int32 evt_number;
  int32 evt_total;
  int32 kn[10];
  char  sender_script[64];
  int32 sender_port;
  int32 sender_event_size;
  byte8 hslb_used[4];
  char  hslb_firmware[128];
  int32 window_a;
  int32 delay_a;
  int32 tdc_a;
  int32 adc_a;
  int32 pedestal_a[48];
  int32 window_b;
  int32 delay_b;
  int32 tdc_b;
  int32 adc_b;
  int32 pedestal_b[48];
  int32 window_c;
  int32 delay_c;
  int32 tdc_c;
  int32 adc_c;
  int32 pedestal_c[48];
  int32 window_d;
  int32 delay_d;
  int32 tdc_d;
  int32 adc_d;
  int32 pedestal_d[48];
};
