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

#include <zmq.hpp>

#include <string>
#include <memory>

namespace Belle2 {
  /**
   * Input connection allowing to speak with non-zmq sockets via a ZMQ_STREAM
   * socket. Can only speak with a single socket.
   *
   * On receiving a TC packet, it is stored in an internal buffer. Only if the
   * message is complete it is returned wrapped up as a zmq:message_t.
   *
   * Two message formats are understood:
   * * if receiveEventMessages is set to false, the first int of the message must be
   *   the size L of the message, in units of sizeof(int) = words.
   *   (so full message size in bytes = L * sizeof(int))
   *   Please note that this size also includes this first int, so the real data message
   *   has a size in bytes of L * sizeof(int) - sizeof(int)
   * * if it is true, the first int of the message must be the ntohl-converted
   *   size of the message in bytes *without this first size int*.
   *
   * The two message formats are chosen to be compatible with the HLT implementation.
   */
  class ZMQRawInput : public ZMQConnectionOverSocket {
  public:
    /// Create a new raw input connection. The bind or connect behavior is chosen according to the given address.
    ZMQRawInput(const std::string& inputAddress, unsigned int maximalBufferSize, bool receiveEventMessages,
                const std::shared_ptr<ZMQParent>& parent);

    /**
     * Block until a TCP packet can be received from the socket.
     * Returns only full messages as zmq:message_t. Please note that this can be
     * * 0 messages if there has not been a full message so far
     * * 1 message if there was
     * * more than 1 if the message size is smaller than the TCP package size (~8kB)
     */
    std::vector<zmq::message_t> handleIncomingData();

    /// Reset the internal buffer and counter
    void clear();

  private:
    /// Parameter for the maximal buffer size. If this size is reached, a FATAL will be issued.
    unsigned int m_maximalBufferSize;
    /// Internal storage for the buffer
    std::vector<char> m_buffer;
    /// Where in the buffer are we currently writing to
    size_t m_writeAddress = 0;
    /// How large should the full message be? The information is from the first int of the message
    unsigned int m_currentSize = 0;
    /// Parameter to receive event messages (see above)
    bool m_receiveEventMessages;
    /// Internal storage of the connected socket to check if we get messages from multiple ones.
    std::string m_inputIdentity = "";
  };

  /**
   * Output connection to speak to non-zmq sockets via a ZMQ_STREAM socket.
   * If a socket is connected, sends out the given message over this socket as it is.
   * If addEventSize is true, will prefix the message with the htonl-converted
   * size of the message in bytes. Compare this to the receiveEventMessages
   * setting of the ZMQRawInput connection.
   * If no socket is connected, this connection is not ready.
   *
   * Please note that although it is an output connection, there will still be
   * "incoming messages", namely whenever the socket connects or disconnects.
   * So you need to poll on this connection.
   * Can only send messages to a single connected socket.
   */
  class ZMQRawOutput : public ZMQConnectionOverSocket {
  public:
    /// Create a new raw output connection. The bind or connect behavior is chosen according to the given address.
    ZMQRawOutput(const std::string& outputAddress, bool addEventSize, const std::shared_ptr<ZMQParent>& parent);

    /// Pass on the message - maybe by prefixing it with a htonl-converted data size in bytes.
    virtual void handleEvent(zmq::message_t message);
    /// Handle incoming data: a socket (dis)connect
    void handleIncomingData();
    /// If no socket is connected, this connection is not ready.
    bool isReady() const final;

  private:
    /// Internal storage of the connected identity to no have multiple connections.
    std::string m_dataIdentity = "";
    /// Parameter to add the event size to a message
    bool m_addEventSize = false;
  };
}