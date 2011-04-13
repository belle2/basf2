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

#include <cstdlib>

#include <framework/logging/Logger.h>

#include <daq/hlt/HLTBuffer.h>
#include <daq/hlt/B2Socket.h>
#include <daq/hlt/B2SocketException.h>

namespace Belle2 {

  /// @class EvtReceiver
  /// @brief Event receiver class based on B2Socket
  class EvtReceiver : private B2Socket {
  public:
    EvtReceiver(int port);
    EvtReceiver() {};
    virtual ~EvtReceiver();

    EStatus init(void);
    EStatus init(HLTBuffer* buffer);
    EStatus listen(void);

    const EvtReceiver& operator << (const std::string&) const;
    const EvtReceiver& operator >> (std::string&) const;

    EStatus accept(EvtReceiver&);

  private:
    int m_port;
    HLTBuffer* m_buffer;
  };
}

#endif
