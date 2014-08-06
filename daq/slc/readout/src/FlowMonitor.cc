#include "daq/slc/readout/FlowMonitor.h"

#include "daq/slc/system/LogFile.h"
#include <daq/slc/system/Time.h>

#include <cstring>

using namespace Belle2;

bool FlowMonitor::open(RunInfoBuffer* info)
{
  m_info = info;
  m_status.nodeid = m_info->getNodeId();
  m_nbyte[0] = m_nbyte[1] = 0;
  m_ioinfo[0].setLocalPort(m_info->getInputPort());
  m_ioinfo[0].setLocalAddress(m_info->getInputAddress());
  m_ip[0] = m_ioinfo[0].getLocalIP();
  LogFile::debug("in host:port=%s:%d", m_ip[0].c_str(),
                 m_ioinfo[0].getLocalPort());
  m_ioinfo[1].setLocalPort(m_info->getOutputPort());
  m_ioinfo[1].setLocalAddress(m_info->getOutputAddress());
  m_ip[1] = m_ioinfo[1].getLocalIP();
  LogFile::debug("out host:port=%s:%d", m_ip[1].c_str(),
                 m_ioinfo[1].getLocalPort());
  return true;
}

bool FlowMonitor::close()
{
  return true;
}

ronode_status& FlowMonitor::monitor()
{
  m_ioinfo[0].setLocalAddress(m_info->getInputAddress());
  m_ioinfo[0].setLocalPort(m_info->getInputPort());
  m_ioinfo[1].setLocalAddress(m_info->getOutputAddress());
  m_ioinfo[1].setLocalPort(m_info->getOutputPort());
  IOInfo::checkTCP(m_ioinfo);
  int ctime = Time().get();
  ronode_info info;
  memcpy(&info, m_info->get(), sizeof(ronode_info));
  float length = ctime - m_status.ctime;
  m_status.eflag = info.eflag;
  m_status.state = info.state;
  m_status.expno = info.expno;
  m_status.runno = info.runno;
  m_status.subno = info.subno;
  m_status.ctime = ctime;

  unsigned int dcount[2];
  float dnbyte[2];
  for (int j = 0; j < 2; j++) {
    m_status.io[j].freq = 0;
    m_status.io[j].evtsize = 0;
    m_status.io[j].rate = 0;
    if (m_ioinfo[j].getLocalPort() > 0) {
      m_status.io[j].state = m_ioinfo[j].getState();
      if (j == 0) {
        m_status.io[j].nqueue = m_ioinfo[j].getRXQueue();
      } else {
        m_status.io[j].nqueue = m_ioinfo[j].getTXQueue();
      }
    }
    if ((dcount[j] = info.io[j].count - m_status.io[j].count) > 0) {
      dnbyte[j] = info.io[j].nbyte - m_nbyte[j];
      m_status.io[j].freq = dcount[j] / length / 1000.;
      m_status.io[j].evtsize = dnbyte[j] / dcount[j] / 1000.;
      m_status.io[j].rate = dnbyte[j] / length / 1000000.;
      m_status.io[j].count = info.io[j].count;
      m_nbyte[j] = info.io[j].nbyte;
    } else {
      m_status.io[j].freq = 0;
      m_status.io[j].evtsize = 0;
      m_status.io[j].rate = 0;
    }
  }
  return m_status;
}
