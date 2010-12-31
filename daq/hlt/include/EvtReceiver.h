/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVTRECEIVER_H
#define EVTRECEIVER_H

#include <daq/hlt/RingBuffer.h>
#include <daq/hlt/B2Socket.h>

namespace Belle2 {

  /// @class EvtReceiver
  /// @brief Event receiver class based on B2Socket
  class EvtReceiver : private B2Socket {
  public:
    EvtReceiver(int port);
    EvtReceiver() {};
    virtual ~EvtReceiver();

    void init(void);
    void init(RingBuffer* buffer);
    void listen(void);

    const EvtReceiver& operator << (const std::string&) const;
    const EvtReceiver& operator << (const NodeInfo&) const;
    const EvtReceiver& operator >> (std::string&) const;
    const EvtReceiver& operator >> (NodeInfo&) const;

    void accept(EvtReceiver&);

  private:
    int m_port;
    RingBuffer* m_buffer;
  };
}

#endif
