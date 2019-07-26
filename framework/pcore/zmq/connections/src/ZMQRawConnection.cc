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

  /// Get next event and confirm
  zmq::message_t identity;
  m_socket->recv(&identity);
  std::string identityString(identity.data<char>(), identity.size());
  B2ASSERT("The message is incomplete!", m_socket->getsockopt<int>(ZMQ_RCVMORE) == 1);
  B2ASSERT("The app can only handle a single connection!",
           m_inputIdentity == identityString or m_inputIdentity.empty());

  const size_t remainingSpace = m_maximalBufferSize - m_writeAddress;
  const size_t receivedBytes = m_socket->recv(&m_buffer[m_writeAddress], remainingSpace);
  B2ASSERT("The message is longer than expected!", m_socket->getsockopt<int>(ZMQ_RCVMORE) == 0);
  if (receivedBytes == 0) {
    // Server connects or disconnects
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
  if (receivedBytes > remainingSpace) {
    B2FATAL("The size of the buffer is too small! " << receivedBytes << " > " << remainingSpace);
  }
  average("average_received_byte_packages", receivedBytes);
  m_writeAddress += receivedBytes;

  log("write_address", static_cast<long>(m_writeAddress));

  // If the current buffer is smaller than an int, we can not get the size
  while (m_writeAddress >= sizeof(int)) {
    if (m_currentSize == 0) {
      memcpy(&m_currentSize, &m_buffer[0], sizeof(int));
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
      average("data_size", m_currentSize);
      increment("received_events");
      timeit("event_rate");

      unsigned int startAddress = 0;
      if (m_receiveEventMessages) {
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
  log("data_size", 0.0);
  log("sent_events", 0l);
  log("event_rate", 0.0);

  log("socket_state", "disconnected");
  log("socket_connects", 0l);
  log("socket_disconnects", 0l);

  m_socket = m_parent->createSocket<ZMQ_STREAM>(outputAddress);
}

void ZMQRawOutput::handleEvent(zmq::message_t message)
{
  B2ASSERT("Data Socket needs to be connected", not m_dataIdentity.empty());

  const auto dataSize = message.size();

  average("data_size", dataSize);
  increment("sent_events");
  timeit("event_rate");

  m_socket->send(ZMQMessageHelper::createZMQMessage(m_dataIdentity), ZMQ_SNDMORE);
  if (not m_addEventSize) {
    m_socket->send(std::move(message));
  } else {
    zmq::message_t tmpMessage(message.size() + sizeof(int));
    const int messageSize = htonl(message.size());
    memcpy(tmpMessage.data<char>(), &messageSize, sizeof(int));
    memcpy(tmpMessage.data<char>() + sizeof(int), message.data(), message.size());
    m_socket->send(std::move(tmpMessage));
  }
}

void ZMQRawOutput::handleIncomingData()
{
  zmq::message_t identity;
  m_socket->recv(&identity);
  zmq::message_t nullMessage;
  m_socket->recv(&nullMessage);
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
  return not m_dataIdentity.empty();
}
