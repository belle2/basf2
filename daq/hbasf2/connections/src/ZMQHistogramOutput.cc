/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/connections/ZMQHistogramOutput.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/pcore/EvtMessage.h>
#include <daq/dqm/DqmMemFile.h>
#include <framework/logging/Logger.h>

#include <THashList.h>
#include <TBufferJSON.h>

#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <lz4.h>
#include <zmq.hpp>

using namespace Belle2;

template<class AConnectionClass>
void ZMQHistogramOutput<AConnectionClass>::handleEvent(std::unique_ptr<ZMQIdMessage> message)
{
  if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
    // This run is over, so merge everything a final time and send it out
    mergeAndSend(EMessageTypes::c_lastEventMessage);
    clear();
    return;
  } else if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    // Everything is over, so send out a terminate message
    mergeAndSend(EMessageTypes::c_terminateMessage);
    clear();
    return;
  }

  auto identity = message->getIdentity();
  auto& dataMessage = message->getDataMessage();
  auto& additionalMessage = message->getAdditionalDataMessage();

  // Check if we get messages with the same event number
  EventMetaData* eventMetaDataPtr = nullptr;
  std::string additionalMessageAsString(additionalMessage.data<const char>(), additionalMessage.size());
  TBufferJSON::FromJSON(eventMetaDataPtr, additionalMessageAsString.c_str());

  if (m_storedExperiment and * m_storedExperiment != eventMetaDataPtr->getExperiment()) {
    B2ERROR("Having received histograms with different experiment numbers! Not a good sign!");
    AConnectionClass::increment("different_event_meta_data");
    clear();
    return;
  }
  if (m_storedRun and * m_storedRun != eventMetaDataPtr->getRun()) {
    B2ERROR("Having received histograms with different run numbers! Not a good sign!");
    AConnectionClass::increment("different_event_meta_data");
    clear();
    return;
  }

  m_storedExperiment = eventMetaDataPtr->getExperiment();
  m_storedRun = eventMetaDataPtr->getRun();

  if (message->isMessage(Belle2::EMessageTypes::c_compressedDataMessage)) {
    int uncompressedSize = LZ4_decompress_safe(dataMessage.data<char>(), &m_uncompressedBuffer[0],
                                               dataMessage.size(), m_maximalUncompressedBufferSize);
    B2ASSERT("Decompression failed", uncompressedSize > 0);

    std::unique_ptr<Belle2::EvtMessage> msg(new Belle2::EvtMessage(&m_uncompressedBuffer[0]));

    AConnectionClass::average("uncompressed_size", msg->size());

    B2DEBUG(10,
            "After decompression, the size is " << uncompressedSize << " and the message itself says " << msg->size());
    HistogramMapping histogram(std::move(msg));
    if (not histogram.empty()) {
      m_storedMessages[identity] = std::move(histogram);
    }
  } else if (message->isMessage(Belle2::EMessageTypes::c_rawDataMessage)) {
    std::unique_ptr<Belle2::EvtMessage> msg(new Belle2::EvtMessage(dataMessage.data<char>()));
    HistogramMapping histogram(std::move(msg));
    if (not histogram.empty()) {
      m_storedMessages[identity] = std::move(histogram);
    }
  } else {
    B2FATAL("Unknown message type!");
  }

  AConnectionClass::log("stored_identities", static_cast<long>(m_storedMessages.size()));
}

template<class AConnectionClass>
void ZMQHistogramOutput<AConnectionClass>::mergeAndSend(EMessageTypes messageType)
{
  AConnectionClass::mergeAndSend(m_storedMessages, m_storedExperiment, m_storedRun, messageType);
}

template<class AConnectionClass>
void ZMQHistogramOutput<AConnectionClass>::clear()
{
  AConnectionClass::increment("histogram_clears");
  AConnectionClass::logTime("last_clear");

  AConnectionClass::clear();

  m_storedMessages.clear();
  m_storedExperiment.reset();
  m_storedRun.reset();
}

template class Belle2::ZMQHistogramOutput<ZMQHistoServerToFileOutput>;
template class Belle2::ZMQHistogramOutput<ZMQHistoServerToZMQOutput>;
template class Belle2::ZMQHistogramOutput<ZMQHistoServerToRawOutput>;