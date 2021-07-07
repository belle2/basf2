/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/utils/ZMQLogger.h>
#include <framework/pcore/zmq/utils/ZMQParent.h>

#include <zmq.hpp>

#include <memory>

namespace Belle2 {
  /**
   * Base class for every connection with virtual functions to be implemented:
   * * isReady(): return True if this connection can send events
   * * getSockets(): return the list of sockets to be used while polling for incoming messages (can be empty)
   *
   * Additionally, it has static functions for polling on a list of connections
   * or checking if a specific connection has messages.
   */
  class ZMQConnection : public ZMQLogger {
  public:
    /// Typedef of a function which will be called if a connection has a message
    using ReactorFunction = std::function<void(void)>;

    /**
     * Poll on the given connections and call the attached function if a messages comes in.
     * If after timeout milliseconds still no message is received, return anyways.
     * If timeout is 0, do not wait. If timeout is -1, wait infinitely.
     *
     * Returns true if a message was received on any socket, false otherwise.
     * Attention: in case of an interrupted system call (e.g. because a signal was received)
     * the function might return anyways with a negative result even before the timeout!
     */
    static bool poll(const std::map<const ZMQConnection*, ReactorFunction>& connectionList, int timeout);

    /// Check if the given connection as an incoming message (right now, no waiting).
    static bool hasMessage(const ZMQConnection* connection);

    /// Virtual default destructor
    virtual ~ZMQConnection() = default;

    /// Return true of this connection is able to send messages right now. Can be overloaded in derived classes.
    virtual bool isReady() const;
    /// Return the list of sockets which should be polled on to check for incoming messages. Can be empty. Must be overloaded in derived classes.
    virtual std::vector<zmq::socket_t*> getSockets() const = 0;
  };

  /**
   * Specialized connection over a ZMQ socket. Keeps track of a shared instance of a ZMQParent and the zmq socket.
   * Will not initialize the socket, you need to do this in a derived class.
   */
  class ZMQConnectionOverSocket : public ZMQConnection {
  public:
    /// Create a new instance passing the shared ZMQParent
    explicit ZMQConnectionOverSocket(const std::shared_ptr<ZMQParent>& parent);

    /// The socket used for polling is just the stored socket.
    std::vector<zmq::socket_t*> getSockets() const final;

    /// Return the connection string for this socket
    std::string getEndPoint() const;
  protected:
    /// The shared ZMQParent instance
    std::shared_ptr<ZMQParent> m_parent;
    /// The memory of the socket. Needs to be initialized in a derived class.
    std::unique_ptr<zmq::socket_t> m_socket;
  };
}
