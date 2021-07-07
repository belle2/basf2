/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/apps/ZMQClasses.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

void ZMQReadySender::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQLoadBalancedInput(m_inputAddress, m_bufferSize, m_parent));
  m_output.reset(new ZMQNullConnection);
}

void ZMQReadySender::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("bufferSize", boost::program_options::value<unsigned int>(&m_bufferSize)->default_value(m_bufferSize),
   "how many events to keep in the buffer");
}

void ZMQAcceptor::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQConfirmedInput(m_inputAddress, m_parent));
  m_output.reset(new ZMQNullConnection);
}

void ZMQAcceptor::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from");
}

void ZMQWorker::initialize()
{
  ZMQStandardApp::initialize();
  m_input.reset(new ZMQLoadBalancedInput(m_inputAddress, m_bufferSize, m_parent));
  m_output.reset(new ZMQConfirmedOutput(m_outputAddress, m_parent));
}

void ZMQWorker::addOptions(po::options_description& desc)
{
  ZMQStandardApp::addOptions(desc);
  desc.add_options()
  ("input", boost::program_options::value<std::string>(&m_inputAddress)->required(),
   "where to read the events from")
  ("output", boost::program_options::value<std::string>(&m_outputAddress)->required(),
   "where to send the events to")
  ("bufferSize", boost::program_options::value<unsigned int>(&m_bufferSize)->default_value(m_bufferSize),
   "how many events to keep in the buffer");
}

void ZMQWorker::handleInput()
{
  auto message = m_input->handleIncomingData();
  B2ASSERT("There should always be a message", message);

  if (message->isMessage(EMessageTypes::c_terminateMessage)) {
    m_terminate = true;
  }

  m_output->handleEvent(std::move(message));
}
