/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
