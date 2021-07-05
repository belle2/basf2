/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/connections/ZMQHistogramConnection.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/logging/Logger.h>

#include <TH1.h>
#include <TBufferJSON.h>

#include <lz4.h>

#include <boost/range/combine.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace Belle2;

ZMQHistoServerToFileOutput::ZMQHistoServerToFileOutput(unsigned int maximalUncompressedBufferSize,
                                                       const std::string& dqmFileName,
                                                       const std::string& rootFileName) :
  m_dqmMemFileName(dqmFileName), m_rootFileName(rootFileName)
{

  // We do not free this on purpose!
  if (not m_dqmMemFileName.empty()) {
    m_sharedMemory = new SharedMem(m_dqmMemFileName.c_str(), maximalUncompressedBufferSize);
  }

  log("histogram_merges", 0l);
  log("last_merged_histograms", 0l);
  log("average_merged_histograms", 0l);
  log("last_merge", "");
  log("last_written_file_name", "");
  log("memory_file_size", 0l);
}

void ZMQHistoServerToFileOutput::mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages,
                                              const std::optional<unsigned int>& experiment,
                                              const std::optional<unsigned int>& run, EMessageTypes messageType)
{
  if (storedMessages.empty()) {
    return;
  }

  B2ASSERT("Experiment and run must be set at this stage", experiment and run);

  increment("histogram_merges");

  // We do not care if this is the run end, or run start or anything. We just write it out.
  HistogramMapping mergeHistograms;

  TMemFile memFile(m_dqmMemFileName.c_str(), "RECREATE");
  memFile.cd();

  log("last_merged_histograms", static_cast<long>(storedMessages.size()));
  average("average_merged_histograms", static_cast<double>(storedMessages.size()));

  logTime("last_merge");
  for (const auto& keyValue : storedMessages) {
    const auto& histogram = keyValue.second;
    mergeHistograms += histogram;
  }

  memFile.Write();

  average("memory_file_size", memFile.GetSize());

  if (m_sharedMemory) {
    m_sharedMemory->lock();
    B2ASSERT("Writing to shared memory failed!",
             memFile.CopyTo(m_sharedMemory->ptr(), memFile.GetSize()) == memFile.GetSize());
    m_sharedMemory->unlock();
  }

  // Also write the memory content out to a regular ROOT file
  auto outputFileName = boost::replace_all_copy(m_rootFileName, "{run_number}", (boost::format("%05d") % *run).str());
  boost::replace_all(outputFileName, "{experiment_number}", (boost::format("%04d") % *experiment).str());
  memFile.Cp(outputFileName.c_str(), false);

  log("last_written_file_name", outputFileName);

  mergeHistograms.clear();
}

void ZMQHistoServerToFileOutput::handleIncomingData()
{
  B2FATAL("There should be no data coming here!");
}

std::vector<zmq::socket_t*> ZMQHistoServerToFileOutput::getSockets() const
{
  return {};
}

void ZMQHistoServerToFileOutput::clear()
{
  // Clear the shared memory by writing an empty ROOT file into it
  if (m_sharedMemory) {
    TMemFile memFile(m_dqmMemFileName.c_str(), "RECREATE");
    memFile.Close();
    m_sharedMemory->lock();
    B2ASSERT("Writing to shared memory failed!",
             memFile.CopyTo(m_sharedMemory->ptr(), memFile.GetSize()) == memFile.GetSize());
    m_sharedMemory->unlock();
  }
}

ZMQHistoServerToZMQOutput::ZMQHistoServerToZMQOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent) :
  m_output(outputAddress, parent)
{
  m_output.log("histogram_merges", 0l);
  m_output.log("last_merged_histograms", 0l);
  m_output.log("average_merged_histograms", 0l);
  m_output.log("last_merge", "");
  m_output.log("size_before_compression", 0.0);
  m_output.log("size_after_compression", 0.0);
}

