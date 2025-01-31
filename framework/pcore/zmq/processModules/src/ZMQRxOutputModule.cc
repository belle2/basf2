/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/core/Environment.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput);

ZMQRxOutputModule::ZMQRxOutputModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
  addParam("maximalWaitingTime", m_param_maximalWaitingTime, "Maximal time to wait for any message");
  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
           "set the number of processes to at least 1.",
           Environment::Instance().getNumberProcesses());
}

void ZMQRxOutputModule::initialize()
{
  m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
}

void ZMQRxOutputModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamer.initialize(m_param_compressionLevel, m_param_handleMergeable);
      m_zmqClient.initialize<ZMQ_PULL>(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, true);

      auto multicastHelloMsg = ZMQMessageFactory::createMessage(EMessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(std::move(multicastHelloMsg));

      // Listen to event backups, the stop message of the input process and the general stop messages
      m_zmqClient.subscribe(EMessageTypes::c_eventMessage);
      m_zmqClient.subscribe(EMessageTypes::c_lastEventMessage);
      m_zmqClient.subscribe(EMessageTypes::c_terminateMessage);
      m_firstEvent = false;

      m_beginRun = Environment::Instance().getNumberProcesses();
      m_endRun = Environment::Instance().getNumberProcesses();
    }

    const auto multicastAnswer = [this](const auto & socket) {
      auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_eventMessage)) {
        B2DEBUG(100, "Having received an event backup. Will go in with this.");
        m_streamer.read(std::move(message));
        StoreObjPtr<EventMetaData> eventMetaData;
        eventMetaData->addErrorFlag(EventMetaData::EventErrorFlag::c_HLTCrash);
        return false;
      } else if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
        B2DEBUG(100, "Having received an end message. Will not go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      } else if (message->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(100, "Having received an graceful stop message. Will not go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      }

      B2ERROR("Undefined message on multicast");
      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_eventMessage)) {
        m_streamer.read(std::move(message));
        B2DEBUG(100, "received event " << m_eventMetaData->getEvent());
        auto confirmMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_confirmMessage, m_eventMetaData);
        m_zmqClient.publish(std::move(confirmMessage));
        // Check EventMetaData and repeat poll until all end run records received
        if (m_eventMetaData->isEndOfRun()) {
          m_endRun--;
          if (m_endRun == 0) {
            m_endRun = Environment::Instance().getNumberProcesses();
            B2INFO("ZMQOutputModule : sending out EndRun record.");
            return false;
          } else {
            return true;
          }
        } else if (Environment::Instance().isZMQDAQFirstEvent(m_eventMetaData->getExperiment(),
                                                              m_eventMetaData->getRun())) {    // Special first event
          m_beginRun--;
          if (m_beginRun == 0) {
            m_beginRun = Environment::Instance().getNumberProcesses();
            B2INFO("ZMQOutputModule : sending out HLTZMQ first event.");
            return false;
          } else {
            return true;
          }
        }

        return false;
      }

      B2ERROR("Undefined message on socket");
      return true;
    };


    B2DEBUG(100, "Start polling");
    //    const int pollReply = m_zmqClient.poll(m_param_maximalWaitingTime, multicastAnswer, socketAnswer);
    //    const int pollReply = m_zmqClient.poll((unsigned int)7200 * 1000, multicastAnswer, socketAnswer);
    const int pollReply = m_zmqClient.poll(Environment::Instance().getZMQMaximalWaitingTime(), multicastAnswer, socketAnswer);
    B2ASSERT("Output process did not receive any message in some time. Aborting.", pollReply);
    //    B2INFO ( "ZMQRxOutput : event received" );

    B2DEBUG(30, "finished reading in an event.");
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Rx output event: " << ex.what());
    }
  }
}

void ZMQRxOutputModule::terminate()
{
  m_zmqClient.terminate();
}
