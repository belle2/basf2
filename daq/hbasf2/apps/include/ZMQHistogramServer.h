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
#include <daq/hbasf2/connections/ZMQHistogramOutput.h>

namespace po = boost::program_options;

namespace Belle2 {
  /**
   * Final histogram app: receive histogram messages from all clients and react with a
   * confirmation message. Periodically or on stop/terminate, merge them and write them out to a
   * ROOT file and optionally to a memory file in shared memory.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * Behaves as a collector:
   * When receiving a stop on monitoring, it merges the histograms (even if not all clients have sent a stop already).
   * When receiving a start on monitoring, it clears the input and output stop counters and the stored histograms.
   */
  class ZMQHistogramToFileServer : public ZMQStandardApp<ZMQConfirmedInput, ZMQHistoServerToFile> {
  protected:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Merge on stop (even if not all clients have stopped) or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the histogram storage
    void handleInput() final;
    /// Call the mergeAndSend function on timeout
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: name of the shared memory (or empty)
    std::string m_sharedMemoryName = "";
    /// Parameter: name of the root file - can include {run_number} or {experiment_number}.
    std::string m_rootFileName;
    /// Parameter: size of the temporary internal compression buffer
    unsigned int m_maximalUncompressedBufferSize = 128'000'000;
  };

  /**
   * Non-Final histogram app: receive histogram messages from all clients and react with a
   * confirmation message. Periodically or on stop/terminate, merge them and send them out to the
   * next histogram server. Good for building up hierarchical structures e.g. because not all
   * histograms can be handled at the same time.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * Behaves as a collector:
   * When receiving a stop on monitoring, it merges the histograms (even if not all clients have sent a stop already).
   * When receiving a start on monitoring, it clears the input and output stop counters and the stored histograms.
   */
  class ZMQHistogramToZMQServer : public ZMQStandardApp<ZMQConfirmedInput, ZMQHistoServerToZMQ> {
  public:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Merge on stop (even if not all clients have stopped) or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the histogram storage
    void handleInput() final;
    /// Call the mergeAndSend function on timeout
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
    /// Parameter: size of the temporary internal compression buffer
    unsigned int m_maximalUncompressedBufferSize = 128'000'000;
  };

  /**
   * Non-Final histogram app: receive histogram messages from all clients and react with a
   * confirmation message. Periodically or on stop/terminate, merge them and send them out to an old
   * implementation of the histogram server. Good for testing in combination with the legacy system.
   * Apart from the connection-typical behavior, it reacts on termination messages by terminating.
   * Behaves as a collector:
   * When receiving a stop on monitoring, it merges the histograms (even if not all clients have sent a stop already).
   * When receiving a start on monitoring, it clears the input and output stop counters and the stored histograms.
   */
  class ZMQHistogramToRawServer : public ZMQStandardApp<ZMQConfirmedInput, ZMQHistoServerToRaw> {
  public:
    /// Initialize the two connections using the command line arguments.
    void initialize() final;
    /// Add the parameters to the cmd line arguments.
    void addOptions(po::options_description& desc) final;
    /// Merge on stop (even if not all clients have stopped) or clear the counters on start from the monitoring connection
    void handleExternalSignal(EMessageTypes type) final;
    /// Pass the message from the input connection to the histogram storage
    void handleInput() final;
    /// Call the mergeAndSend function on timeout
    void handleTimeout() final;

  private:
    /// Parameter: input address
    std::string m_inputAddress;
    /// Parameter: output address
    std::string m_outputAddress;
    /// Parameter: size of the temporary internal compression buffer
    unsigned int m_maximalUncompressedBufferSize = 128'000'000;
  };
}