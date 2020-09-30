/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/connections/ZMQConnection.h>

#include <memory>
#include <zmq.hpp>

namespace Belle2 {
  /// Connection doing just nothing: without the capability to pull, send or receive. Useful for template interfaces.
  class ZMQNullConnection : public ZMQConnection {
  public:
    /// There is no socket, so just return an empty list
    std::vector<zmq::socket_t*> getSockets() const final;

    /// Raise an exception if called - there should never be any incoming message
    void handleIncomingData();
    /// Return the connection string
    std::string getEndPoint() const { return ""; }
  };
}
