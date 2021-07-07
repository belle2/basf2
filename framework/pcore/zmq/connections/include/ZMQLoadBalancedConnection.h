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

#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

#include <zmq.hpp>

#include <string>
#include <memory>
#include <set>
#include <deque>

namespace Belle2 {
  /**
   * Input part of a load-balanced connection.
   *
   * This part is quite strait forward: it connects via a ZMQ_DEALER socket
   * to a single output and registers itself by sending a defined number
   * of ready messages.
   * For each event message it receives it answers with a ready message.
   * All messages are passed on.
   *
   * Please note that only event messages (and raw event messages) are answered
   * with a ready message.
   */
  class ZMQLoadBalancedInput : public ZMQConnectionOverSocket {
  public:
    /// Create a new load-balanced input connecting to the given address. Send bufferSize ready messages.
    ZMQLoadBalancedInput(const std::string& inputAddress, unsigned int bufferSize,
                         const std::shared_ptr<ZMQParent>& parent);

    /// Answer event messages with a ready message and pass on every received message.
    std::unique_ptr<ZMQNoIdMessage> handleIncomingData();
  };

  /**
   * Output part of a load-balanced connection.
   *
   * Multiple inputs can connect to this output. The output keeps a list of all
   * received ready messages and sends events on requests always to the next
   * input in the list, which creates some sort of load-balancing.
   *
   * Stop and terminate messages are sent all all inputs that have sent at least
   * a single ready so far. There is no unregistration happening, so dead inputs will
   * still be served with both events and stop/terminate messages. For stop/terminate
   * messages this is no problem and event messages will only be sent as long as there
   * are ready messages.
   *
   * After a stop message is sent all additional incoming events will be dismissed.
   *
   * If no input has send any ready message, the output is not ready and needs to be
   * polled for new ready messages. However, there is also a "lax" mode which
   * makes the events being silently discarded if no input is ready.
   *
   * Internally a ZMQ_ROUTER in bind mode is used.
   */
  class ZMQLoadBalancedOutput : public ZMQConnectionOverSocket {
  public:
    /// Create a new load-balanced output and bind to the given address.
    ZMQLoadBalancedOutput(const std::string& outputAddress, bool lax, const std::shared_ptr<ZMQParent>& parent);

    /**
     * Send the given message (without identity) to the next input
     * in the ready list.
     * If it is a stop or terminate message send the message to all inputs
     * which have ever sent a ready message.
     * If it is an event message, sent it only to the next ready input.
     * If there is no ready input it either
     * (a) throws and exception if lax mode is not enabled. Make sure to only
     *     sent events if the output is ready (which indicates exactly this: an
     *     input is ready). Or
     * (b) discard the event without warning if lax mode is enabled.
     */
    void handleEvent(std::unique_ptr<ZMQNoIdMessage> message);

    /// Block until a ready message from an input is received and add it to the ready queue
    void handleIncomingData();
    /// Clear the counter for sent stop and terminate messages. Should be called on run start.
    void clear();
    /// If lax mode is disabled, the output is ready if at least a single input is ready. Else always.
    bool isReady() const final;

  protected:
    /// List of identities of ready inputs in LIFO order
    std::deque<std::string> m_readyWorkers;
    /// All ever registered inputs
    std::set<std::string> m_allWorkers;
    /// Did we already sent a stop message?
    bool m_sentStopMessages = false;
    /// Did we already sent a terminate message?
    bool m_sentTerminateMessages = false;
    /// Parameter to enable lax mode
    bool m_lax = false;
  };
}