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
  /// A helper class for communicating over ZMQ. Includes a multicast and (if needed) also a data socket.
  class ZMQClient {
  public:
    /// Initialize the multicast and a data socket of the given type.
    template <int AZMQType>
    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress, const std::string& socketName, bool bind);

    /// Initialize only the multicast.
    void initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress);

    /// Terminate the sockets properly.
    void terminate(bool sendGoodbye = true);

    /// Reset the sockets. ATTENTION: this does not close the sockets! Use only after forks to not clean up to times.
    void reset();

    /// Subscribe to the given multicast message type.
    void subscribe(c_MessageTypes messageType);

    /// Send a message over the data socket.
    template <class AZMQMessage>
    void send(AZMQMessage message) const
    {
      AZMQMessage::element_type::toSocket(std::move(message), m_socket);
    }

    /// Send a zmq message over the data socket. ATTENTION: we are taking ownership here!
    void send(zmq::message_t& message) const;

    /// Publish the message to the multicast.
    template <class AZMQMessage>
    void publish(AZMQMessage message) const
    {
      AZMQMessage::element_type::toSocket(std::move(message), m_pubSocket);
    }

    /// Check if the client was initialized and not terminated.
    bool isOnline() const
    {
      return m_context.get();
    }


    /**
     * Poll both the multicast and the data socket until, either:
     * * the timeout is raised. The method will return with 0.
     * * a message came to the multicast socket. Then, the multicastAnswer will be called with the socket as an argument.
     *   If this method returns true, the polling will go on like before. If it returns false, the polling will be stopped. The return value of this function is 1.
     * * a message came to the data socket. See above.
     *
     * A note on the order: if there are messages on both the data socket and the multicast socket,
     * first all messages to the multicast socket will be answered and then all messages to the data socket.
     */
    template <class AMulticastAnswer, class ASocketAnswer>
    int poll(unsigned int timeout, AMulticastAnswer multicastAnswer, ASocketAnswer socketAnswer) const;

    /**
     * Poll method to only the data socket. Basically the same as the poll function to both sockets, except:
     * If the answer function returns false, the polling will stop as above, BUT: it will first answer all messages
     * that are already in the socket!
     */
    template <class ASocketAnswer>
    int pollSocket(unsigned int timeout, ASocketAnswer socketAnswer) const;

    /**
     * Poll method to only the multicast socket. Basically the same as the poll function to both sockets, except:
     * If the answer function returns false, the polling will stop as above, BUT: it will first answer all messages
     * that are already in the socket!
     */
    template <class AMulticastAnswer>
    int pollMulticast(unsigned int timeout, AMulticastAnswer multicastAnswer) const;

  private:
    /// Internal poll function.
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
