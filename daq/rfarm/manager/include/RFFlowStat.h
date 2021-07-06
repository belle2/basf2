/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef FLOWSTAT_H
#define FLOWSTAT_H

#include <sys/time.h>

#include "framework/pcore/RingBuffer.h"
#include "daq/rfarm/manager/RFSharedMem.h"
#include "daq/rfarm/manager/RfNodeInfo.h"

#define RF_INPUT_ID  51
#define RF_OUTPUT_ID 52
#define RF_ROI_ID 53

namespace Belle2 {

  class RFFlowStat {
  public:
    RFFlowStat(const char* shmname, int id, RingBuffer* rbuf);    // Constructor for fillter
    RFFlowStat(const char* shmname);    // Constructor for retriever
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
