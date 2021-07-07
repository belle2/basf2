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

REG_MODULE(ZMQRxWorker)

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
      const auto pollResult = m_zmqClient.pollSocket(60 * 1000, socketHelloAnswer);
      if (inputProcessIsGone or not pollResult) {
        B2WARNING("It seems the input process is already gone.");
        return;
      }

      // send ready msg x buffer size
      for (unsigned int bufferIndex = 0; bufferIndex < m_param_bufferSize; bufferIndex++) {
        auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
        m_zmqClient.send(std::move(readyMessage));
      }
      m_firstEvent = false;
    }

    const auto multicastAnswer = [](const auto & socket) {
      const auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(10, "Having received an graceful stop message. Will now go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      }

      B2ERROR("Undefined message on multicast");
      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      auto message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_eventMessage)) {
        B2DEBUG(10, "received event message... write it to data store");
        m_streamer.read(std::move(message));
        auto readyMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_readyMessage);
        m_zmqClient.send(std::move(readyMessage));
        return false;
      } else if (message->isMessage(EMessageTypes::c_lastEventMessage)) {
        B2DEBUG(10, "received end message from input");
        return false;
      }

      B2DEBUG(10, "received unexpected message from input");
      return true;
    };

    const int pollReply = m_zmqClient.poll(m_param_maximalWaitingTime, multicastAnswer, socketAnswer);
    B2ASSERT("The input process did not send any event in some time!", pollReply);

    B2DEBUG(10, "Finished with event");
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
