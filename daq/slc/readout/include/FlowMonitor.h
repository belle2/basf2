/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FlowMonitor_h
#define _Belle2_FlowMonitor_h

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/slc/readout/IOInfo.h>
#include <daq/slc/readout/ronode_status.h>

namespace Belle2 {

  class FlowMonitor {

  public:
    FlowMonitor()
    {
      m_ioinfo.push_back(IOInfo());
      m_ioinfo.push_back(IOInfo());
      m_info = NULL;
      m_status.connection_in = 0;
      m_status.connection_out = 0;
    }
    ~FlowMonitor() {}

  public:
    bool open(RunInfoBuffer* m_info);
    bool close();
    ronode_status& monitor();
    const ronode_status& getStatus() const { return m_status; }
    ronode_status& getStatus() { return m_status; }
    bool isAvailable() const { return m_info != NULL; }

  private:
    std::string m_nodename;
    RunInfoBuffer* m_info;
    std::vector<IOInfo> m_ioinfo;
    std::string m_ip[2];
    unsigned long long m_nbyte[2];
    ronode_status m_status;

  };

}

#endif
