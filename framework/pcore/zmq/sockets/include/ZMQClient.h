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
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  class ZMQClient {
  public:
    template <int AZMQType>
    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress, const std::string& socketName, bool bind);
    void terminate(bool sendGoodbye = true);
    void reset();
    void subscribe(c_MessageTypes messageType);

    template <class AZMQMessage>
    void send(const AZMQMessage& message) const
    {
      message->toSocket(m_socket);
    }

    void send(zmq::message_t& message) const;

    template <class AZMQMessage>
    void publish(const AZMQMessage& message) const
    {
      message->toSocket(m_pubSocket);
    }

    bool isOnline() const
    {
      return m_context.get();
    }

    template <class AMulticastAnswer, class ASocketAnswer>
    int poll(unsigned int timeout, AMulticastAnswer multicastAnswer, ASocketAnswer socketAnswer) const;

    template <class ASocketAnswer>
    int pollSocket(unsigned int timeout, ASocketAnswer socketAnswer) const;

    template <class AMulticastAnswer>
    int pollMulticast(unsigned int timeout, AMulticastAnswer multicastAnswer) const;

  private:
    static int pollSocketVector(const std::vector<zmq::socket_t*>& socketList, int timeout);

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

    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress);
  };

  template <class AMulticastAnswer, class ASocketAnswer>
  int ZMQClient::poll(unsigned int timeout, AMulticastAnswer multicastAnswer, ASocketAnswer socketAnswer) const
  {
    B2ASSERT("Can only run this on started clients", m_subSocket and m_socket);
    bool repeat = true;
    int pollResult;
    do {
      pollResult = pollSocketVector(m_pollSocketPtrList, timeout);
      if (pollResult & 1) {
        // The multicast is the first entry.
        // Get all entries if possible, but do not block anymore.
        std::vector<zmq::socket_t*> vector = {m_subSocket.get()};
        while (pollSocketVector(vector, 0) and repeat) {
          repeat = multicastAnswer(m_subSocket);
        }
      }

      if (pollResult & 2) {
        // Get all entries if possible, but do not block anymore.
        std::vector<zmq::socket_t*> vector = {m_socket.get()};
        while (pollSocketVector(vector, 0) and repeat) {
          repeat = socketAnswer(m_socket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }

  // TODO: Attention: repeat meaning changed
  template <class ASocketAnswer>
  int ZMQClient::pollSocket(unsigned int timeout, ASocketAnswer socketAnswer) const
  {
    B2ASSERT("Can only run this on started clients", m_socket);
    std::vector<zmq::socket_t*> vector = {m_socket.get()};

    bool repeat = true;
    int pollResult;
    do {
      pollResult = pollSocketVector(vector, timeout);
      if (pollResult) {
        while (pollSocketVector(vector, 0)) {
          repeat = socketAnswer(m_socket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }

  template <class AMulticastAnswer>
  int ZMQClient::pollMulticast(unsigned int timeout, AMulticastAnswer multicastAnswer) const
  {
    B2ASSERT("Can only run this on started clients", m_subSocket);
    std::vector<zmq::socket_t*> vector = {m_subSocket.get()};

    bool repeat = true;
    int pollResult;
    do {
      pollResult = pollSocketVector(vector, timeout);
      if (pollResult) {
        while (pollSocketVector(vector, 0)) {
          repeat = multicastAnswer(m_subSocket);
        }
      }
    } while (repeat and pollResult);

    return pollResult;
  }
} // namespace Belle2
