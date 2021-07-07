/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/connections/ZMQConnection.h>
#include <daq/hbasf2/utils/HistogramMapping.h>

#include <framework/pcore/zmq/connections/ZMQNullConnection.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>
#include <framework/pcore/zmq/connections/ZMQRawConnection.h>

#include <framework/pcore/zmq/messages/ZMQIdMessage.h>

#include <framework/pcore/EvtMessage.h>
#include <daq/dqm/DqmMemFile.h>
#include <daq/rfarm/manager/SharedMem.h>

#include <string>
#include <memory>
#include <map>
#include <optional>

namespace Belle2 {
  /**
   * Output histograms into a ROOT file and shared memory after merging.
   * This "connection" class needs to be wrapped with a ZMQHistogramOutput
   * before using it as a real output connection (which adds the common functionalities).
   *
   * The ROOT and shared memory are written using ROOTs own serialization. The shared memory
   * is herby filled with the content of a temporary TMemFile.
   * On clear the shared memory is cleared (ROOT files stay what they are).
   *
   * Nothing special happens on stop or terminate other than that a merge is performed.
   *
   * The given file path for the root file can include placeholders
   * "{experiment_number}" and "{run_number}" which will be replaced accordingly.
   * If no root file output name or no shared memory file for the DQMMemFile is given,
   * this feature is not used.
   *
   * Please note that we do not call the destructor of the shared memory on purpose
   * as this leads to problems.
   */
  class ZMQHistoServerToFileOutput : public ZMQConnection {
  public:
    /// Create a new connection initializing the DQMMemFile.
    ZMQHistoServerToFileOutput(unsigned int maximalUncompressedBufferSize,
                               const std::string& dqmFileName,
                               const std::string& rootFileName);

    /// Merge the given histograms into a single set of histograms and store them to file/shm
    void mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages, const std::optional<unsigned int>& experiment,
                      const std::optional<unsigned int>& run,
                      EMessageTypes messageType);
    /// Clear the shared memory
    void clear();

    /// No sockets to poll on, so return an empty list
    std::vector<zmq::socket_t*> getSockets() const final;
    /// There should be never incoming data, so raise an exception if called anyways.
    void handleIncomingData();
    /// Return the connection string
    std::string getEndPoint() const { return "file://" + m_rootFileName; }
  private:
    /// The SHM file. Please note that we do not call its destructor on purpose.
    SharedMem* m_sharedMemory = nullptr;
    /// Name of the shared memory
    std::string m_dqmMemFileName;
    /// Output file name (possible with placeholders)
    std::string m_rootFileName;
  };

  /**
   * Send the histograms as compressed byte stream via a ZMQConfirmedOutput
   * connection after merging with all the properties of a normal confirmed output.
   * Stop and terminate messages are sent as normal (after performing a last merge and sending it).
   * As additional message the event meta data is passed.
   *
   * This connection allows to built hierarchies of histogram servers with ZMQ.
   */
  class ZMQHistoServerToZMQOutput : public ZMQConnection {
  public:
    /// Initialize the ZMQConfirmedOutput with the given address
    ZMQHistoServerToZMQOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent);

    /// Merge the histograms and send them via the connection. Stop/Terminate messages are sent after that.
    void mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages, const std::optional<unsigned int>& experiment,
                      const std::optional<unsigned int>& run,
                      EMessageTypes messageType);
    /// Nothing to do on clear.
    void clear() {}

    /// The monitoring JSON is just passed from the ZMQConfirmedOutput
    std::string getMonitoringJSON() const final { return m_output.getMonitoringJSON(); }
    /// The input data handling is passed from the ZMQConfirmedOutput
    void handleIncomingData() { m_output.handleIncomingData(); }
    /// The is ready is passed from the ZMQConfirmedOutput
    bool isReady() const { return m_output.isReady(); }
    /// The sockets are passed from ZMQConfirmedOutput
    std::vector<zmq::socket_t*> getSockets() const final { return m_output.getSockets(); }
    /// Return the connection string
    std::string getEndPoint() const { return m_output.getEndPoint(); }
  private:
    /// The output connection used for sending the histograms
    ZMQConfirmedOutput m_output;

    /// Buffer used for compression
    std::vector<char> m_outputBuffer;
    /// Maximal size of the compression buffer
    unsigned int m_maximalCompressedSize = 100'000'000;
  };

  /**
   * Same as ZMQHistoServerToZMQOutput just send
   * uncompressed to a raw output.
   * Can be used to directly send to the legacy HistoServer.
   *
   * Stop/Terminate messages are not sent.
   */
  class ZMQHistoServerToRawOutput : public ZMQConnection {
  public:
    /// Create a new raw output with the given address
    ZMQHistoServerToRawOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent);

    /// Merge the histograms and send them via the connection. Stop/Terminate messages are not sent.
    void mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages, const std::optional<unsigned int>& experiment,
                      const std::optional<unsigned int>& run,
                      EMessageTypes messageType);
    /// Nothing to do on clear
    void clear() {}

    /// The monitoring JSON is just passed from the ZMQRawOutput
    std::string getMonitoringJSON() const final { return m_output.getMonitoringJSON(); }
    /// The input data handling is passed from the ZMQRawOutput
    void handleIncomingData() { m_output.handleIncomingData(); }
    /// The is ready is passed from the ZMQRawOutput
    bool isReady() const { return m_output.isReady(); }
    /// The sockets are passed from ZMQRawOutput
    std::vector<zmq::socket_t*> getSockets() const final { return m_output.getSockets(); }
    /// Return the connection string
    std::string getEndPoint() const { return m_output.getEndPoint(); }
  private:
    /// The output connection used for sending the histograms
    ZMQRawOutput m_output;
  };
}
