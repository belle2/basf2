/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQCollector.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void ZMQCollector::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("lax", boost::program_options::bool_switch(&m_lax)->default_value(m_lax),
   "dismiss events if no worker is ready (lax) or not")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQCollector::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQLoadBalancedOutput(m_outputAddress, m_lax, m_parent));
}

void ZMQCollector::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    m_output->clear();
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  }
}

void ZMQCollector::handleInput()
{
  auto message = m_input->handleIncomingData();

  // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
  resetTimer();

  if (message) {
    if (message->isMessage(EMessageTypes::c_terminateMessage)) {
      m_terminate = true;
    }

    m_output->handleEvent(ZMQMessageFactory::stripIdentity(std::move(message)));
  }
}

void ZMQCollector::handleTimeout()
{
  auto message = m_input->overwriteStopMessage();
  if (message) {
    m_output->handleEvent(ZMQMessageFactory::stripIdentity(std::move(message)));
  }

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}

void ZMQOutputAdapter::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to");
}

void ZMQOutputAdapter::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQLoadBalancedInput(m_inputAddress, 1, m_parent));
  m_output.reset(new ZMQRawOutput(m_outputAddress, true, m_parent));
}

void ZMQOutputAdapter::handleExternalSignal(EMessageTypes type)
{
}

void ZMQOutputAdapter::handleInput()
{
  auto message = m_input->handleIncomingData();
  if (not message) {
    return;
  }

  if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    m_terminate = true;
  }

  if (message->isMessage(EMessageTypes::c_rawDataMessage) or message->isMessage(EMessageTypes::c_eventMessage)) {
    B2INFO(message->getDataMessage().size());
    m_output->handleEvent(std::move(message->getDataMessage()));
  }
}

void ZMQProxyCollector::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQProxyCollector::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQConfirmedOutput(m_outputAddress, m_parent));
}

void ZMQProxyCollector::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  }
}

void ZMQProxyCollector::handleInput()
{
  auto message = m_input->handleIncomingData();

  // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
  resetTimer();

  if (message) {
    if (message->isMessage(EMessageTypes::c_terminateMessage)) {
      m_terminate = true;
    }

    m_output->handleEvent(ZMQMessageFactory::stripIdentity(std::move(message)));
  }
}

void ZMQProxyCollector::handleTimeout()
{
  auto message = m_input->overwriteStopMessage();
  if (message) {
    m_output->handleEvent(ZMQMessageFactory::stripIdentity(std::move(message)));
  }

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}

void ZMQFinalCollector::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("addEventSize", boost::program_options::bool_switch(&m_addEventSize)->default_value(false),
   "add the hlon of the event size at the beginning")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQFinalCollector::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQRawOutput(m_outputAddress, m_addEventSize, m_parent));

  m_monitorHasPriority = true;
}

void ZMQFinalCollector::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    return;
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  }
}

void ZMQFinalCollector::handleInput()
{
  auto message = m_input->handleIncomingData();

  // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
  resetTimer();

  if (not message) {
    return;
  }

  if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    m_terminate = true;
  }

  if (message->isMessage(EMessageTypes::c_rawDataMessage) or message->isMessage(EMessageTypes::c_eventMessage)) {
    m_output->handleEvent(std::move(message->getDataMessage()));
  }
}

void ZMQFinalCollector::handleTimeout()
{
  m_input->overwriteStopMessage();

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}

void ZMQFinalCollectorWithROI::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_dataOutputAddress)->required(),
   "where to send the events to")
  ("roi", boost::program_options::value<std::string>(&m_roiOutputAddress)->required(),
   "where to send the rois to")
  ("addEventSize", boost::program_options::bool_switch(&m_addEventSize)->default_value(false),
   "add the hlon of the event size at the beginning")
  ("stopWaitingTime",
   boost::program_options::value<unsigned int>(&m_stopWaitingTime)->default_value(m_stopWaitingTime),
   "how long to wait after no events come anymore");
}

void ZMQFinalCollectorWithROI::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQDataAndROIOutput(m_dataOutputAddress, m_roiOutputAddress, m_addEventSize, m_parent));

  m_monitorHasPriority = true;
}

void ZMQFinalCollectorWithROI::handleInput()
{
  auto message = m_input->handleIncomingData();

  // So there has been a message, make sure to reset the timer for waiting (if no timer is set this will just return)
  resetTimer();

  if (not message) {
    return;
  }

  if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    m_terminate = true;
  }

  m_output->handleEvent(ZMQMessageFactory::stripIdentity(std::move(message)));
}

void ZMQFinalCollectorWithROI::handleExternalSignal(EMessageTypes type)
{
  if (type == EMessageTypes::c_newRunMessage) {
    m_input->clear();
    return;
  } else if (type == EMessageTypes::c_lastEventMessage) {
    m_timeout = m_stopWaitingTime;
    resetTimer();
  }
}

void ZMQFinalCollectorWithROI::fillMonitoringJSON(std::stringstream& buffer) const
{
  buffer << "{" << std::endl;
  buffer << "\"monitor\": " << m_monitor->getMonitoringJSON() << "," << std::endl;
  buffer << "\"input\": " << m_input->getMonitoringJSON() << "," << std::endl;
  buffer << "\"output\": " << m_output->getMonitoringJSON() << "," << std::endl;
  buffer << "\"roi\": " << m_output->getROIMonitoringJSON() << std::endl;
  buffer << "}" << std::endl;
}

void ZMQFinalCollectorWithROI::handleTimeout()
{
  m_input->overwriteStopMessage();

  // We do not want to send out another stop message, so reset the counter
  m_timeout = 0;
  resetTimer();
}