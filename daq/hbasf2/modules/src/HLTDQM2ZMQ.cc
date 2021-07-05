/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/modules/HLTDQM2ZMQ.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/RbTuple.h>
#include <framework/core/HistoModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HLTDQM2ZMQ)

HLTDQM2ZMQModule::HLTDQM2ZMQModule() : Module()
{
  setDescription(
    "Module to collect DQM histograms (written out by HistoModules) and "
    "send them every time period to a running ZMQ DQM server "
    "(either a finalhistoserver or a proxyhistorver). "
    "The address as well as the send interval are module parameters. "
    "As the old DQM module, this module works by streaming everything in the current ROOT main "
    "directory, which is either a TDirectory or a TH1. For the specific implementation on how "
    "the streaming is done, please see the HLTStreamHelper class. "
    "The histogram sending is handled via a confirmed connection (output in this case), "
    "so all the usual conventions for a confirmed connection apply. "
    "This module does only makes sense to run on the HLT, it is not useful for local "
    "file writeout."
  );
  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  addParam("output", m_param_output, "ZMQ address to send the histograms to (the local histo server)");
  addParam("sendOutInterval", m_param_sendOutInterval, "Time interval in seconds to send out the histograms. "
           "Please note that the full stack of DQM histo servers"
           "could delay this, as each of them have a timeout.",
           m_param_sendOutInterval);
}

void HLTDQM2ZMQModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamHelper.initialize();
      m_parent.reset(new ZMQParent);
      m_output.reset(new ZMQConfirmedOutput(m_param_output, m_parent));
      m_start = std::chrono::system_clock::now();

      m_firstEvent = false;
    }

    auto currentTime = std::chrono::system_clock::now();
    auto timeDifference = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_start).count();
    if (timeDifference > m_param_sendOutInterval) {
      sendOutHistograms();
      m_start = std::chrono::system_clock::now();
    }
  } catch (zmq::error_t& error) {
    if (error.num() == EINTR) {
      // Well, that is probably ok. It will be handled by the framework, just go out here.
      B2DEBUG(10, "Received an signal interrupt during the event call. Will return");
      return;
    }
    // This is an unexpected error: better report it.
    B2ERROR("ZMQ Error while calling the event: " << error.num());
  }
}

void HLTDQM2ZMQModule::beginRun()
{
  if (m_histogramsDefined) {
    return;
  }
  const auto& modules = RbTupleManager::Instance().getHistDefiningModules();
  for (const auto& module : modules) {
    B2INFO(module->getName() << " is a histo module");
    auto* histoModule = dynamic_cast<HistoModule*>(module);
    B2ASSERT("The added module is not a histogram module!", histoModule);
    histoModule->defineHisto();
  }

  m_histogramsDefined = true;
}

void HLTDQM2ZMQModule::endRun()
{
  if (m_firstEvent) {
    return;
  }

  try {
    B2DEBUG(10, "Sending out old run message");
    sendOutHistograms();
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage);
    m_output->handleEvent(std::move(message), false, 1000);
  } catch (zmq::error_t& error) {
    if (error.num() == EINTR) {
      // Well, that is probably ok. It will be handled by the framework, just go out here.
      B2DEBUG(10, "Received an signal interrupt during the event call. Will return");
      return;
    }
    // This is an unexpected error: better report it.
    B2ERROR("ZMQ Error while calling the event: " << error.num());
  }

  // TODO: we need to get rid of the histograms, or?
}

void HLTDQM2ZMQModule::terminate()
{
  if (m_firstEvent) {
    return;
  }

  try {
    B2DEBUG(10, "Sending out terminate message");
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_terminateMessage);
    m_output->handleEvent(std::move(message));
  } catch (zmq::error_t& error) {
    if (error.num() == EINTR) {
      // Well, that is probably ok. It will be handled by the framework, just go out here.
      B2DEBUG(10, "Received an signal interrupt during the event call. Will return");
      return;
    }
    // This is an unexpected error: better report it.
    B2ERROR("ZMQ Error while calling the event: " << error.num());
  }
}

void HLTDQM2ZMQModule::sendOutHistograms()
{
  if (m_firstEvent) {
    return;
  }

  auto msg = m_streamHelper.streamHistograms();
  m_output->handleEvent(std::move(msg), false, 1000);
}
