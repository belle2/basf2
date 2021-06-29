/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQRawConnection.h>
#include <framework/pcore/zmq/messages/ZMQMessageHelper.h>
#include <framework/logging/Logger.h>

#include <arpa/inet.h>

using namespace Belle2;

ZMQRawInput::ZMQRawInput(const std::string& inputAddress, unsigned int maximalBufferSize, bool receiveEventMessages,
                         const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(parent),
  m_maximalBufferSize(maximalBufferSize),
  m_receiveEventMessages(receiveEventMessages)
{
  // We clear all our internal state and counters
  log("data_size", 0.0);
  log("received_events", 0l);
  log("event_rate", 0.0);
  log("average_received_byte_packages", 0.0);

  log("socket_state", "disconnected");
  log("socket_connects", 0l);
  log("socket_disconnects", 0l);
  log("current_size", 0l);
  log("write_address", 0l);
  log("average_number_of_events_per_package", 0l);

  // STREAM is the ZMQ type for raw, non ZMQ connections
  m_socket = m_parent->createSocket<ZMQ_STREAM>(inputAddress);

  m_buffer.reserve(maximalBufferSize);
}

void ZMQRawInput::clear()
{
  m_writeAddress = 0;
  m_currentSize = 0;
}

std::vector<zmq::message_t> ZMQRawInput::handleIncomingData()
{
  std::vector<zmq::message_t> receivedMessages;

  // We will always get one or two parts. The first one is the identity..
  zmq::message_t identity;
  auto received = m_socket->recv(identity, zmq::recv_flags::none);
  B2ASSERT("No message received", received);
  B2ASSERT("Message should not be empty", *received > 0);
  std::string identityString(identity.data<char>(), identity.size());
  B2ASSERT("The message is incomplete!", m_socket->get(zmq::sockopt::rcvmore) == 1);
  B2ASSERT("The app can only handle a single connection!",
           m_inputIdentity == identityString or m_inputIdentity.empty());

  // ... and the second one is the message itself.
  const size_t remainingSpace = m_maximalBufferSize - m_writeAddress;
  auto receivedBytes = m_socket->recv(zmq::mutable_buffer{&m_buffer[m_writeAddress], remainingSpace}, zmq::recv_flags::none);
  B2ASSERT("No message received", receivedBytes);
  B2ASSERT("The message is longer than expected! Increase the buffer size.", !receivedBytes->truncated());
  if (receivedBytes->size == 0) {
    // Empty message means the client connected or disconnected
    if (m_inputIdentity.empty()) {
      m_inputIdentity = identityString;
      log("socket_state", "connected");
      increment("socket_connects");
    } else {
      m_inputIdentity = "";
      log("socket_state", "disconnected");
      increment("socket_disconnects");
    }
    return receivedMessages;
  }
  // We can maximal write `remainingSpace` into the buffer. If the message was longer, ZMQ will just cut it.
  // This means we are loosing data and the buffer size should be increased.
  if (receivedBytes->untruncated_size > remainingSpace) {
    B2FATAL("The size of the buffer is too small! " << receivedBytes->untruncated_size << " > " << remainingSpace);
  }
  average("average_received_byte_packages", receivedBytes->size);

  // `m_writeAddress` always points to the index on where we will write next.
  // As we have written `receivedBytes` we need to advance
  m_writeAddress += receivedBytes->size;

  log("write_address", static_cast<long>(m_writeAddress));

  // If the current buffer is smaller than an int, we can not get the size
  while (m_writeAddress >= sizeof(int)) {
    if (m_currentSize == 0) {
      // we do not know the size of the data package already, so lets get it from the buffer.
      // It is always in the first sizeof(int) of the data
      memcpy(&m_currentSize, &m_buffer[0], sizeof(int));

      // Here the two different message formats differ
      if (m_receiveEventMessages) {
        m_currentSize = ntohl(m_currentSize);
      }
      B2ASSERT("Strange size in the data!", m_currentSize > 0);
      if (m_receiveEventMessages) {
        m_currentSize += sizeof(int);
      } else {
        m_currentSize *= sizeof(int);
      }

      log("current_size", static_cast<long>(m_currentSize));
    }
    if (m_writeAddress >= m_currentSize) {
      // Now we know the size already, and we have enough data received so we have actually the full
      // data. We can build the total message.
      average("data_size", m_currentSize);
      increment("received_events");
      timeit("event_rate");

      // Again, here the two different message formats differ. One includes the first int with the length...
      unsigned int startAddress = 0;
      if (m_receiveEventMessages) {
        // .. and the other does not
        startAddress = sizeof(int);
      }
      zmq::message_t dataMessage(&m_buffer[startAddress], static_cast<size_t>(m_currentSize - startAddress));

      memmove(&m_buffer[0], &m_buffer[m_currentSize], m_writeAddress - m_currentSize);
      m_writeAddress -= m_currentSize;
      m_currentSize = 0;

      log("write_address", static_cast<long>(m_writeAddress));
      log("current_size", static_cast<long>(m_currentSize));

      receivedMessages.push_back(std::move(dataMessage));
    } else {
      // We did not receive a full message up to now
      break;
    }
  }

  average("average_number_of_events_per_package", receivedMessages.size());
  return receivedMessages;
}

ZMQRawOutput::ZMQRawOutput(const std::string& outputAddress, bool addEventSize,
                           const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(
                               parent), m_addEventSize(addEventSize)
{
  // We clear all our internal state and counters
  log("data_size", 0.0);
  log("sent_events", 0l);
  log("event_rate", 0.0);

  log("socket_state", "disconnected");
  log("socket_connects", 0l);
  log("socket_disconnects", 0l);

  // STREAM is the ZMQ type for raw, non ZMQ connections
  m_socket = m_parent->createSocket<ZMQ_STREAM>(outputAddress);
}

void ZMQRawOutput::handleEvent(zmq::message_t message)
{
  // Send the message. If requested, add the message size in front of the message
  B2ASSERT("Data Socket needs to be connected", not m_dataIdentity.empty());

  const auto dataSize = message.size();

  average("data_size", dataSize);
  increment("sent_events");
  timeit("event_rate");

  m_socket->send(ZMQMessageHelper::createZMQMessage(m_dataIdentity), zmq::send_flags::sndmore);
  if (not m_addEventSize) {
    m_socket->send(std::move(message), zmq::send_flags::none);
  } else {
    zmq::message_t tmpMessage(message.size() + sizeof(int));
    const int messageSize = htonl(message.size());
    memcpy(tmpMessage.data<char>(), &messageSize, sizeof(int));
    memcpy(tmpMessage.data<char>() + sizeof(int), message.data(), message.size());
    m_socket->send(std::move(tmpMessage), zmq::send_flags::none);
  }
}

void ZMQRawOutput::handleIncomingData()
{
  // The only possibility that we can receive a message is when the client connects or disconnects
  zmq::message_t identity;
  auto received = m_socket->recv(identity, zmq::recv_flags::none);
  B2ASSERT("No message received", received);
  zmq::message_t nullMessage;
  received = m_socket->recv(nullMessage, zmq::recv_flags::none);
  B2ASSERT("No message received", received);
  std::string identityString(identity.data<char>(), identity.size());

  if (m_dataIdentity.empty()) {
    m_dataIdentity = identityString;
    log("socket_state", "connected");
    increment("socket_connects");
  } else {
    B2ASSERT("The app can only handle a single connection!", m_dataIdentity == identityString);
    m_dataIdentity = "";
    log("socket_state", "disconnected");
    increment("socket_disconnects");
  }
}

bool ZMQRawOutput::isReady() const
{
  // Only ready of the client connected
  return not m_dataIdentity.empty();
}
