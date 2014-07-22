#include "daq/slc/readout/ReadoutMonitor.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"
#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

void ReadoutMonitor::run()
{
  /*
  int interval = 2;
  ronode_info info;
  std::vector<IOInfo> ioinfo;
  ioinfo.push_back(IOInfo());
  ioinfo.push_back(IOInfo());
  while (true) {
    sleep(interval);
    memcpy(&info, m_info, sizeof(ronode_info));
    int ctime = Time().get();
    float length = ctime - m_status->ctime;
    m_status->expno = info.expno;
    m_status->runno = info.runno;
    m_status->subno = info.subno;
    m_status->ctime = ctime;
    unsigned int dcount[2];
    float dnbyte[2];
    for (int i = 0; i < 2; i++) {
      ioinfo[i].setLocalAddress(info.io[i].addr);
      ioinfo[i].setLocalPort(info.io[i].port);
    }
    IOInfo::checkTCP(ioinfo);
    for (int i = 0; i < 2; i++) {
      if (info.io[i].port == 0) {
        m_status->io[i].port = 0;//not used
      } else if (info.io[i].port < 0) {
        if (m_status->io[i].port > 0) {
          m_callback->getCommunicator()->sendError("Lost connection");
        }
        m_status->io[i].port = -1;//offline
      } else {
        if (ioinfo[i].getState() == 1) {
          m_status->io[i].port = info.io[i].port;
        } else {
          if (m_status->io[i].port > 0) {
            m_callback->getCommunicator()->sendError("Lost connection");
          }
          m_status->io[i].port = -1;//offline
        }
      }
      if (m_status->io[i].port > 0) {
        if ((dcount[i] = info.io[i].count - m_status->io[i].count) > 0) {
          dnbyte[i] = info.io[i].nbyte - m_status->io[i].nbyte;
          m_status->io[i].freq = dcount[i] / length / 1000.;
          m_status->io[i].evtsize = dnbyte[i] / dcount[i] / 1000.;
          m_status->io[i].rate = dnbyte[i] / length / 1000000.;
          m_status->io[i].count = info.io[i].count;
          m_status->io[i].nbyte = info.io[i].nbyte;
    if (i == 0) {
      m_status->io[i].nqueue = ioinfo[i].getRXQueue();
    } else {
      m_status->io[i].nqueue = ioinfo[i].getTXQueue();
    }
        }
      } else {
        m_status->io[i].freq = 0;
        m_status->io[i].evtsize = 0;
        m_status->io[i].rate = 0;
        m_status->io[i].count = 0;
        m_status->io[i].nbyte = 0;
  m_status->io[i].nqueue = 0;
      }
    }

    if (m_status->io[0].port < 0 || m_status->io[1].port < 0) {
      m_status->state = 2;//not ready
    } else if (dcount[0] > 0 || dcount[1] > 0) {
      m_status->state = 4;//running
    } else if (info.state >= RunInfoBuffer::READY) {
      m_status->state = 3;//ready
    } else {
      m_status->state = 1;//unknown
    }
    //LogFile::debug("state =%d", m_status->state);
  }
  */
}
