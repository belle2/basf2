#ifndef _Belle2_ArichHVStatus_h
#define _Belle2_ArichHVStatus_h

const int ArichHVStatus_revision = 1;

struct ArichHVStatus {
  byte8  status[4];
  uint32 voltage_mon[4];
  uint32 current_mon[4];
  uint32 voltage_demand[4];
  uint32 rampup_speed[4];
  uint32 rampdown_speed[4];
  uint32 voltage_limit[4];
  uint32 current_limit[4];
};

#endif
