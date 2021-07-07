/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <thread>
#include <chrono>
#include <set>

using namespace Belle2;

ZMQConfirmedInput::ZMQConfirmedInput(const std::string& inputAddress, const std::shared_ptr<ZMQParent>& parent)
  : ZMQConnectionOverSocket(parent)
{
  // These are all the log output we will have, set to 0 in the beginning.
  log("last_received_message", "");
  log("total_number_messages", 0l);
  log("registered_workers",  0l);
  log("hello_messages",  0l);
  log("dead_workers",  0l);
  log("all_stop_messages", 0l);
  log("sent_stop_messages",  0l);
  log("last_stop_sent",  "");
  log("received_stop_messages", 0l);
  log("all_terminate_messages", 0l);
  log("sent_terminate_messages",  0l);
  log("last_terminate_sent",  "");
  log("received_terminate_messages", 0l);
  log("received_messages_after_stop", 0l);
  log("last_received_event_message", "");
  log("last_clear", "");
  log("stop_overwrites", 0l);
  log("last_stop_overwrite", "");

  log("data_size", 0.0);
  log("received_events", 0l);
  log("event_rate", 0.0);

  // Create a binding socket of router type
  m_socket = m_parent->createSocket<ZMQ_ROUTER>(inputAddress, true);
}

std::unique_ptr<ZMQIdMessage> ZMQConfirmedInput::handleIncomingData()
{
  auto message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);
  const auto fromIdentity = message->getIdentity();

  logTime("last_received_message");
  increment("total_number_messages");
  increment("total_number_messages_from[" + fromIdentity + "]");

  auto confirmMessage = ZMQMessageFactory::createMessage(fromIdentity, EMessageTypes::c_confirmMessage);
  ZMQParent::send(m_socket, std::move(confirmMessage));

  if (message->isMessage(EMessageTypes::c_helloMessage)) {
    // a hello message makes us register the worker identity - which is the identity of the sender of the message
    m_registeredWorkersInput.emplace(fromIdentity);
    log("registered_workers", static_cast<long>(m_registeredWorkersInput.size()));
    increment("hello_messages");
    increment("hello_messages_from[" + fromIdentity + "]");
    return {};
  } else if (message->isMessage(EMessageTypes::c_deleteWorkerMessage)) {
    // a delete message makes us forget about the worker identity. The identity is taken from the message data
    // making it possible to delete other workers.
    B2DEBUG(10, "Got message from " << message->getIdentity() << " to kill " << message->getMessagePartAsString<2>());
    const std::string& killedIdentity = message->getMessagePartAsString<2>();
    m_registeredWorkersInput.erase(killedIdentity);

    log("registered_workers", static_cast<long>(m_registeredWorkersInput.size()));
    increment("dead_workers");
    increment("dead_worker_messaged_from[" + fromIdentity + "]");

    if (m_registeredWorkersInput.empty()) {
      B2ERROR("There is not a single worker registered anymore!");
      return {};
    }

    // Corner case: could be that this was the one we were waiting for
    if (not m_allStopMessages and m_receivedStopMessages == m_registeredWorkersInput) {
      m_allStopMessages = true;
      log("all_stop_messages", static_cast<long>(m_allStopMessages));
      increment("sent_stop_messages");
      logTime("last_stop_sent");

      return ZMQMessageFactory::createMessage(killedIdentity, EMessageTypes::c_lastEventMessage);
    }
    // Corner case: could be that this was the one we were waiting for
    if (not m_allTerminateMessages and m_receivedTerminateMessages == m_registeredWorkersInput) {
      m_allTerminateMessages = true;
      log("all_terminate_messages", static_cast<long>(m_allTerminateMessages));
      increment("sent_terminate_messages");
      logTime("last_terminate_sent");

      return ZMQMessageFactory::createMessage(killedIdentity, EMessageTypes::c_terminateMessage);
    }

    return {};
  }

  B2ASSERT("Worker without proper registration!",
           m_registeredWorkersInput.find(fromIdentity) != m_registeredWorkersInput.end());

  if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
    // Increment the stop messages
    m_receivedStopMessages.emplace(fromIdentity);
    log("received_stop_messages", static_cast<long>(m_receivedStopMessages.size()));
    increment("total_received_stop_messages");

    if (not m_allStopMessages and m_receivedStopMessages == m_registeredWorkersInput) {
      // But only return this if everyone has sent a stop message already
      m_allStopMessages = true;
      log("all_stop_messages", static_cast<long>(m_allStopMessages));
      increment("sent_stop_messages");
      logTime("last_stop_sent");

      return ZMQMessageFactory::createMessage(fromIdentity, EMessageTypes::c_lastEventMessage);
    }

    // Whatever we return here will be carried on to the application and eventually also to the output.
    // This means as we are not passing the stop message now, we return nothing.
    return {};
  } else if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    // Increment the terminate messages
    m_receivedTerminateMessages.emplace(fromIdentity);
    log("received_terminate_messages", static_cast<long>(m_receivedTerminateMessages.size()));
    increment("total_received_terminate_messages");

    if (not m_allTerminateMessages and m_receivedTerminateMessages == m_registeredWorkersInput) {
      // But only return this if everyone has sent a terminate message already
      m_allTerminateMessages = true;
      log("all_terminate_messages", static_cast<long>(m_allTerminateMessages));
      increment("sent_terminate_messages");
      logTime("last_terminate_sent");

      return ZMQMessageFactory::createMessage(fromIdentity, EMessageTypes::c_terminateMessage);
    }

    // Whatever we return here will be carried on to the application and eventually also to the output.
    // This means as we are not passing the stop message now, we return nothing.
    return {};
  }

  if (m_allStopMessages) {
    B2ERROR("Received an event after having received stop messages from every worker. This is not a good sign! I will dismiss this event!");
    increment("received_messages_after_stop");
    return {};
  }

  // Now it can only be a plain normal data message, so just pass it on
  const auto dataSize = message->getDataMessage().size();

  average("data_size", dataSize);
  average("data_size_from[" + fromIdentity + "]", dataSize);

  increment("received_events");
  increment("received_events[" + fromIdentity + "]");

  timeit("event_rate");
  timeit<200>("event_rate_from[" + fromIdentity + "]");

  logTime("last_received_event_message");

  return message;
}

