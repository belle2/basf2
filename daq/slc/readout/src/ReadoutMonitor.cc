#include "daq/slc/readout/ReadoutMonitor.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace Belle2;

int ReadoutMonitor::checkConnection(const std::string& proc_name, int port)
{
  //std::string cmd =
  //  StringUtil::form("/usr/sbin/lsof -a -c '%s' -i TCP:%d | "
  //                   "grep ESTABLISHED | wc -l",
  //                   proc_name.c_str(), port);
  std::string cmd =
    StringUtil::form("/usr/sbin/lsof -a -i TCP:%d | "
                     "grep ESTABLISHED | wc -l", port);
  FILE* file = popen(cmd.c_str(), "r");
  char str[1024];
  memset(str, '\0', 1024);
  fread(str, 1, 1024 - 1, file);
  pclose(file);
  std::string s = str;
  return atoi(s.c_str());
}

void ReadoutMonitor::run()
{
  int interval = 2;
  ronode_info info;
  while (true) {
    sleep(interval);
    memcpy(&info, m_info, sizeof(ronode_info));
    int ctime = Time().get();
    float length = ctime - m_status->ctime;
    m_status->expno = info.expno;
    m_status->runno = info.runno;
    m_status->subno = info.subno;
    m_status->stime = info.stime;
    m_status->ctime = ctime;
    unsigned int dcount[2];
    float dnbyte[2];
    for (int i = 0; i < 2; i++) {
      if (info.io[i].port == 0) {
        m_status->io[i].port = 0;//not used
      } else if (info.io[i].port < 0) {
        m_status->io[i].port = -1;//offline
      } else {
        if (checkConnection("", info.io[i].port) > 0) {
          m_status->io[i].port = info.io[i].port;
        } else {
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
          LogFile::debug("expno =%d, runno =%d, subno =%d, "
                         "stime =%d, ctime =%d, freq =%f, "
                         "evtsize =%f, rate =%f, count = %d, nbyte = %d",
                         m_status->expno,
                         m_status->runno, m_status->subno,
                         m_status->stime, m_status->ctime,
                         m_status->io[i].freq, m_status->io[i].evtsize,
                         m_status->io[i].rate, m_status->io[i].count,
                         m_status->io[i].nbyte);
        }
      } else {
        m_status->io[i].freq = 0;
        m_status->io[i].evtsize = 0;
        m_status->io[i].rate = 0;
        m_status->io[i].count = 0;
        m_status->io[i].nbyte = 0;
      }
    }

    if (m_status->io[0].port < 0 || m_status->io[1].port < 0) {
      m_status->state = 2;//not ready
    } else if (dcount[0] > 0) {
      m_status->state = 5;//running
    } else if (info.stime > 0) {
      m_status->state = 4;//ready
    } else {
      m_status->state = 1;//unknown
    }
    LogFile::debug("state =%d", m_status->state);
  }
}
