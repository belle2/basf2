/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQDistributor.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

using namespace Belle2;

void ZMQDistributor::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("expressRecoMode", boost::program_options::bool_switch(&m_expressRecoMode)->default_value(m_expressRecoMode),
   "express reco mode: dismiss events if no worker is ready and send out event messages (instead of raw messages)")
  ("maximalBufferSize",
   boost::program_options::value<unsigned int>(&m_maximalBufferSize)->default_value(m_maximalBufferSize),
   "size of the input buffer")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQDistributor::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQRawInput(m_inputAddress, m_maximalBufferSize, m_expressRecoMode, m_parent));
  m_output.reset(new ZMQLoadBalancedOutput(m_outputAddress, m_expressRecoMode, m_parent));
}

void ZMQDistributor::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    m_output->clear();
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  } else if (type == EMessageTypes::c_terminateMessage) {
    m_output->handleEvent(ZMQMessageFactory::createMessage(EMessageTypes::c_terminateMessage));
    m_input->clear();
    m_terminate = true;
  }
}

void ZMQDistributor::handleTimeout()
{
  m_output->handleEvent(ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage));

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}

void ZMQDistributor::handleInput()
{
  auto messages = m_input->handleIncomingData();

  for (auto && message : messages) {
    // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
    resetTimer();

    EMessageTypes messageType = EMessageTypes::c_rawDataMessage;
    if (m_expressRecoMode) {
      messageType = EMessageTypes::c_eventMessage;
    }

    auto outputMessage = ZMQMessageFactory::createMessage(messageType, std::move(message));

    // We know that the output is ready for the first message, but we do not know anything about any other messages, so lets
    // be safe and poll the output if it is not ready so far
    while (not m_output->isReady() and not terminated()) {
      pollEvent(false);
    }
    if (terminated()) {
      return;
    }
    m_output->handleEvent(std::move(outputMessage));
  }
}

void ZMQInputAdapter::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("expressRecoMode", boost::program_options::bool_switch(&m_expressRecoMode)->default_value(m_expressRecoMode),
   "express reco mode: dismiss events if no worker is ready and send out event messages (instead of raw messages)")
  ("maximalBufferSize",
   boost::program_options::value<unsigned int>(&m_maximalBufferSize)->default_value(m_maximalBufferSize),
   "size of the input buffer")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQInputAdapter::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQRawInput(m_inputAddress, m_maximalBufferSize, m_expressRecoMode, m_parent));
  m_output.reset(new ZMQConfirmedOutput(m_outputAddress, m_parent));

  m_monitorHasPriority = true;
}

void ZMQInputAdapter::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  } else if (type == EMessageTypes::c_terminateMessage) {
    m_output->handleEvent(ZMQMessageFactory::createMessage(EMessageTypes::c_terminateMessage));
    m_input->clear();
    m_terminate = true;
  }
}

void ZMQInputAdapter::handleTimeout()
{
  m_output->handleEvent(ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage));

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}

void ZMQInputAdapter::handleInput()
{
  auto messages = m_input->handleIncomingData();

  for (auto && message : messages) {
    // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
    resetTimer();

    EMessageTypes messageType = EMessageTypes::c_rawDataMessage;
    if (m_expressRecoMode) {
      messageType = EMessageTypes::c_eventMessage;
    }

    auto outputMessage = ZMQMessageFactory::createMessage(messageType, std::move(message));

    // We know that the output is ready for the first message, but we do not know anything about any other messages, so lets
    // be safe and poll the output if it is not ready so far
    while (not m_output->isReady() and not terminated()) {
      pollEvent(false);
    }
    if (terminated()) {
      return;
    }
    m_output->handleEvent(std::move(outputMessage));
  }
}