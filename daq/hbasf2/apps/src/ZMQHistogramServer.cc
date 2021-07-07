/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQHistogramServer.h>
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>
#include <daq/hbasf2/connections/ZMQHistogramConnection.h>

using namespace Belle2;

void ZMQHistogramToFileServer::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(
    new ZMQHistoServerToFile(m_maximalUncompressedBufferSize, m_maximalUncompressedBufferSize, m_sharedMemoryName, m_rootFileName));
}

void ZMQHistogramToFileServer::addOptions(po::options_description& desc)
{
  m_timeout = 30;
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("sharedMemoryName",
   boost::program_options::value<std::string>(&m_sharedMemoryName)->default_value(m_sharedMemoryName),
   "name of the shared memory")
  ("rootFileName", boost::program_options::value<std::string>(&m_rootFileName)->required(),
   "name of the ROOT file - can include {run_number} or {experiment_number}")
  ("timeout", boost::program_options::value<unsigned int>(&m_timeout)->default_value(m_timeout),
   "how many seconds to wait between histogram dumps")
  ("maximalUncompressedBufferSize",
   boost::program_options::value<unsigned int>(&m_maximalUncompressedBufferSize)->default_value(
     m_maximalUncompressedBufferSize),
   "size of the uncompress buffer");
}

void ZMQHistogramToFileServer::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    m_output->clear();
    return;
  } else if (type == EMessageTypes::c_lastEventMessage) {
    auto message = m_input->overwriteStopMessage();
    if (message) {
      m_output->handleEvent(std::move(message));
    }
    return;
  }
}

void ZMQHistogramToFileServer::handleInput()
{
  auto message = m_input->handleIncomingData();
  if (message) {
    if (message->isMessage(EMessageTypes::c_terminateMessage)) {
      m_terminate = true;
    }

    m_output->handleEvent(std::move(message));
  }
}

void ZMQHistogramToFileServer::handleTimeout()
{
  m_output->mergeAndSend();
}

void ZMQHistogramToZMQServer::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQHistoServerToZMQ(m_maximalUncompressedBufferSize, m_outputAddress, m_parent));
}

void ZMQHistogramToZMQServer::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("timeout", boost::program_options::value<unsigned int>(&m_timeout)->default_value(30),
   "how many seconds to wait between histogram dumps")
  ("maximalUncompressedBufferSize",
   boost::program_options::value<unsigned int>(&m_maximalUncompressedBufferSize)->default_value(
     m_maximalUncompressedBufferSize),
   "size of the uncompress buffer");
}

void ZMQHistogramToZMQServer::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    m_output->clear();
    return;
  } else if (type == EMessageTypes::c_lastEventMessage) {
    auto message = m_input->overwriteStopMessage();
    if (message) {
      m_output->handleEvent(std::move(message));
    }
    return;
  }
}

void ZMQHistogramToZMQServer::handleInput()
{
  auto message = m_input->handleIncomingData();
  if (message) {
    if (message->isMessage(EMessageTypes::c_terminateMessage)) {
      m_terminate = true;
    }

    m_output->handleEvent(std::move(message));
  }
}

void ZMQHistogramToZMQServer::handleTimeout()
{
  m_output->mergeAndSend();
}

void ZMQHistogramToRawServer::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQHistoServerToRaw(m_maximalUncompressedBufferSize, m_outputAddress, m_parent));
}

void ZMQHistogramToRawServer::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("timeout", boost::program_options::value<unsigned int>(&m_timeout)->default_value(30),
   "how many seconds to wait between histogram dumps")
  ("maximalUncompressedBufferSize",
   boost::program_options::value<unsigned int>(&m_maximalUncompressedBufferSize)->default_value(
     m_maximalUncompressedBufferSize),
   "size of the uncompress buffer");
}

void ZMQHistogramToRawServer::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    m_output->clear();
    return;
  } else if (type == EMessageTypes::c_lastEventMessage) {
    auto message = m_input->overwriteStopMessage();
    if (message) {
      m_output->handleEvent(std::move(message));
    }
    return;
  }
}

void ZMQHistogramToRawServer::handleInput()
{
  auto message = m_input->handleIncomingData();
  if (message) {
    if (message->isMessage(EMessageTypes::c_terminateMessage)) {
      m_terminate = true;
    }

    m_output->handleEvent(std::move(message));
  }
}

void ZMQHistogramToRawServer::handleTimeout()
{
  m_output->mergeAndSend();
}