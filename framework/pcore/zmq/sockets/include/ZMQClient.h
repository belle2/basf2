/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>
#include <zmq.hpp>
#include <memory>

namespace Belle2 {

  template <int AZMQType>
  class ZMQClient {
  public:
    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress, const std::string& socketName, bool bind);
    void terminate();

    template <class AZMQMessage>
    void send(const AZMQMessage& message)
    {
      message->toSocket(m_socket);
    }

  private:
    /// ZMQ context
    std::unique_ptr<zmq::context_t> m_context;

    /// ZMQ Pub socket
    std::unique_ptr<zmq::socket_t> m_pubSocket;
    /// ZMQ sub socket
    std::unique_ptr<zmq::socket_t> m_subSocket;
    /// ZMQ socket
    std::unique_ptr<zmq::socket_t> m_socket;
  };
} // namespace Belle2