void ZMQHistoServerToZMQOutput::mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages,
                                             const std::optional<unsigned int>& experiment,
                                             const std::optional<unsigned int>& run, EMessageTypes messageType)
{
  if (messageType == EMessageTypes::c_lastEventMessage) {
    // merge one last time
    mergeAndSend(storedMessages, experiment, run, EMessageTypes::c_eventMessage);
    // and then send out a stop signal by ourself
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage);
    m_output.handleEvent(std::move(message));
    return;
  } else if (messageType == EMessageTypes::c_terminateMessage) {
    // merge one last time
    mergeAndSend(storedMessages, experiment, run, EMessageTypes::c_eventMessage);
    // and send out a terminate message
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_terminateMessage);
    m_output.handleEvent(std::move(message));
    return;
  }

  B2ASSERT("This should be an event message!", messageType == EMessageTypes::c_eventMessage);

  // Makes no sense to send out an empty event
  if (storedMessages.empty()) {
    return;
  }

  B2ASSERT("Experiment and run must be set at this stage", experiment and run);

  m_output.increment("histogram_merges");

  HistogramMapping mergeHistograms;

  m_output.log("last_merged_histograms", static_cast<long>(storedMessages.size()));
  m_output.average("average_merged_histograms", static_cast<double>(storedMessages.size()));
  m_output.logTime("last_merge");

  for (const auto& keyValue : storedMessages) {
    const auto& histogram = keyValue.second;
    mergeHistograms += histogram;
  }

  auto eventMessage = mergeHistograms.toMessage();

  if (m_outputBuffer.empty()) {
    m_outputBuffer.resize(m_maximalCompressedSize, 0);
  }

  m_output.average("size_before_compression", eventMessage->size());
  int size = m_maximalCompressedSize;
  size = LZ4_compress_default(eventMessage->buffer(), &m_outputBuffer[0], eventMessage->size(), size);
  B2ASSERT("Compression failed", size > 0);
  m_output.average("size_after_compression", size);

  zmq::message_t message(&m_outputBuffer[0], size);

  EventMetaData eventMetaData(0, *run, *experiment);
  auto eventInformationString = TBufferJSON::ToJSON(&eventMetaData);
  zmq::message_t additionalEventMessage(eventInformationString.Data(), eventInformationString.Length());

  auto zmqMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_compressedDataMessage, std::move(message),
                                                     std::move(additionalEventMessage));
  m_output.handleEvent(std::move(zmqMessage), true, 20000);
}

ZMQHistoServerToRawOutput::ZMQHistoServerToRawOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent) :
  m_output(outputAddress, false, parent)
{
  m_output.log("histogram_merges", 0l);
  m_output.log("last_merged_histograms", 0l);
  m_output.log("average_merged_histograms", 0l);
  m_output.log("last_merge", "");
  m_output.log("size_before_compression", 0.0);
}

void ZMQHistoServerToRawOutput::mergeAndSend(const std::map<std::string, HistogramMapping>& storedMessages,
                                             const std::optional<unsigned int>& experiment,
                                             const std::optional<unsigned int>& run, EMessageTypes messageType)
{
  if (messageType == EMessageTypes::c_lastEventMessage) {
    // merge one last time
    mergeAndSend(storedMessages, experiment, run, EMessageTypes::c_eventMessage);
    // but do not send out any message
    return;
  } else if (messageType == EMessageTypes::c_terminateMessage) {
    // merge one last time
    mergeAndSend(storedMessages, experiment, run, EMessageTypes::c_eventMessage);
    return;
  }

  B2ASSERT("This should be an event message!", messageType == EMessageTypes::c_eventMessage);

  // Makes no sense to send out an empty event
  if (storedMessages.empty()) {
    return;
  }

  B2ASSERT("Experiment and run must be set at this stage", experiment and run);

  m_output.increment("histogram_merges");

  HistogramMapping mergeHistograms;

  m_output.log("last_merged_histograms", static_cast<long>(storedMessages.size()));
  m_output.average("average_merged_histograms", static_cast<double>(storedMessages.size()));
  m_output.logTime("last_merge");

  for (const auto& keyValue : storedMessages) {
    const auto& histogram = keyValue.second;
    mergeHistograms += histogram;
  }

  auto eventMessage = mergeHistograms.toMessage();

  m_output.average("size_before_compression", eventMessage->size());

  zmq::message_t message(eventMessage->buffer(), eventMessage->size());
  m_output.handleEvent(std::move(message));
}
