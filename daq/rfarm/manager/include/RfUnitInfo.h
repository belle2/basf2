#ifndef RFUNITINFO_H
#define RFUNITINFO_H
//+
// File : RfUnitInfo.h
// Description : Unit information format file for NSM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - June - 2013
//-

// Node info placed in NSM shared memory
namespace Belle2 {

  struct RfUnitInfo {
    struct RfNodeInfo {
      int sysstate;
      int nevent_in;
      int nqueue_in;
      int nevent_out;
      int nqueue_out;
      int error;
      int i_reserved[9];
      float flowrate_in;
      float flowrate_out;
      float avesize_in;
      float avesize_out;
      float evtrate_in;
      float evtrate_out;
      float loadave;
      float r_reserved[9];
    } nodeinfo[24];
    unsigned int nnodes;
    unsigned int updatetime;
    int reserved[2];
  };

}
#endif
