/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_IOInfo_h
#define _Belle2_IOInfo_h

#include <vector>

namespace Belle2 {

  class IOInfo {
  public:
    static int checkTCP(std::vector<IOInfo>& info);
    static int checkTCP(IOInfo& info);

  public:
    IOInfo() {}
    ~IOInfo() {}

  public:
    const char* getLocalIP() const;
    unsigned int getLocalAddress() const { return m_local_addr; }
    int getLocalPort() const { return m_local_port; }
    const char* getRemoteIP() const;
    unsigned int getRemoteAddress() const { return m_remote_addr; }
    int getRemotePort() const { return m_remote_port; }
    int getState() const { return m_state; }
    int getTXQueue() const { return m_tx_queue; }
    int getRXQueue() const { return m_rx_queue; }
    void setLocalAddress(unsigned int addr) { m_local_addr = addr; }
    void setLocalPort(int port) { m_local_port = port; }
    void setRemoteAddress(unsigned int addr) { m_remote_addr = addr; }
    void setRemotePort(int port) { m_remote_port = port; }
    void setState(int state) { m_state = state; }
    void setTXQueue(int queue) { m_tx_queue = queue; }
    void setRXQueue(int queue) { m_rx_queue = queue; }

  private:
    unsigned int m_local_addr;
    int m_local_port;
    unsigned int m_remote_addr;
    int m_remote_port;
    int m_state;
    int m_tx_queue;
    int m_rx_queue;

  };

}

#endif
