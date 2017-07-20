#ifndef FLOWSTAT_H
#define FLOWSTAT_H
//+
// File : RFFlowStat.h
// Description : Calculate data flow statistics and put them in
//               RfNodeInfo structure
//
// Author : Ryosuke Itoh, KEK
// Date : 21 - Aug - 2013
//-

#include <sys/time.h>
#include <stdlib.h>

#include "framework/pcore/RingBuffer.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

#define RF_INPUT_ID  51
#define RF_OUTPUT_ID 52
#define RF_ROI_ID 53

namespace Belle2 {

  class RFFlowStat {
  public:
    RFFlowStat(char* shmname, int id, RingBuffer* rbuf);    // Constructor for fillter
    RFFlowStat(char* shmname);    // Constructor for retriever
    ~RFFlowStat();

    // functions for filler
    void log(int size);

    void clear(int id);

    // functions for retriever
    RfShm_Cell& getinfo(int id);
    void fillNodeInfo(int id, RfNodeInfo* info, bool inout);
    void fillProcessStatus(RfNodeInfo* info, int pid_input = 0, int pid_output = 0,
                           int pid_basf2 = 0, int pid_hserver = 0, int pid_hrelay = 0);

  private:
    RFSharedMem* m_rfshm;
    RfShm_Cell* m_cell;
    RingBuffer* m_rbuf;
    float m_flowsize;
    int m_nevtint;
    int m_interval;
    struct timeval m_t0;

  };
}
#endif
