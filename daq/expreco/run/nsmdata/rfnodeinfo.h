const int rfnodeinfo_revision = 1;

struct rfnodeinfo {
  int32 sysstate;
  int32 nevent_in;
  int32 nqueue_in;
  int32 nevent_out;
  int32 nqueue_out;
  int32 error;
  int32 pid_input;
  int32 pid_output;
  int32 pid_basf2;
  int32 pid_hserver;
  int32 pid_hrelay;
  int32 i_reserved[4];
  float flowrate_in;
  float flowrate_out;
  float avesize_in;
  float avesize_out;
  float evtrate_in;
  float evtrate_out;
  float loadave;
  float r_reserved[9];
};




