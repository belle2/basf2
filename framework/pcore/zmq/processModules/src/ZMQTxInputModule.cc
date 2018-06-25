#include <framework/pcore/zmq/processModules/ZMQTxInputModule.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/utils/EventMetaDataSerialization.h>
#include <framework/pcore/EvtMessage.h>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQTxInput)

ZMQTxInputModule::ZMQTxInputModule() : Module()
{
  addParam("socketName", m_param_socketName, "Name of the socket to connect this module to.");
  addParam("xpubProxySocketName", m_param_xpubProxySocketName, "Address of the XPUB socket of the proxy");
  addParam("xsubProxySocketName", m_param_xsubProxySocketName, "Address of the XSUB socket of the proxy");
  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
           "set the number of processes to at least 1.",
           Environment::Instance().getNumberProcesses());
}

void ZMQTxInputModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamer.initialize(m_param_compressionLevel, m_param_handleMergeable);
      m_zmqClient.initialize<ZMQ_ROUTER>(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, true);

      const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(multicastHelloMsg);

      // Listen to event confirmations, hello of workers, the messages to delete a worker and the general stop messages
      m_zmqClient.subscribe(c_MessageTypes::c_confirmMessage);
      m_zmqClient.subscribe(c_MessageTypes::c_whelloMessage);
      m_zmqClient.subscribe(c_MessageTypes::c_deleteMessage);
      m_zmqClient.subscribe(c_MessageTypes::c_stopMessage);

      m_firstEvent = false;
    }

    int timeout = 20 * 1000;
    if (not m_nextWorker.empty()) {
      // if next worker are available do not waste time
      timeout = 0;
    }

    const auto multicastAnswer = [this](const auto & socket) {
      const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      const std::string& data = multicastMessage->getData();

      if (multicastMessage->isMessage(c_MessageTypes::c_whelloMessage)) {
        m_workers.push_back(std::stoi(data));
        B2RESULT("received c_whelloMessage from " << data << "... replying");

        const auto& replyHelloMessage = ZMQMessageFactory::createMessage(data, c_MessageTypes::c_whelloMessage);
        m_zmqClient.send(replyHelloMessage);
        return true;
      } else if (multicastMessage->isMessage(c_MessageTypes::c_confirmMessage)) {
        const auto& eventMetaData = EventMetaDataSerialization::deserialize(data);
        m_procEvtBackupList.removeEvt(eventMetaData);
        B2RESULT("removed event backup.. list size: " << m_procEvtBackupList.size());
        return true;
      } else if (multicastMessage->isMessage(c_MessageTypes::c_deleteMessage)) {
        const int workerID = std::atoi(data.c_str());
        B2RESULT("received worker delete message, workerID: " << workerID);

        m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
        m_nextWorker.erase(std::remove(m_nextWorker.begin(), m_nextWorker.end(), workerID), m_nextWorker.end());
        return true;
      } else if (multicastMessage->isMessage(c_MessageTypes::c_stopMessage)) {
        // TODO
        B2WARNING("Having received a stop message, but what should I do?");
      }

      B2ERROR("Received an unknown message");
      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      const auto& message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(socket);
      if (message->isMessage(c_MessageTypes::c_readyMessage)) {
        B2RESULT("got worker ready message");
        m_nextWorker.push_back(std::stoi(message->getIdentity()));
        return false;
      }

      B2ERROR("Invalid message from worker");
      return true;
    };

    m_zmqClient.poll(timeout, multicastAnswer, socketAnswer);
    B2ASSERT("Did not receive any ready messaged for quite some time!", not m_nextWorker.empty());

    const unsigned int nextWorker = m_nextWorker.front();
    m_nextWorker.pop_front();
    B2RESULT("Next worker is " << nextWorker);

    const auto& eventMessage = m_streamer.stream();

    if (eventMessage->size() > 0) {
      const auto& message = ZMQMessageFactory::createMessage(std::to_string(nextWorker), eventMessage);
      m_zmqClient.send(message);
      B2RESULT("Having send message to worker " << nextWorker);

      m_procEvtBackupList.storeEvt(eventMessage, m_eventMetaData, nextWorker);
      B2RESULT("stored event " << m_eventMetaData->getEvent() << " backup.. list size: " << m_procEvtBackupList.size());
      checkWorkerProcTimeout();
      B2RESULT("finished event");
    }
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Tx input event: " << ex.what());
    }
  } catch (exception& ex) {
    B2ERROR(ex.what());

  }
}

//TODO: wait for confirmation before deleting when sending backup messages to output
void ZMQTxInputModule::checkWorkerProcTimeout()
{
  // HUH?
  const auto& workerProcTimeout =  std::chrono::duration<int, std::ratio<1, 1000>>(6000);
  int workerID = m_procEvtBackupList.checkForTimeout(workerProcTimeout);
  if (workerID > -1) {
    B2WARNING("Worker process timeout, workerID: " << workerID);
    const auto& deathMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_deathMessage, std::to_string(workerID));
    m_zmqClient.publish(deathMessage);

    m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
    m_nextWorker.erase(std::remove(m_nextWorker.begin(), m_nextWorker.end(), workerID), m_nextWorker.end());
  }
}

void ZMQTxInputModule::terminate()
{
  for (unsigned int workerID : m_workers) {
    std::string workerIDString = std::to_string(workerID);
    const auto& message = ZMQMessageFactory::createMessage(workerIDString, c_MessageTypes::c_endMessage);
    m_zmqClient.send(message);
  }

  const auto multicastAnswer = [this](const auto & socket) {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
    const std::string& data = multicastMessage->getData();

    if (multicastMessage->isMessage(c_MessageTypes::c_confirmMessage)) {
      const auto& eventMetaData = EventMetaDataSerialization::deserialize(data);
      m_procEvtBackupList.removeEvt(eventMetaData);
      B2RESULT("removed event backup.. list size: " << m_procEvtBackupList.size());
      return true;
    } else if (multicastMessage->isMessage(c_MessageTypes::c_deleteMessage)) {
      const int workerID = std::atoi(data.c_str());

      B2RESULT("received worker delete message, workerID: " << workerID);
      m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
      return true;
    }

    B2ERROR("Received unknown message!");
    return true;
  };

  while (m_procEvtBackupList.size() > 0) {
    checkWorkerProcTimeout();
    m_zmqClient.pollMulticast(0, multicastAnswer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // this message is especially for the output, all events reached the output
  const auto& message = ZMQMessageFactory::createMessage(c_MessageTypes::c_endMessage);
  m_zmqClient.publish(message);

  m_zmqClient.terminate();
}
