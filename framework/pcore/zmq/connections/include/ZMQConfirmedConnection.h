/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/pcore/zmq/connections/ZMQConnection.h>

#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>

#include <set>
#include <string>
#include <memory>


namespace Belle2 {
  /**
   * Input part of a confirmed connection.
   *
   * In a confirmed connection every message sent by the input to the output is confirmed
   * by a confirmation message by the output. The output can handle multiple inputs, each
   * input can only be connected to a single output.
   *
   * The input part keeps track of the registered workers and if it has received
   * all stop and/or terminate messages. Stop/Terminate messages are only passed on if
   * received from all registered workers.
   *
   * If the input receives a message from one of the outputs,
   * it sends back a confirmation message. Depending on the message type, it
   * does several things:
   * * if the message is a hello message (the input registered itself), it adds the input
   *   identity to the registered workers. Nothing is returned to the caller in this case.
   * * if the message is a delete message (someone unregistered a worker), it removes
   *   the specified input identity in the message from the registered workers.
   *   Nothing is returned to the caller in this case. (corner case: if it turns out
   *   the worker to be unregistered was the last one missing for a full stop or
   *   terminate message from all workers, this message is passed on)
   * * if the message is a stop message, add it to the received stop messages. Only if
   *   all registered workers have send a stop message pass it on.
   * * same for terminate messages
   * * in all other cases just pass on the message.
   *
   * Internally, a ZMQ_ROUTER is used in bind-mode. The order
   * of starting output and input does not play any role.
   */
  class ZMQConfirmedInput : public ZMQConnectionOverSocket {
  public:
    /// Create a new confirmed output by binding to the address.
    ZMQConfirmedInput(const std::string& inputAddress, const std::shared_ptr<ZMQParent>& parent);

    /**
     * Block until a message can be received from one of the inputs.
     * React as described in the general description of this class.
     * Only if a message is to be passed on actually returns a message
     * (in all other cases a nullptr).
     */
    std::unique_ptr<ZMQIdMessage> handleIncomingData();

    /// Reset the counters for all received stop and terminate messages. Should be called on run start.
    void clear();

    /// Manually overwrite the stop message counter and set it to have all stop messages received.
    std::unique_ptr<ZMQIdMessage> overwriteStopMessage();

  private:
    /// The set of input identities which have already sent a stop message.
    std::set<std::string> m_receivedStopMessages;
    /// Have we received all stop messages?
    bool m_allStopMessages = false;
    /// The set of input identities which have already sent a terminate message.
    std::set<std::string> m_receivedTerminateMessages;
    /// Have we received all terminante messages?
    bool m_allTerminateMessages = false;
    /// The set of all registered inputs
    std::set<std::string> m_registeredWorkersInput;
  };

  /**
   * Output part of a confirmed connection.
   *
   * For an introduction to confirmed connections please see the ZMQConfirmedInput.
   *
   * The confirmed output basically just sends out messages and waits for
   * confirmations. Two additions to that:
   * * on startup, it sends a single hello message to the output to register itself.
   *   This message also needs to be confirmed.
   * * Instead of waiting for a confirmation after sending the message, the
   *   blocking happens before sending the next message. This gives the output
   *   some time for reacting without slowing down the output part.
   *
   * If no confirmation is received on time, either a FATAL is issued or (configurable)
   * only a warning.
   *
   * Internally, a ZMQ_DEALER socket is used in non-bind mode. The order
   * of starting output and input does not play any role.
   */
  class ZMQConfirmedOutput : public ZMQConnectionOverSocket {
  public:
    /// Create a new confirmed output by connecting to the address.
    ZMQConfirmedOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent);

    /**
     * Send the message to the output (a message without a ID as there is only a single output).
     * If requireConfirmation is set to true and no confirmation (from the last sent event!)
     * is received before sending the new event, a B2FATAL is issued (in other cases only a warning).
     * Set the maximalWaitTime to -1 to always wait infinitely (not preferred).
     */
    void handleEvent(std::unique_ptr<ZMQNoIdMessage> message, bool requireConfirmation = true, int maximalWaitTime = 10000);

    /**
     * Blocks until it can receive the (hopefully confirmation) message from the output.
     * Can be called even before sending an event to receive the confirmation message
     * ahead of time (e.g. when there is nothing else to do).
     */
    void handleIncomingData();

  private:
    /// On how many confirmation messages are we still waiting?
    unsigned int m_waitingForConfirmation = 0;
    /// Internal monitoring how long we were waiting for confirmation messages.
    long m_timespanWaitingForConfirmation = 0;
  };
}