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

#include <framework/logging/Logger.h>

#include <zmq.hpp>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

namespace Belle2 {
  /**
   * A helper class for creating ZMQ sockets keeping track of the ZMQ context
   * and terminating it if needed. Basically just a convenience wrapper
   * around a zmq::context_t.
   *
   * In your application, you should have only a single instance of
   * a ZMQParent, for example a std::shared_ptr in your main class.
   * Use it to create sockets and let the automatic destruction at the end
   * call the cleanup functions properly. After the parent is closed, all
   * its sockets are also invalid. The parent gets initialized
   * on first socket creation.
   *
   * Only in very rare cases you need to not terminate this parent,
   * e.g. when you create a parent and then fork (because you do not want to
   * close the parent twice). This however is an expert function.
   */
  class ZMQParent {
  public:
    /// Destroy the parent by terminating the ZMQ context
    ~ZMQParent();

    /// Send a given message over the given created socket. You need to move in the message for zero-copy.
    template<class AZMQMessage>
    static void send(std::unique_ptr<zmq::socket_t>& socket, AZMQMessage message);

    /**
     * Poll function.
     *
     * Poll on the given socket pointers and return a bitmask indicating which
     * sockets received a message. If the poll is interrupted (e.g. by a system interrupt),
     * return 0.
     *
     * If the timeout is positive, wait maximal timeout milliseconds before returning
     * (even if there is no message in the sockets).
     * If the timeout is 0, only check once and return immediately.
     * If the timeout is -1, wait infinitely.
     * */
    static unsigned int poll(const std::vector<zmq::socket_t*>& socketList, int timeout);

    /// Create a unique ZMQ identity in the form <hostname>_<pid> (if pid is 0, use the current processes PID).
    static std::string createIdentity(unsigned int pid = 0);

    /// Terminate the parent manually (before calling its destructor). You probably do not need to do this.
    void terminate();

    /**
     * Create a socket of the given type with the given address and bind or not bind it.
     * Typical socket types are ZMQ_ROUTER, ZMQ_DEALER and ZMQ_STREAM.
     *
     * The address needs to be in the form "tcp://ast:<port>" (with ast = *) if you want to bind to a local
     * interface or "tcp://<host>:<port>" if you want to connect. Use the convenience
     * function below if you want the program to decide based on your address and use
     * this function if you want the program to force a specific bind behaviour
     * (it will raise an exception if given an invalid address).
     * */
    template<int AZMQType>
    std::unique_ptr<zmq::socket_t> createSocket(const std::string& socketAddress, bool bind);

    /**
     * Create a socket of the given type while deciding on the bind behaviour via the address.
     *
     * An address in the form "tcp://ast:<port>" (with ast = *) means the socket is bound to a local interface.
     * An address in the from "tcp://<host>:<port>" means it connects to the given host.
     *
     * If you want to fore a given bind behaviour (and fail if the address is invalid),
     * use the function above.
     * */
    template<int AZMQType>
    std::unique_ptr<zmq::socket_t> createSocket(const std::string& socketAddress);

    /// Expert function: Reset the parent without context closing. ATTENTION: which will not clean up properly! Do only use after forks.
    void reset();

  private:
    /// ZMQ context
    std::unique_ptr<zmq::context_t> m_context;

    /// Initialize the parent by creating the context
    void initialize();
  };

  template<int AZMQType>
  std::unique_ptr<zmq::socket_t> ZMQParent::createSocket(const std::string& socketAddress, bool bind)
  {
    initialize();

    try {
      // TODO: options to test additionally in the future:
      // * ZMQ_IMMEDIATE
      // * ZMQ_PROBE_ROUTER
      // * ZMQ_ROUTER_MANDATORY
      // * ZMQ_ROUTER_NOTIFY
      auto socket = std::make_unique<zmq::socket_t>(*m_context, AZMQType);

      if (AZMQType == ZMQ_DEALER) {
        auto identity = createIdentity();
        socket->set(zmq::sockopt::routing_id, identity);
      }

      // Linger means: close the socket immediately if requested, do not wait until all messages have been sent.
      // This is needed because if we want to ABORT, we usually know what we are doing (and want to do it now).
      socket->set(zmq::sockopt::linger, 0);
      if (bind) {
        socket->bind(socketAddress.c_str());
      } else {
        socket->connect(socketAddress.c_str());
      }

      // Wait a bit to give the framework time to initialize the socket
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      return socket;
    } catch (zmq::error_t& error) {
      B2FATAL("Creating the ZMQ socket for address " << socketAddress << " failed because of: " << error.what());
    }
  }

  template<int AZMQType>
  std::unique_ptr<zmq::socket_t> ZMQParent::createSocket(const std::string& socketAddress)
  {
    // We only check for "*" in the full address for now. This is fine, as neither a valid hostname nor a valid
    // port number can ever contain a "*".
    if (socketAddress.find("*") != std::string::npos) {
      return createSocket<AZMQType>(socketAddress, true);
    } else {
      return createSocket<AZMQType>(socketAddress, false);
    }
  }

  template<class AZMQMessage>
  void ZMQParent::send(std::unique_ptr<zmq::socket_t>& socket, AZMQMessage message)
  {
    AZMQMessage::element_type::toSocket(std::move(message), socket);
  }

} // namespace Belle2
