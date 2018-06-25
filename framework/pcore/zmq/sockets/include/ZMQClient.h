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
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  template <int AZMQType>
  class ZMQClient {
  public:
    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress, const std::string& socketName, bool bind);
    void terminate();
    void subscribe(c_MessageTypes messageType);

    template <class AZMQMessage>
    void send(const AZMQMessage& message) const
    {
      message->toSocket(m_socket);
    }

    template <class AZMQMessage>
    void publish(const AZMQMessage& message) const
    {
      message->toSocket(m_pubSocket);
    }

    template <class AMulticastAnswer, class ASocketAnswer>
    int poll(unsigned int timeout, AMulticastAnswer multicastAnswer, ASocketAnswer socketAnswer) const;

    template <class ASocketAnswer>
    int pollSocket(unsigned int timeout, ASocketAnswer socketAnswer) const;

    template <class AMulticastAnswer>
    int pollMulticast(unsigned int timeout, AMulticastAnswer multicastAnswer) const;

  private:
    /// ZMQ context
    std::unique_ptr<zmq::context_t> m_context;

    /// Will use this vector for polling
    std::vector<zmq::socket_t*> m_pollSocketPtrList;

    /// ZMQ Pub socket
    std::unique_ptr<zmq::socket_t> m_pubSocket;
    /// ZMQ sub socket
    std::unique_ptr<zmq::socket_t> m_subSocket;
    /// ZMQ socket
    std::unique_ptr<zmq::socket_t> m_socket;
  };

  template <int AZMQType>
  template <class AMulticastAnswer, class ASocketAnswer>
  int ZMQClient<AZMQType>::poll(unsigned int timeout, AMulticastAnswer multicastAnswer, ASocketAnswer socketAnswer) const
  {
    bool repeat = true;
    int pollResult;
    do {
      pollResult = ZMQHelper::pollSockets(m_pollSocketPtrList, timeout);
      if (pollResult & 1) {
        // The multicast is the first entry.
        // Get all entries if possible, but do not block anymore.
        while (ZMQHelper::pollSocket(m_subSocket, 0) and repeat) {
          repeat = multicastAnswer(m_subSocket);
        }
      }

      if (pollResult & 2) {
        // Get all entries if possible, but do not block anymore.
        while (ZMQHelper::pollSocket(m_socket, 0) and repeat) {
          repeat = socketAnswer(m_socket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }

  template <int AZMQType>
  template <class ASocketAnswer>
  int ZMQClient<AZMQType>::pollSocket(unsigned int timeout, ASocketAnswer socketAnswer) const
  {
    bool repeat = true;
    int pollResult;
    do {
      pollResult = ZMQHelper::pollSocket(m_socket, timeout);
      if (pollResult) {
        while (ZMQHelper::pollSocket(m_socket, 0) and repeat) {
          repeat = socketAnswer(m_socket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }

  template <int AZMQType>
  template <class AMulticastAnswer>
  int ZMQClient<AZMQType>::pollMulticast(unsigned int timeout, AMulticastAnswer multicastAnswer) const
  {
    bool repeat = true;
    int pollResult;
    do {
      pollResult = ZMQHelper::pollSocket(m_subSocket, timeout);
      if (pollResult) {
        while (ZMQHelper::pollSocket(m_subSocket, 0) and repeat) {
          repeat = multicastAnswer(m_subSocket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }
} // namespace Belle2
