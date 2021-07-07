/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <daq/hbasf2/connections/ZMQHistogramConnection.h>
#include <daq/hbasf2/utils/HistogramMapping.h>

#include <framework/pcore/zmq/messages/ZMQIdMessage.h>

#include <framework/pcore/EvtMessage.h>
#include <daq/dqm/DqmMemFile.h>

#include <TBufferJSON.h>

#include <lz4.h>
#include <TH1.h>

#include <zmq.hpp>

#include <string>
#include <memory>
#include <map>
#include <optional>

namespace Belle2 {
  /**
   * Add the common functionality to the histogram output classes.
   * This histogram output connection itself does not know how to merge or send
   * histograms, everything is implemented in the template class AConnectionClass.
   *
   * This class just adds the common code on top of those to prevent code
   * duplication. The mergeAndSend calls and the clear call are directly
   * passed on to the parent class.
   * On a message, different things happen depending on the message type:
   * * on stop or terminate messages one last mergeAndSend is called from the base class.
   *   The base class needs to handle any needed sending etc.
   * * If a compressed data message is retrieved it is decompressed.
   * * After that (or for uncompressed messages) the received histograms in the message
   *   are stored in the message map with the identity as key. In this way only ever the latest
   *   message for each sender is stored (and used when merging).
   *
   * Data messages are supposed to have the run and experiment number stored as JSON-transformed
   * EventMetaData in the additional messages. This sent event meta data is compared with the
   * already received data. On mismatch, all data is cleared.
   */
  template<class AConnectionClass>
  class ZMQHistogramOutput : public AConnectionClass {
  public:
    /// Perfectly forward the given arguments to the base class initializer (and init the buffer size)
    template<class... Args>
    ZMQHistogramOutput(unsigned int maximalUncompressedBufferSize, Args&& ... args);

    /// Handle a new message to be "sent" (what this means is up to the base class) as described above
    void handleEvent(std::unique_ptr<ZMQIdMessage> message);
    /// Forward a merge call to the base class handing over the stored messages
    void mergeAndSend(EMessageTypes messageType = EMessageTypes::c_eventMessage);
    /// Forward a clear call to the base class and clear the stored messages. Should be called on run start
    void clear();

  private:
    /// Paramter for the buffer size (needed during decompression)
    unsigned int m_maximalUncompressedBufferSize;

    /// The stored histograms for each sender identity
    std::map<std::string, HistogramMapping> m_storedMessages;
    /// The buffer used during decompression
    std::vector<char> m_uncompressedBuffer;

    /// If already received: the experiment number of the data (on mismatch, everything is cleared)
    std::optional<unsigned int> m_storedExperiment = {};
    /// If already received: the run number of the data (on mismatch, everything is cleared)
    std::optional<unsigned int> m_storedRun = {};
  };

  template<class AConnectionClass>
  template<class... Args>
  ZMQHistogramOutput<AConnectionClass>::ZMQHistogramOutput(unsigned int maximalUncompressedBufferSize,
                                                           Args&& ... args) : AConnectionClass(std::forward<Args>(args)...), m_maximalUncompressedBufferSize(
                                                               maximalUncompressedBufferSize)
  {
    m_uncompressedBuffer.reserve(m_maximalUncompressedBufferSize);

    AConnectionClass::log("uncompressed_size", 0.0);
    AConnectionClass::log("stored_identities", 0l);
    AConnectionClass::log("histogram_clears", 0l);
    AConnectionClass::log("last_clear", "");
  }

  using ZMQHistoServerToFile = ZMQHistogramOutput<ZMQHistoServerToFileOutput>;
  using ZMQHistoServerToZMQ = ZMQHistogramOutput<ZMQHistoServerToZMQOutput>;
  using ZMQHistoServerToRaw = ZMQHistogramOutput<ZMQHistoServerToRawOutput>;
}