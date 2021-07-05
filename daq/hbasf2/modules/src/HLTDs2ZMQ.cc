/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/modules/HLTDs2ZMQ.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HLTDs2ZMQ)

HLTDs2ZMQModule::HLTDs2ZMQModule() : Module()
{
  setDescription(
    "On every event, serialize the data store and send the binary data out to "
    "the connected ZMQ application (most likely a collector or final collector). "
    "The sending is handled via a confirmed connection (output in this case), "
    "so all the typical behaviour applies. Also sends out end run and termination "
    "messages. Depending on the module setting, can send out events in  "
    "raw format (with send header and trailer and a serialized event message as buffer)  "
    "or only as normal ROOT serialized stream (evt message).  "
    "The former is the typical use case when talking with e.g. storage, the  "
    "latter can be used for local tests or when sending full events e.g. to the event display. "
    "Please note that the environment setting of the stream objects heavily "
    "influences the time spent in this module (because serialization needs time). "
    "This module is only useful in the HLT context or for testing it and it optimized to be used "
    "together with the HLTEventProcessor. Please note the special handling of the first event in the "
    "HLTEventProcessor (therefore we do not stream the first event)"
  );
  setPropertyFlags(EModulePropFlags::c_Output | EModulePropFlags::c_ParallelProcessingCertified);

  addParam("output", m_param_output, "The ZMQ address of the connected application (to receive the messages).");
  addParam("raw", m_param_raw, "Send out raw data with send header and trailer around the evtmessage instead of just the evtmessage. "
           "The former is the typical use case when talking with e.g. storage, "
           "the latter can be used for local tests or when sending full events e.g. to the event display.");
}

void HLTDs2ZMQModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamHelper.initialize();
      m_parent.reset(new ZMQParent);
      m_output.reset(new ZMQConfirmedOutput(m_param_output, m_parent));

      m_firstEvent = false;
      return;
    }

    if (m_param_raw) {
      auto zmqMessage = m_streamHelper.streamRaw();
      m_output->handleEvent(std::move(zmqMessage));
    } else {
      auto zmqMessage = m_streamHelper.stream(false, false);
      m_output->handleEvent(std::move(zmqMessage));
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

void HLTDs2ZMQModule::endRun()
{
  try {
    B2DEBUG(10, "Sending out old run message");
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage);
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

void HLTDs2ZMQModule::terminate()
{
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
