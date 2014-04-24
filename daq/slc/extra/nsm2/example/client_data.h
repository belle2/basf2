/*
  client_data sample to collect status and statistics
 */

const int client_data_revision = 4;

#define NAMESIZ 16
#define DIM 4

struct client_data {
  int32 is_running;
  char  multidim[DIM][DIM][86]; /* 1392 bytes to make nodes in the boundary */
  char  mynode[NAMESIZ];
  int32 run_number;
  int32 run_count;
  struct node {
    char nodename[NAMESIZ];
    int32 evt_number;
  } nodes[4];
  int32 evt_total;
};
