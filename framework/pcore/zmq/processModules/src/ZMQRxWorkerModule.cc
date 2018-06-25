/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/zmq/processModules/ZMQRxWorkerModule.h>

#include <framework/pcore/ProcHandler.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxWorker)

ZMQRxWorkerModule::ZMQRxWorkerModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
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
      m_zmqClient.subscribe(c_MessageTypes::c_stopMessage);

      // General hello
      const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(multicastHelloMsg);
      // Hello for input process. TODO: merge this
      const auto& helloMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_whelloMessage, getpid());
      m_zmqClient.publish(helloMessage);

      // send ready msg x buffer size
      for (unsigned int bufferIndex = 0; bufferIndex < m_bufferSize; bufferIndex++) {
        const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
        m_zmqClient.send(readyMessage);
      }
      m_firstEvent = false;
    }

    const auto multicastAnswer = [](const auto & socket) {
      const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(c_MessageTypes::c_stopMessage)) {
        B2DEBUG(100, "Having received an graceful stop message. Will now go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      }

      B2ERROR("Undefined message on multicast");
      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(c_MessageTypes::c_eventMessage)) {
        B2DEBUG(100, "received event message... write it to data store");
        m_streamer.read(message, m_randomgenerator);
        const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
        m_zmqClient.send(readyMessage);
        return false;
      } else if (message->isMessage(c_MessageTypes::c_endMessage)) {
        B2DEBUG(100, "received end message from input");
        return false;
      }

      B2DEBUG(100, "received unexpected message from input");
      return true;
    };

    const int pollReply = m_zmqClient.poll(20 * 1000, multicastAnswer, socketAnswer);
    B2ASSERT("The input process did not send any event in some time!", pollReply);

    B2DEBUG(100, "Finished with event");
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