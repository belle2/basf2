/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/modules/HLTZMQ2DsDirect.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(HLTZMQ2DsDirect)

HLTZMQ2DsDirectModule::HLTZMQ2DsDirectModule() : Module()
{
  setDescription(
    ""
  );
  setPropertyFlags(EModulePropFlags::c_Input | EModulePropFlags::c_ParallelProcessingCertified);

  addParam("input", m_param_input, "ZMQ address of the input ZMQ application");
  addParam("bufferSize", m_param_bufferSize,
           "How many events should be kept in flight. Has an impact on the stopping time as well as the rate stability", m_param_bufferSize);
}

void HLTZMQ2DsDirectModule::readEvent()
{
  try {
    bool tryAgain = false;
    const auto reactToInput = [this, &tryAgain]() {
      auto eventMessage = m_input->handleIncomingData();

      if (eventMessage->isMessage(EMessageTypes::c_lastEventMessage)) {
        // We do not care about those messages, so just continue
        tryAgain = true;
        return;
      } else if (eventMessage->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(10, "Received termination request");
        tryAgain = false;
        return;
      }

      B2ASSERT("Must be event message", eventMessage->isMessage(EMessageTypes::c_eventMessage) or
               eventMessage->isMessage(EMessageTypes::c_rawDataMessage));
      B2DEBUG(10, "received event message... write it to data store");

      m_streamHelper.read(std::move(eventMessage));
      tryAgain = false;
    };

    do {
      ZMQConnection::poll({{m_input.get(), reactToInput}}, -1);
    } while (tryAgain);
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

void HLTZMQ2DsDirectModule::initialize()
{
  m_streamHelper.initialize();

  m_parent = std::make_unique<ZMQParent>();
  m_input = std::make_unique<ZMQLoadBalancedInput>(m_param_input, m_param_bufferSize, m_parent);

  readEvent();
}

void HLTZMQ2DsDirectModule::event()
{
  // We do not need to process this event again
  if (m_firstEvent) {
    m_firstEvent = false;
    return;
  }

  readEvent();
}
