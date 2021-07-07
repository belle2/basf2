/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQLoadBalancedConnection.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ZMQLoadBalancedInput::ZMQLoadBalancedInput(const std::string& inputAddress, unsigned int bufferSize,
                                           const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(parent)
{
  // We clear all our internal state and counters
  log("sent_ready", 0l);
  log("data_size", 0.0);
  log("received_events", 0l);
  log("event_rate", 0.0);

  // Create a non-binding DEALER socket
  m_socket = m_parent->createSocket<ZMQ_DEALER>(inputAddress, false);

  // Send as many ready message as our buffer size is. This means we will get that many events, which will then be "in flight"
  for (unsigned int i = 0; i < bufferSize; i++) {
    auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
    ZMQParent::send(m_socket, std::move(readyMessage));
    increment("sent_ready");
  }
}

std::unique_ptr<ZMQNoIdMessage> ZMQLoadBalancedInput::handleIncomingData()
{
  auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);

  if (message->isMessage(EMessageTypes::c_rawDataMessage) or message->isMessage(EMessageTypes::c_eventMessage)) {
    // if it is an event message, return a ready message back. If not, no need for that.
    auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
    ZMQParent::send(m_socket, std::move(readyMessage));
    increment("sent_ready");

    // and also do some logging
    const auto dataSize = message->getDataMessage().size();

    average("data_size", dataSize);
    increment("received_events");
    timeit("event_rate");
  }

  return message;
}

ZMQLoadBalancedOutput::ZMQLoadBalancedOutput(const std::string& outputAddress, bool lax,
                                             const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(
                                                 parent), m_lax(lax)
{
  // We clear all our internal state and counters
  log("ready_queue_size", static_cast<long>(m_readyWorkers.size()));
  log("registered_workers", static_cast<long>(m_allWorkers.size()));

  log("data_size", 0.0);
  log("dismissed_events", 0l);
  log("event_rate", 0.0);
  log("sent_events", 0l);

  log("all_stop_messages", 0l);
  log("sent_stop_messages",  0l);
  log("last_stop_sent",  "");

  log("all_terminate_messages", 0l);
  log("sent_terminate_messages",  0l);
  log("last_terminate_sent",  "");

  // Create a binding ROUTER socket
  m_socket = m_parent->createSocket<ZMQ_ROUTER>(outputAddress, true);
}

void ZMQLoadBalancedOutput::handleEvent(std::unique_ptr<ZMQNoIdMessage> message)
{
  if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
    // Tell all workers to stop this run
    for (auto worker : m_allWorkers) {
      auto sendMessage = ZMQMessageFactory::createMessage(worker, EMessageTypes::c_lastEventMessage);
      ZMQParent::send(m_socket, std::move(sendMessage));
    }
    m_sentStopMessages = true;
    log("all_stop_messages", static_cast<long>(m_sentStopMessages));
    increment("sent_stop_messages");
    logTime("last_stop_sent");
    return;
  } else if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    // Tell all workers to terminate
    for (auto worker : m_allWorkers) {
      auto sendMessage = ZMQMessageFactory::createMessage(worker, EMessageTypes::c_terminateMessage);
      ZMQParent::send(m_socket, std::move(sendMessage));
    }
    m_sentTerminateMessages = true;
    log("all_terminate_messages", static_cast<long>(m_sentTerminateMessages));
    increment("sent_terminate_messages");
    logTime("last_terminate_sent");
    return;
  }

  if (m_lax and m_readyWorkers.empty()) {
    // There is no one that can handle the event in the moment, dismiss it (if lax is true)
    increment("dismissed_events");
    return;
  }
  if (m_sentStopMessages) {
    B2ERROR("Received events after stop! I will dismiss this event.");
    increment("dismissed_events");
    return;
  }

  const auto dataSize = message->getDataMessage().size();

  B2ASSERT("Must be > 0", not m_readyWorkers.empty());
  auto nextWorker = m_readyWorkers.front();
  m_readyWorkers.pop_front();

  average("data_size", dataSize);
  average("data_size_to[" + nextWorker + "]", dataSize);

  increment("sent_events");
  increment("sent_events[" + nextWorker + "]");

  timeit("event_rate");
  timeit<200>("event_rate_to[" + nextWorker + "]");

  m_socket->send(ZMQMessageHelper::createZMQMessage(nextWorker), zmq::send_flags::sndmore);
  ZMQParent::send(m_socket, std::move(message));

  log("ready_queue_size", static_cast<long>(m_readyWorkers.size()));
  decrement("ready_messages[" + nextWorker + "]");
}

void ZMQLoadBalancedOutput::handleIncomingData()
{
  auto readyMessage = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);
  B2ASSERT("Should be a ready message", readyMessage->isMessage(EMessageTypes::c_readyMessage));

  // Register it as another ready worker
  const auto toIdentity = readyMessage->getIdentity();
  m_readyWorkers.push_back(toIdentity);

  if (m_allWorkers.emplace(toIdentity).second) {
    // Aha, we did never see this worker so far, so add it to our list.
    if (m_sentStopMessages) {
      // If it turned up late (everyone else has already stopped), send a stop message directly
      auto sendMessage = ZMQMessageFactory::createMessage(toIdentity, EMessageTypes::c_lastEventMessage);
      ZMQParent::send(m_socket, std::move(sendMessage));
    }

    if (m_sentTerminateMessages) {
      // If it turned up late (everyone else has already terminates), send a terminate message directly
      auto sendMessage = ZMQMessageFactory::createMessage(toIdentity, EMessageTypes::c_terminateMessage);
      ZMQParent::send(m_socket, std::move(sendMessage));
    }
  }

  log("ready_queue_size", static_cast<long>(m_readyWorkers.size()));
  log("registered_workers", static_cast<long>(m_allWorkers.size()));
  increment("ready_messages[" + toIdentity + "]");
}

void ZMQLoadBalancedOutput::clear()
{
  m_sentStopMessages = false;
  m_sentTerminateMessages = false;

  log("all_stop_messages", static_cast<long>(m_sentStopMessages));
  log("all_terminate_messages", static_cast<long>(m_sentTerminateMessages));
}

bool ZMQLoadBalancedOutput::isReady() const
{
  // if we are lax, we are always ready. If not, we need to have at least a single ready worker. This prevents the B2ASSERT to fail.
  return m_lax or not m_readyWorkers.empty();
}
