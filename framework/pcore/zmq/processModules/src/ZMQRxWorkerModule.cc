/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/pcore/zmq/processModules/ZMQRxWorkerModule.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQDefinitions.h>
#include <framework/core/Environment.h>

using namespace std;
using namespace Belle2;



REG_MODULE(ZMQRxWorker);


ZMQRxWorkerModule::ZMQRxWorkerModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
  addParam("eventBufferSize", m_param_bufferSize, "Maximal number of events to store in the internal buffer");
  addParam("maximalWaitingTime", m_param_maximalWaitingTime, "Maximal time to wait for any message");

  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
           "set the number of processes to at least 1.",
           Environment::Instance().getNumberProcesses());
}

void ZMQRxWorkerModule::initialize()
{
  m_randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
}

void ZMQRxWorkerModule::event()
{
  try {

    if (m_firstEvent) {
      m_streamer.initialize(m_param_compressionLevel, m_param_handleMergeable);
      m_zmqClient.initialize<ZMQ_DEALER>(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, false);

      // Wait some time until input process comes up
      sleep(1);

      // Listen to stop messages
      m_zmqClient.subscribe(EMessageTypes::c_terminateMessage);

      // General hello
      auto multicastHelloMsg = ZMQMessageFactory::createMessage(EMessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(std::move(multicastHelloMsg));
      // Hello for input process. TODO: merge this
      auto helloMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(std::move(helloMessage));

      bool inputProcessIsGone = false;
      // The following as actually not needed, as we already know at this stage that the input process is up.
      // But in some cases, the input process is already down again (because it was so fast), so will never receive any event...
      const auto socketHelloAnswer = [&inputProcessIsGone](const auto & socket) {
        const auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
        if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
          inputProcessIsGone = true;
          return false;
        }
        B2ASSERT("Received unexpected message from input.", message->isMessage(EMessageTypes::c_helloMessage));
        return false;
      };
      //      const auto pollResult = m_zmqClient.pollSocket(7200 * 1000, socketHelloAnswer);
      const auto pollResult = m_zmqClient.pollSocket(Environment::Instance().getZMQMaximalWaitingTime(), socketHelloAnswer);
      if (inputProcessIsGone or not pollResult) {
        B2WARNING("It seems the input process is already gone.");
        return;
      }
      //      B2INFO ( "ZMQRxWorker :: hello message received, sending back ready message for buffers" << m_param_bufferSize );

      // send ready msg x buffer size
      for (unsigned int bufferIndex = 0; bufferIndex < m_param_bufferSize; bufferIndex++) {
        auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
        m_zmqClient.send(std::move(readyMessage));
      }
      m_firstEvent = false;
      //      B2INFO ( "ZMQRxWorker :: Connection established after sending reply" );
    }

    const auto multicastAnswer = [](const auto & socket) {
      const auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(30, "Having received an graceful stop message. Will now go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      }

      B2ERROR("Undefined message on multicast");
      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_eventMessage)) {
        B2DEBUG(30, "received event message... write it to data store");
        //  B2INFO ( "ZMQRxWorker : event received" );
        m_streamer.read(std::move(message));
        //  if ( m_eventMetaData->getExperiment() == 42 && m_eventMetaData->getRun() == 8 )
        if (Environment::Instance().isZMQDAQFirstEvent(m_eventMetaData->getExperiment(), m_eventMetaData->getRun()))
          B2INFO("ZMQRxWorker : special event generated by HLTZMQ2Ds received.");
        auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
        m_zmqClient.send(std::move(readyMessage));
        return false;
      } else if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
        B2DEBUG(30, "received end message from input");
        return false;
      }

      B2DEBUG(30, "received unexpected message from input");
      return true;
    };

    //    const int pollReply = m_zmqClient.poll(m_param_maximalWaitingTime, multicastAnswer, socketAnswer);
    //    B2INFO ( "ZMQRxWorker : polliing started" );
    //    const int pollReply = m_zmqClient.poll(7200 * 1000, multicastAnswer, socketAnswer);
    const int pollReply = m_zmqClient.poll(Environment::Instance().getZMQMaximalWaitingTime(), multicastAnswer, socketAnswer);
    B2ASSERT("The input process did not send any event in some time!", pollReply);

    //    B2INFO ( "ZMQRxWorker : event received and moved to data store" );
    //    B2INFO ( " ---- exp = " << m_eventMetaData->getExperiment() << "  run = " << m_eventMetaData->getRun() );

    B2DEBUG(30, "Finished with event");
    //    B2INFO ( "ZMQRxWorker :: Finished with the event" );
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Rx worker event: " << ex.what());
    }
  }
}

void ZMQRxWorkerModule::terminate()
{
  m_zmqClient.terminate();
}
