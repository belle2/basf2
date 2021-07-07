/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/modules/HLTZMQ2Ds.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HLTZMQ2Ds)

HLTZMQ2DsModule::HLTZMQ2DsModule() : Module()
{
  setDescription(
    "Input module in the ZMQ reconstruction path receiving events via ZMQ "
    "and deserializing the to the data store. The connection to the previous ZMQ application "
    "(most likely a distributor or collector) is handled via a load balanced connection "
    "(input in this case). The buffer size for the load balanced connection can be  "
    "controlled via a module parameter. "
    "This module only works in the context of the HLT when using the HLTEventProcessor, "
    "due to the special form the first event as well as beginRun and endRun are handled. "
    "Please read the overall description in the HLTEventProcessor for an overview. "
    "Before the first real event is received (which is the first time the event function "
    "is called by the HLTEventProcessor, but before the forking), the "
    "event meta data is initialized with a predefined experiment and run number (set via "
    "module parameters) so make module initialization in all other modules possible. "
    "However, no event function should be called for other modules in this event "
    "(as the data store is invalid). In the first real event after the forking, "
    "the connection and streamer is initialized. Then, normal event messages "
    "are deserialized and written to data store. End run or terminate messages are  "
    "handled by setting a special flag of the EventMetaData. Also in this case  "
    "the remaining modules should not process this event via an event function "
    "(assured by the HLTEventProcessor)."
  );
  setPropertyFlags(EModulePropFlags::c_Input | EModulePropFlags::c_ParallelProcessingCertified);

  addParam("input", m_param_input, "ZMQ address of the input ZMQ application");
  addParam("addExpressRecoObjects", m_param_addExpressRecoObjects,
           "Additional to the raw data, also register the data store objects needed for express reco. TODO: this might change",
           m_param_addExpressRecoObjects);
  addParam("bufferSize", m_param_bufferSize,
           "How many events should be kept in flight. Has an impact on the stopping time as well as the rate stability", m_param_bufferSize);

  addParam("defaultExperiment", m_lastExperiment,
           "Default experiment number to be set during initialization/run end to have something to load the geometry.", m_lastExperiment);
  addParam("defaultRun", m_lastRun,
           "Default run number to be set during initialization/run end to have something to load the geometry.", m_lastRun);
}

void HLTZMQ2DsModule::initialize()
{
  m_streamHelper.registerStoreObjects(m_param_addExpressRecoObjects);
}

void HLTZMQ2DsModule::event()
{
  setReturnValue(0);

  // The very first event is actually not the first event for processing.
  // It is just used to initialize the geometry, so we write out
  // a default event and return immediately. This will cause
  // all subsequent modules to be initialized.
  if (m_inInitialize) {
    m_inInitialize = false;

    m_eventMetaData.create();
    m_eventMetaData->setExperiment(m_lastExperiment);
    m_eventMetaData->setRun(m_lastRun);

    setReturnValue(1);
    return;
  }

  try {
    // If we are not in this initialization step, we can do the normal event processing
    // This becomes now the first "real" event
    if (m_firstEvent) {
      m_streamHelper.initialize();

      m_parent = std::make_unique<ZMQParent>();
      m_input = std::make_unique<ZMQLoadBalancedInput>(m_param_input, m_param_bufferSize, m_parent);

      m_firstEvent = false;
    }

    const auto reactToInput = [this]() {
      auto eventMessage = m_input->handleIncomingData();

      if (eventMessage->isMessage(EMessageTypes::c_lastEventMessage)) {
        B2DEBUG(10, "Received run change request");

        m_eventMetaData.create();
        m_eventMetaData->setEndOfRun(m_lastExperiment, m_lastRun);
        return;
      } else if (eventMessage->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(10, "Received termination request");

        m_eventMetaData.create();
        m_eventMetaData->setEndOfData();
        return;
      }

      B2ASSERT("Must be event message", eventMessage->isMessage(EMessageTypes::c_eventMessage) or
               eventMessage->isMessage(EMessageTypes::c_rawDataMessage));
      B2DEBUG(10, "received event message... write it to data store");

      m_streamHelper.read(std::move(eventMessage));

      B2ASSERT("There is still no event meta data present!", m_eventMetaData);
      m_lastRun = m_eventMetaData->getRun();
      m_lastExperiment = m_eventMetaData->getExperiment();
    };

    bool result = ZMQConnection::poll({{m_input.get(), reactToInput}}, -1);
    if (!result) {
      // didn't get any events, probably interrupted by a signal.
      // We're the input module so let's better have some event meta data
      // even if it's not useful
      m_eventMetaData.create();
      m_eventMetaData->setEndOfData();
    }
  } catch (zmq::error_t& error) {
    // This is an unexpected error: better report it.
    B2ERROR("ZMQ Error while calling the event: " << error.num());
  }
}
