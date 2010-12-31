/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTSENDER_H
#define EVTSENDER_H

#include <daq/hlt/RingBuffer.h>
#include <daq/hlt/B2Socket.h>

namespace Belle2 {

  /// @class EvtSender
  /// @brief Event sender class based on B2Socket
  class EvtSender : private B2Socket {
  public:
    EvtSender(std::string host, int port);
    EvtSender() {};
    virtual ~EvtSender();

    void init();
    void init(RingBuffer* buffer);
    void setDestination(std::string dest);
    int connect();
    int broadCasting();

    const EvtSender& operator << (const std::string&) const;
    const EvtSender& operator << (const NodeInfo&) const;
    const EvtSender& operator >> (std::string&) const;
    const EvtSender& operator >> (NodeInfo&) const;

  private:
    int m_port;
    std::string m_host;
    RingBuffer* m_buffer;
  };
}

#endif
