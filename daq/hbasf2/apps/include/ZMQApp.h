/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/pcore/zmq/connections/ZMQSimpleConnection.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/logging/Logger.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

namespace Belle2 {
  /**
   * Generic base class for all standalone ZMQ applications.
   * It always consists of three connections, two of them are given as template parameters:
   * the input and the output connection.
   * By default, the main loop listens on the output and monitoring socket(s)
   * while the output is not ready and also listens on the input if the output is ready.
   *
   * Before starting the main loop, the virtual initialize() function should be called.
   * When the input socket receives a message, the virtual "handleInput" function is called
   * (defaults to handleIncomingData of the connection).
   * When the output socket receives a message, the virtual "handleOutput" function is called
   * (defaults to handleIncomingData of the connection).
   * When the monitoring receives a signal message, the virtual "handleExternalSignal" function is called
   * (defaults to nothing).
   * When the timeout is set to a value other than 0, the virtual handleTimeout function is called
   * with the given frequency (defaults to nothing).
   *
   * Be reimplementing those four functions (or parts of them), the behaviour can be customized.
   */
  template<class AInputConnection, class AOutputConnection>
  class ZMQStandardApp  {
  public:
    /**
     * Should be called before the main() function to initialize the connections using the paremeters given on command line.
     * Custom implementations should implement the addOptions function to pass the command line arguments
     * correctly.
     * Calls the initialize function (which should also be overridden).
     */
    void initFromConsole(const std::string& description, int argc, char* argv[]);

    /**
     * Start the main loop polling on the output and monitoring connections and eventually also on the input if the output is ready.
     * Calls the functions handleExternalSignal, handleInput, handleOutput and handleTimeout as described in the documentation of
     * this class.
     */
    void main();

  protected:
    /// Pointer to the ZMQParent to be used as base for all connections
    std::shared_ptr<ZMQParent> m_parent;
    /// Pointer to the input connection. Should be set in initialize.
    std::unique_ptr<AInputConnection> m_input;
    /// Pointer to the output connection. Should be set in initialize.
    std::unique_ptr<AOutputConnection> m_output;
    /// Pointer to the monitoring connection. Should be set in initialize.
    std::unique_ptr<ZMQSimpleConnection> m_monitor;
    /// Can be set by functions to terminate the main loop at the next possibility.
    bool m_terminate = false;
    /// If set to a value != 0, will call handleTimeout with this frequency (in seconds).
    unsigned int m_timeout = 0;
    /// Flag to break out of the polling loop to check for monitoring messages. Except for the finalcollector you probably do not want this.
    bool m_monitorHasPriority = false;

    /// Override in a derived class to initialize the connections from the given command line arguments. Do not forget to call this base function also.
    virtual void initialize();
    /// Override in a derived class to add the command line arguments. Do not forget to call this base function also.
    virtual void addOptions(po::options_description& desc);
    /// Will get called for every signal message on the monitoring connection. Can be overridden in a derived class. Empty by default.
    virtual void handleExternalSignal(EMessageTypes);
    /// Will get called for every message on the input connection. Can be overridden in a derived class. Calls handleIncomingData by default.
    virtual void handleInput();
    /// Will get called for every message on the output connection. Can be overridden in a derived class. Calls handleIncomingData by default.
    virtual void handleOutput();
    /// Will get called on a timeout. Can be overridden in a derived class. Empty by default.
    virtual void handleTimeout();
    /// Using the connections, fill up a buffer with the content to be monitored
    virtual void fillMonitoringJSON(std::stringstream& buffer) const;
    /// Check if the main loop will be exited on next occasion. Can be set via the "m_terminate" flag.
    bool terminated() const;
    /// Helper function to reset the start time and the remaining time
    void resetTimer();
    /// Poll until a single event is retreived
    void pollEvent(bool pollOnInput);

  private:
    /// Internal signal handler
    HLTMainLoop m_mainLoop;
    /// Storage for the monitoring address for the cmd arguments.
    std::string m_monitoringAddress;
    /// Counter for the remaining time until a timeout happens
    int m_remainingTime = -1;
    /// Start time for the timeout
    std::chrono::system_clock::time_point m_start;

    /// Helper function to check, if the timeout should happen
    bool checkTimer();
    /// Helper function to update the remaining time
    void updateTimer();
    /// Handle an incoming message on the monitoring socket by either calling handleExternalSignal() or by passing on the monitor JSONs of the connections.
    void handleMonitoring();
  };
}

/// Template implementation details in a second header
#include <daq/hbasf2/apps/ZMQApp.details.h>
