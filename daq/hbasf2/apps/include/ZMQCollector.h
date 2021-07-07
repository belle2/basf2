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
#include <daq/hbasf2/connections/ZMQROIConnection.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace Belle2 {
  /**
   * Normal collector app: receive messages on the input reacting with a confirmation message and sends them out via a load-balanced connection to ready workers.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * When receiving a stop on monitoring, it starts a counter. Once there has not been any event for N second, it sends a stop to all clients.
   * (even if not all clients have sent a stop)
   * When receiving a start on monitoring, it clears the input and output stop counters.
   */
  class ZMQCollector : public ZMQStandardApp<ZMQConfirmedInput, ZMQLoadBalancedOutput> {
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
    /// Parameter: Do not wait for a ready worker if set to true, but dismiss the incoming event
    bool m_lax = false;
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };

  /**
   * Special collector app for translating between ZMQ and raw connections:
   * send ready messages (like a typical worker) on the input and send all received events to the raw connection at output.
   * Can be used as a buffered output sink (as the normal raw connection is always ready).
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   */
  class ZMQOutputAdapter : public ZMQStandardApp<ZMQLoadBalancedInput, ZMQRawOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Send a stop message on stop or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the output connection (if there is a message)
    void handleInput() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
  };

  /**
   * Special collector app: receive messages on the input reacting with a confirmation message and sends them out via a confirmed connection to ready workers.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * When receiving a stop on monitoring, it starts a counter. Once there has not been any event for N second, it sends a stop to all clients.
   * (even if not all clients have sent a stop)
   * When receiving a start on monitoring, it clears the input and output stop counters.
   */
  class ZMQProxyCollector : public ZMQStandardApp<ZMQConfirmedInput, ZMQConfirmedOutput> {
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
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };

  /**
   * Final collector app: receive messages on the input reacting with a confirmation message and sends them out via a raw connection (e.g. to storage).
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * Only data messages are passed to the raw output.
   * When receiving a stop on monitoring, it starts a counter. Once there has not been any event for N second, it resets it stop counter
   * (even if not all clients have sent a stop)
   * When receiving a start on monitoring, it clears the input stop counters.
   */
  class ZMQFinalCollector : public ZMQStandardApp<ZMQConfirmedInput, ZMQRawOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Set the stop message counter on stop or clear the counters on start from the monitoring connection
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
    /// Parameter: add the event size at the beginning of the message
    bool m_addEventSize;
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };

  /// Special form of the ZMQFinalCollector for sending out the additional data message to a ROI receiver.
  class ZMQFinalCollectorWithROI : public ZMQStandardApp<ZMQConfirmedInput, ZMQDataAndROIOutput> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Set the stop message counter on stop or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the output connection (data message to first, additional data message to second address)
    void handleInput() final;
    /// Special handling of the JSON function with additonal ROI
    void fillMonitoringJSON(std::stringstream& buffer) const final;
    /// When a timeout is set (= we are waiting for all messages after a stop), send a stop message once we have not seen any more events
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address for data (first part of message)
    std::string m_dataOutputAddress;
    /// Parameter: output address for ROIs (second part of message)
    std::string m_roiOutputAddress;
    /// Parameter: add the event size at the beginning of the message
    bool m_addEventSize;
    /// Parameter: how long to wait after no events come anymore
    unsigned int m_stopWaitingTime = 2;
  };
}