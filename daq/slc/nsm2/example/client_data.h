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
};
