/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/hbasf2/apps/ZMQApp.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>
#include <framework/pcore/zmq/connections/ZMQLoadBalancedConnection.h>
#include <framework/pcore/zmq/connections/ZMQRawConnection.h>
#include <framework/pcore/zmq/connections/ZMQNullConnection.h>

namespace Belle2 {
  /**
   * App to only send a ready on every incoming message - behaves the same as the input of a worker (but without an output).
   * The incoming message itself will be dropped. Can be used for testing purposes.
   * Apart from the connection-typical behavior, no special handling for different message types.
   */
  class ZMQReadySender : public ZMQStandardApp<ZMQLoadBalancedInput, ZMQNullConnection> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: buffer size
    unsigned int m_bufferSize = 20;
  };

  /**
   * App to only send a confirmation on every incoming message - behaves the same as the input of a (final)collector (but without an output).
   * The incoming message itself will be dropped. Can be used for testing purposes.
   * Apart from the connection-typical behavior, no special handling for different message types.
   */
  class ZMQAcceptor : public ZMQStandardApp<ZMQConfirmedInput, ZMQNullConnection> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
  };

  /**
   * App to mimick a basf2-worker by accepting an incoming message with a ready message and sending it out via the output -
   * requiring a confirmation message from the next step. Does not do any serialization or reconstruction.
   * Can be used for testing purposes.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   */
  class ZMQWorker : public ZMQStandardApp<ZMQLoadBalancedInput, ZMQConfirmedOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Transport the message from the input to the output connection
    void handleInput() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
    /// Parameter: buffer size
    unsigned int m_bufferSize = 20;
  };
}