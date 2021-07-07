/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/utils/StreamHelper.h>

#include <string>
#include <map>

namespace Belle2 {
  /**
   * Class to monitor all started framework processes (input, workers, output),
   * kill them if requested and handle the signals from the OS.
   *
   * Is used in the ZMQ event processor and can only be used together with ZMQ.
   */
  class ProcessMonitor {
  public:
    /// Start listening for process management information on the given address.
    void subscribe(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                   const std::string& controlSocketAddress);
    /// Block until either the input process is running or the timeout (in seconds) is raised.
    void waitForRunningInput(int timeout);
    /// Block until either the output process is running or the timeout (in seconds) is raised.
    void waitForRunningOutput(int timeout);
    /// Block until either the worker process is running or the timeout (in seconds) is raised.
    void waitForRunningWorker(int timeout);
    /// Ask all processes to terminate. If not, kill them after timeout seconds.
    void killProcesses(unsigned int timeout);

    /// Init the processing with that many workers
    void initialize(unsigned int requestedNumberOfWorkers);

    /// Terminate the processing
    void terminate();

    /// Reset the internal state
    void reset();

    /// check multicast for messages and kill workers if requested
    void checkMulticast(int timeout = 0);
    /// check the child processes, if one has died
    void checkChildProcesses();
    /// check if we have received any signal from the user or OS. Kill the processes if not SIGINT.
    void checkSignals(int g_signalReceived);

    /// If we have received a SIGINT signal or the last process is gone, we can end smoothly
    bool hasEnded() const;

    /// Compare our current list of workers of how many we want to have
    unsigned int needMoreWorkers() const;

    /// Check if there is at least one running worker
    bool hasWorkers() const;

  private:
    /// The client used for message processing
    ZMQClient m_client;

    /// How many workers we should request to start
    unsigned int m_requestedNumberOfWorkers = 0;
    /// The current list of pid -> process types (to be compared to the proc handler)
    std::map<int, ProcType> m_processList;
    /// Someone requested us to end the processing
    bool m_hasEnded = false;

    /// The data store streamer
    StreamHelper m_streamer;

    /// Did we already receive the statistics?
    bool m_receivedStatistics = false;

    /// Cound the number of processes with a certain type
    unsigned int processesWithType(const ProcType& procType) const;

    /// Process a message from the multicast
    template <class ASocket>
    void processMulticast(const ASocket& socket);
  };
} // namespace Belle2
