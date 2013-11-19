
const int hv_status_revision = 1;

struct hv_status {
  byte8 mode;          // offline, online ....
  byte8 state;         // state
  byte8 reserve8[6];
  int   lasttime;
  int   reserve16[3];
  float mv[92]; // measured voltage
  float mc[92]; // measured current
};

