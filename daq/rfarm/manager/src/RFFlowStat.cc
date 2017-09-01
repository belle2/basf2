//+
// File : RFFlowStat.cc
// Description : Log flow statistics
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 21 - Aug - 2013
//-

#include "daq/rfarm/manager/RFFlowStat.h"

using namespace Belle2;
using namespace std;

RFFlowStat::RFFlowStat(char* shmname, int id, RingBuffer* rbuf)
{
  m_rfshm = new RFSharedMem(shmname);
  m_cell = &(m_rfshm->GetCell(id));
  m_rbuf = rbuf;
  m_flowsize = 0.0;
  m_nevtint = 0;
  m_interval = 100;
  gettimeofday(&m_t0, 0);
}

RFFlowStat::RFFlowStat(char* shmname)
{
  m_rfshm = new RFSharedMem(shmname);
}

RFFlowStat::~RFFlowStat()
{
}

// Filler function
void RFFlowStat::log(int size)
{
  struct timeval tnow;

  m_cell->nevent++;
  if (m_rbuf != NULL)
    m_cell->nqueue = m_rbuf->numq();
  else
    m_cell->nqueue = 0;
  m_flowsize += (float)size;
  m_nevtint++;
  if (m_cell->nevent % m_interval == 0) {
    gettimeofday(&tnow, 0);
    float delta = (float)((tnow.tv_sec - m_t0.tv_sec) * 1000000 +
                          (tnow.tv_usec - m_t0.tv_usec));
    m_cell->flowrate = m_flowsize / delta;
    //    m_cell->flowrate = delta;
    m_cell->avesize = m_flowsize / (float)m_nevtint / 1000.0;
    m_cell->evtrate = (float)m_nevtint / delta * 1000000.0;
    m_cell->elapsed = time(NULL);
    m_flowsize = 0.0;
    m_nevtint = 0;
    m_t0 = tnow;
  }
}
void RFFlowStat::clear(int cellid)
{
  RfShm_Cell& cell = getinfo(cellid);
  cell.nevent = 0;
  cell.nqueue = 0;
  cell.flowrate = 0;
  cell.avesize = 0;
  cell.evtrate = 0;
  cell.elapsed = 0;
}

// Retriever function
RfShm_Cell& RFFlowStat::getinfo(int id)
{
  return m_rfshm->GetCell(id);
}

void RFFlowStat::fillNodeInfo(int id, RfNodeInfo* info, bool outflag)
{
  RfShm_Cell& cell = getinfo(id);
  //DEBUG  printf ( "fillNodeInfo: celleid = %d, nevent = %d\n", id, cell.nevent );
  if (!outflag) {
    info->nevent_in = cell.nevent;
    info->nqueue_in = cell.nqueue;
    info->flowrate_in = cell.flowrate;
    info->avesize_in = cell.avesize;
    info->evtrate_in = cell.evtrate;
  } else {
    info->nevent_out = cell.nevent;
    info->nqueue_out = cell.nqueue;
    info->flowrate_out = cell.flowrate;
    info->avesize_out = cell.avesize;
    info->evtrate_out = cell.evtrate;
  }
  double loads[3];
  if (getloadavg(loads, 3) > 0)
    info->loadave = (float)loads[0];

  /* Test
  info->nevent_in = 99999;
  info->nqueue_in = 123;
  info->flowrate_in = 10.0;
  info->avesize_in = 100.0;
  info->evtrate_in = 1000.0;
  */

}

void RFFlowStat::fillProcessStatus(RfNodeInfo* info, int input, int output, int basf2, int hserver, int hrelay)
{
  info->pid_input = input;
  info->pid_output = output;
  info->pid_basf2 = basf2;
  info->pid_hserver = hserver;
  info->pid_hrelay = hrelay;
}






