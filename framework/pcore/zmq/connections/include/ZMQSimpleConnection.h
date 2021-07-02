/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/connections/ZMQConnection.h>
#include <framework/pcore/zmq/utils/ZMQParent.h>

#include <framework/pcore/zmq/messages/ZMQIdMessage.h>

#include <memory>
#include <string>

namespace Belle2 {
  /**
   * Connection type to be used for answering simple requests, e.g. for monitoring.
   * Does no load-balancing or confirmation in anykind but just
   * sends a given message (which includes the id of the receiver) over the
   * initialized socket of ZMQ_ROUTER (bind) type.
   * On received messages, returns the message as ZMQIdMessage (including the id of the
   * sender).
   *
   * Please note, that the message content (e.g. monitoring) needs to be collected
   * externally from this class.
   */
  class ZMQSimpleConnection : public ZMQConnectionOverSocket {
  public:
    /// Create a new simple connection with a ZMQ_ROUTER socket binding to the given address.
    ZMQSimpleConnection(const std::string& monitoringAddress, const std::shared_ptr<ZMQParent>& parent);

    /// Block until there is an incoming message and return it as a ZMQIdMessage
    std::unique_ptr<ZMQIdMessage> handleIncomingData();
    /// Send the given message via the socket (the message includes the identity to send to)
    void handleEvent(std::unique_ptr<ZMQIdMessage> message);
  };
}