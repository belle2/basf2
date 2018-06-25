#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <chrono>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput)

ZMQRxOutputModule::ZMQRxOutputModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
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
      m_zmqClient.initialize(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, true);

      const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(multicastHelloMsg);

      // Listen to event backups, the stop message of the input process and the general stop messages
      m_zmqClient.subscribe(c_MessageTypes::c_eventMessage);
      m_zmqClient.subscribe(c_MessageTypes::c_endMessage);
      m_zmqClient.subscribe(c_MessageTypes::c_stopMessage);
      m_firstEvent = false;
    }

    const auto multicastAnswer = [this](const auto & socket) {
      const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      if (message->isMessage(c_MessageTypes::c_eventMessage)) {
        B2DEBUG(100, "Having received an event backup. Will go in with this.");
        // TODO: We would set a flag here, as we have received this message from the input process
        m_streamer.read(message, m_randomgenerator);
        // TODO: do not go on
        return false;
      } else if (message->isMessage(c_MessageTypes::c_endMessage)) {
        B2DEBUG(100, "Having received an end message. Will now go on.");
        // By not storing anything in the data store, we will just stop event processing here...
        return false;
      } else if (message->isMessage(c_MessageTypes::c_stopMessage)) {
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
        m_streamer.read(message, m_randomgenerator);
        B2DEBUG(100, "received event " << m_eventMetaData->getEvent());
        const auto& confirmMessage = ZMQMessageFactory::createMessage(m_eventMetaData);
        m_zmqClient.publish(confirmMessage);
        return false;
      }

      B2ERROR("Undefined message on socket");
      return true;
    };


    B2DEBUG(100, "Start polling");
    const int pollReply = m_zmqClient.poll(100000, multicastAnswer, socketAnswer);
    B2ASSERT("Output process did not receive any message in some time. Aborting.", pollReply);

    B2DEBUG(100, "finished reading in an event.");
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
