/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQLoadBalancedConnection.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ZMQLoadBalancedInput::ZMQLoadBalancedInput(const std::string& inputAddress, unsigned int bufferSize,
                                           const std::shared_ptr<ZMQParent>& parent) : ZMQConnectionOverSocket(parent)
{
  log("sent_ready", 0l);
  log("data_size", 0.0);
  log("received_events", 0l);
  log("event_rate", 0.0);

  m_socket = m_parent->createSocket<ZMQ_DEALER>(inputAddress, false);

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
    auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
    ZMQParent::send(m_socket, std::move(readyMessage));
    increment("sent_ready");

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

  m_socket->send(ZMQMessageHelper::createZMQMessage(nextWorker), ZMQ_SNDMORE);
  ZMQParent::send(m_socket, std::move(message));

  log("ready_queue_size", static_cast<long>(m_readyWorkers.size()));
  decrement("ready_messages[" + nextWorker + "]");
}

void ZMQLoadBalancedOutput::handleIncomingData()
{
  auto readyMessage = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);
  B2ASSERT("Should be a ready message", readyMessage->isMessage(EMessageTypes::c_readyMessage));

  const auto toIdentity = readyMessage->getIdentity();
  m_readyWorkers.push_back(toIdentity);

  if (m_allWorkers.find(toIdentity) == m_allWorkers.end()) {
    m_allWorkers.emplace(toIdentity);

    if (m_sentStopMessages) {
      auto sendMessage = ZMQMessageFactory::createMessage(toIdentity, EMessageTypes::c_lastEventMessage);
      ZMQParent::send(m_socket, std::move(sendMessage));
    }

    if (m_sentTerminateMessages) {
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
  return m_lax or not m_readyWorkers.empty();
}