void ZMQConfirmedInput::clear()
{
  // We clear all our internal state and counters
  m_receivedStopMessages.clear();
  m_allStopMessages = false;
  m_receivedTerminateMessages.clear();
  m_allTerminateMessages = false;

  log("received_stop_messages", static_cast<long>(m_receivedStopMessages.size()));
  log("all_stop_messages", static_cast<long>(m_allStopMessages));
  log("received_terminate_messages", static_cast<long>(m_receivedTerminateMessages.size()));
  log("all_terminate_messages", static_cast<long>(m_allTerminateMessages));

  logTime("last_clear");
}

std::unique_ptr<ZMQIdMessage> ZMQConfirmedInput::overwriteStopMessage()
{
  if (not m_allStopMessages) {
    // We did not already receive all stop messages, but someone externally asked us to stop anyways. So lets do it.
    B2ERROR("Sending out a stop message although not all of the workers are finished already!");
    increment("stop_overwrites");
    logTime("last_stop_overwrite");

    m_allStopMessages = true;
    log("all_stop_messages", static_cast<long>(m_allStopMessages));
    increment("sent_stop_messages");
    logTime("last_stop_sent");

    return ZMQMessageFactory::createMessage("", EMessageTypes::c_lastEventMessage);
  }

  // We have already stopped, no need to sent it twice.
  return {};
}

ZMQConfirmedOutput::ZMQConfirmedOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent)
  : ZMQConnectionOverSocket(parent)
{
  // These are all the log output we will have, set to 0 in the beginning.
  log("no_confirmation_message", 0l);
  log("last_sent_event_message", 0l);
  log("data_size", 0.0);
  log("sent_events", 0l);
  log("event_rate", 0.0);
  log("timespan_waiting_for_confirmation", 0l);

  // Register a non-binding DEALER socket
  m_socket = m_parent->createSocket<ZMQ_DEALER>(outputAddress, false);

  logTime("last_hello_sent");

  // Say hello to the receiver end of the connection (which is a confirmed input)
  auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_helloMessage);
  handleEvent(std::move(message));
}

void ZMQConfirmedOutput::handleEvent(std::unique_ptr<ZMQNoIdMessage> message, bool requireConfirmation, int maximalWaitTime)
{
  auto current = std::chrono::system_clock::now();
  while (m_waitingForConfirmation > 0) {
    // Wait for all conformation messages that are still pending.
    if (ZMQParent::poll({m_socket.get()}, maximalWaitTime)) {
      handleIncomingData();
    } else if (requireConfirmation) {
      B2FATAL("Did not receive a confirmation message in time!");
    } else {
      B2WARNING("Did not receive a confirmation message in time!");
      increment("no_confirmation_message");
      // If we did not receive one, we will also not receive the next one so lets break out.
      break;
    }
  }
  auto afterWaiting = std::chrono::system_clock::now();
  m_timespanWaitingForConfirmation += std::chrono::duration_cast<std::chrono::milliseconds>(afterWaiting - current).count();
  log("timespan_waiting_for_confirmation", m_timespanWaitingForConfirmation);

  // We have received a confirmation for the old, so we can also sent a new message.

  // TODO: makes no sense for signal messages!
  const auto dataSize = message->getDataMessage().size();

  average("data_size", dataSize);
  increment("sent_events");
  timeit("event_rate");

  logTime("last_sent_event_message");

  ZMQParent::send(m_socket, std::move(message));
  m_waitingForConfirmation++;
}

void ZMQConfirmedOutput::handleIncomingData()
{
  // This should only ever be a confirmation message
  B2ASSERT("There should be no data coming here, if we have already a confirmation!", m_waitingForConfirmation > 0);
  auto acceptMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
  B2ASSERT("Should be an accept message", acceptMessage->isMessage(EMessageTypes::c_confirmMessage));
  m_waitingForConfirmation--;
}
