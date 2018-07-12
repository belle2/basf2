/*
  bridge_data sample to collect status and statistics
 */

const int bridge_data_revision = 1;

struct bridge_data {
  /* state of the bridge program */
  int32 state;

  /* minimum among multiple clients */
  int32 run_number_min;
  int32 run_count_min;
  int32 evt_number_min;
  int32 evt_total_min;

  /* maximum among multiple clients */
  int32 run_number_max;
  int32 run_count_max;
  int32 evt_number_max;
  int32 evt_total_max;
};
