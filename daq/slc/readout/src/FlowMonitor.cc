/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/readout/FlowMonitor.h"

#include <daq/slc/system/Time.h>

#include <cstring>

using namespace Belle2;

bool FlowMonitor::open(RunInfoBuffer* info)
{
  m_info = info;
  if (m_info->isAvailable()) {
    m_status.nodeid = m_info->getNodeId();
    m_nbyte[0] = m_nbyte[1] = 0;
    m_ioinfo[0].setLocalPort(m_info->getInputPort());
    m_ioinfo[0].setLocalAddress(m_info->getInputAddress());
    m_ip[0] = m_ioinfo[0].getLocalIP();
    m_ioinfo[1].setLocalPort(m_info->getOutputPort());
    m_ioinfo[1].setLocalAddress(m_info->getOutputAddress());
    m_ip[1] = m_ioinfo[1].getLocalIP();
  }
  return true;
}

bool FlowMonitor::close()
{
  return true;
}

ronode_status& FlowMonitor::monitor()
{
  if (m_info->isAvailable()) {
    m_ioinfo[0].setLocalAddress(m_info->getInputAddress());
    m_ioinfo[0].setLocalPort(m_info->getInputPort());
    m_ioinfo[1].setLocalAddress(m_info->getOutputAddress());
    m_ioinfo[1].setLocalPort(m_info->getOutputPort());
    IOInfo::checkTCP(m_ioinfo);
    unsigned int ctime = Time().getSecond();
    ronode_info info;
    memcpy(&info, m_info->get(), sizeof(ronode_info));
    memcpy(&(m_status.header), &(info.header), sizeof(event_header));
    double length = ctime - m_status.ctime;
    m_status.eflag = info.eflag;
    m_status.state = info.state;
    m_status.expno = info.expno;
    m_status.runno = info.runno;
    m_status.subno = info.subno;
    m_status.reserved_i[0] = info.reserved[0];
    m_status.reserved_i[1] = info.reserved[1];
    m_status.reserved_f[0] = info.reserved_f[0];
    m_status.reserved_f[1] = info.reserved_f[1];
    m_status.reserved_f[2] = info.reserved_f[2];
    m_status.reserved_f[3] = info.reserved_f[3];
    m_status.ctime = ctime;

    double dcount;
    double dnbyte;
    m_status.evtrate_in = 0;
    m_status.evtsize_in = 0;
    m_status.flowrate_in = 0;
    m_status.evtrate_out = 0;
    m_status.evtsize_out = 0;
    m_status.flowrate_out = 0;
    if (m_ioinfo[0].getLocalPort() > 0) {
      m_status.connection_in = m_ioinfo[0].getState() == 1;
      m_status.nqueue_in = m_ioinfo[0].getRXQueue();
    }
    if (m_ioinfo[1].getLocalPort() > 0) {
      m_status.connection_out = m_ioinfo[1].getState() == 1;
      m_status.nqueue_out = m_ioinfo[1].getRXQueue();
    }
    if ((dcount = info.io[0].count - m_status.nevent_in) > 0) {
      dnbyte = info.io[0].nbyte - m_nbyte[0];
      m_status.evtrate_in = dcount / length / 1000.;
      m_status.evtsize_in = dnbyte / dcount / 1000.;
      m_status.flowrate_in = dnbyte / length / 1000000.;
      m_status.nevent_in = info.io[0].count;
      m_nbyte[0] = info.io[0].nbyte;
    } else {
      m_status.evtrate_in = 0;
      m_status.evtsize_in = 0;
      m_status.flowrate_in = 0;
    }
    if ((dcount = info.io[1].count - m_status.nevent_out) > 0) {
      dnbyte = info.io[1].nbyte - m_nbyte[1];
      m_status.evtrate_out = dcount / length / 1000.;
      m_status.evtsize_out = dnbyte / dcount / 1000.;
      m_status.flowrate_out = dnbyte / length / 1000000.;
      m_status.nevent_out = info.io[1].count;
      m_nbyte[1] = info.io[1].nbyte;
    } else {
      m_status.evtrate_out = 0;
      m_status.evtsize_out = 0;
      m_status.flowrate_out = 0;
    }
  }
  return m_status;
}
