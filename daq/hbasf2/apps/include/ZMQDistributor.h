/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/hbasf2/apps/ZMQApp.h>
#include <framework/pcore/zmq/connections/ZMQLoadBalancedConnection.h>
#include <framework/pcore/zmq/connections/ZMQRawConnection.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace Belle2 {
  /**
   * Standard distributor app: receive data via a raw connection (e.g. from event builder)
   * and send them out to any ready worker (via a load-balanced connection).
   * If expressRecoMode is not set, wait until a worker is ready (otherwise dismiss events).
   * As there are not signal messages on input, does not react on anything from input.
   * When receiving a stop on monitoring, it starts a counter. Once there has not been any event for N second, it sends a stop to all clients.
   * When receiving a start on monitoring, it clears the output.
   * When receiving a terminate on monitoring, it sends out a terminate message to all clients and terminated itself.
   */
  class ZMQDistributor : public ZMQStandardApp<ZMQRawInput, ZMQLoadBalancedOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Handle stop, start and terminate messages as described above
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the output connection (only data messages)
    void handleInput() final;
    /// When a timeout is set (= we are waiting for all messages after a stop), send a stop message once we have not seen any more events
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
    /// Parameter: buffer size for storing input messages
    unsigned int m_maximalBufferSize = 80'000'000;
    /// Parameter: Do not wait for a ready worker if set to true, but dismiss the incoming event
    bool m_expressRecoMode = false;
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };

  class ZMQInputAdapter : public ZMQStandardApp<ZMQRawInput, ZMQConfirmedOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Send a stop message on stop or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the output connection (if there is a message)
    void handleInput() final;
    /// When a timeout is set (= we are waiting for all messages after a stop), send a stop message once we have not seen any more events
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
    /// Parameter: buffer size for storing input messages
    unsigned int m_maximalBufferSize = 80'000'000;
    /// Parameter: Choose how the input events are formatted
    bool m_expressRecoMode = false;
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };
}