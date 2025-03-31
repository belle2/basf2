/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/modules/HLTDs2ZMQRaw.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HLTDs2ZMQRaw)

HLTDs2ZMQRawModule::HLTDs2ZMQRawModule() : Module()
{
  setDescription(
    "On every event, serialize the data store and send the binary data out to "
    "the connected ZMQ application (most likely a collector or final collector). "
    "The sending is handled via a raw connection (output in this case), "
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
  addParam("addEventSize", m_param_addEventSize, "add the hlon of the event size at the beginning", true);
  addParam("addPersistentDurability", m_param_addPersistentDurability, "For streamHelper", false);
  addParam("streamTransientObjects", m_param_streamTransientObjects, "For streamHelper", false);
}

void HLTDs2ZMQRawModule::initialize()
{
  if (!m_firstEvent) {
    m_streamHelper.initialize(0, true);
    m_parent.reset(new ZMQParent);
    m_output.reset(new ZMQRawOutput(m_param_output, m_param_addEventSize, m_parent));
    m_output->handleIncomingData();
  }
}

void HLTDs2ZMQRawModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamHelper.initialize(0, true);
      m_parent.reset(new ZMQParent);
      m_output.reset(new ZMQRawOutput(m_param_output, m_param_addEventSize, m_parent));
      m_output->handleIncomingData();

      m_firstEvent = false;
      return;
    }

    const auto evtMessage = m_streamHelper.stream(m_param_addPersistentDurability, m_param_streamTransientObjects);
    auto zmqMessage = m_messageHelper.createZMQMessage(evtMessage);
    m_output->handleEvent(std::move(zmqMessage));

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

