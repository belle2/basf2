/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/pcore/zmq/processModules/ZMQTxInputModule.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/utils/EventMetaDataSerialization.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/core/Environment.h>
#include <framework/core/RandomGenerator.h>
#include <framework/datastore/StoreObjPtr.h>
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
  addParam("maximalWaitingTime", m_param_maximalWaitingTime, "Maximal time to wait for any message");
  addParam("workerProcessTimeout", m_param_workerProcessTimeout, "Maximal time a worker is allowed to spent per event");
  addParam("useEventBackup", m_param_useEventBackup, "Turn on the event backup");

  setPropertyFlags(EModulePropFlags::c_ParallelProcessingCertified);

  B2ASSERT("Module is only allowed in a multiprocessing environment. If you only want to use a single process,"
           "set the number of processes to at least 1.",
           Environment::Instance().getNumberProcesses());
}

void ZMQTxInputModule::initialize()
{
  StoreObjPtr<RandomGenerator> randomgenerator;
  randomgenerator.registerInDataStore(DataStore::c_DontWriteOut);
}

void ZMQTxInputModule::event()
{
  try {
    if (m_firstEvent) {
      m_streamer.initialize(m_param_compressionLevel, m_param_handleMergeable);
      m_zmqClient.initialize<ZMQ_ROUTER>(m_param_xpubProxySocketName, m_param_xsubProxySocketName, m_param_socketName, true);

      auto multicastHelloMsg = ZMQMessageFactory::createMessage(EMessageTypes::c_helloMessage, getpid());
      m_zmqClient.publish(std::move(multicastHelloMsg));

      // Listen to event confirmations, hello of workers, the messages to delete a worker and the general stop messages
      m_zmqClient.subscribe(EMessageTypes::c_confirmMessage);
      m_zmqClient.subscribe(EMessageTypes::c_helloMessage);
      m_zmqClient.subscribe(EMessageTypes::c_deleteWorkerMessage);
      m_zmqClient.subscribe(EMessageTypes::c_terminateMessage);

      m_firstEvent = false;
    }

    if (not m_zmqClient.isOnline()) {
      return;
    }

    int timeout = m_param_maximalWaitingTime;
    if (not m_nextWorker.empty()) {
      // if next worker are available do not waste time
      timeout = 0;
    }

    bool terminate = false;

    const auto multicastAnswer = [this, &terminate](const auto & socket) {
      const auto multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
      const std::string& data = multicastMessage->getData();

      if (multicastMessage->isMessage(EMessageTypes::c_helloMessage)) {
        m_workers.push_back(std::stoi(data));
        B2DEBUG(10, "received c_helloMessage from " << data << "... replying");
        auto replyHelloMessage = ZMQMessageFactory::createMessage(data, EMessageTypes::c_helloMessage);
        m_zmqClient.send(std::move(replyHelloMessage));
        return true;
      } else if (multicastMessage->isMessage(EMessageTypes::c_confirmMessage) and m_param_useEventBackup) {
        const auto& eventMetaData = EventMetaDataSerialization::deserialize(data);
        m_procEvtBackupList.removeEvent(eventMetaData);
        B2DEBUG(10, "removed event backup.. list size: " << m_procEvtBackupList.size());
        return true;
      } else if (multicastMessage->isMessage(EMessageTypes::c_deleteWorkerMessage) and m_param_useEventBackup) {
        const int workerID = std::atoi(data.c_str());
        B2DEBUG(10, "received worker delete message, workerID: " << workerID);

        m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
        m_nextWorker.erase(std::remove(m_nextWorker.begin(), m_nextWorker.end(), workerID), m_nextWorker.end());
        return true;
      } else if (multicastMessage->isMessage(EMessageTypes::c_terminateMessage)) {
        B2DEBUG(10, "Having received a stop message. I can not do much here, but just hope for the best.");
        terminate = true;
        return false;
      }

      return true;
    };

    const auto socketAnswer = [this](const auto & socket) {
      const auto message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(socket);
      if (message->isMessage(EMessageTypes::c_readyMessage)) {
        B2DEBUG(10, "got worker ready message");
        m_nextWorker.push_back(std::stoi(message->getIdentity()));
        return false;
      }

      B2ERROR("Invalid message from worker");
      return true;
    };

    m_zmqClient.poll(timeout, multicastAnswer, socketAnswer);
    // false positive due to lambda capture ...
    if (terminate) {
      m_zmqClient.terminate();
      return;
    }

    B2ASSERT("Did not receive any ready messaged for quite some time!", not m_nextWorker.empty());

    const unsigned int nextWorker = m_nextWorker.front();
    m_nextWorker.pop_front();
    B2DEBUG(10, "Next worker is " << nextWorker);

    auto eventMessage = m_streamer.stream();

    if (eventMessage->size() > 0) {
      auto message = ZMQMessageFactory::createMessage(std::to_string(nextWorker), EMessageTypes::c_eventMessage, eventMessage);
      m_zmqClient.send(std::move(message));
      B2DEBUG(10, "Having send message to worker " << nextWorker);

      if (m_param_useEventBackup) {
        m_procEvtBackupList.storeEvent(std::move(eventMessage), m_eventMetaData, nextWorker);
        B2DEBUG(10, "stored event " << m_eventMetaData->getEvent() << " backup.. list size: " << m_procEvtBackupList.size());
        checkWorkerProcTimeout();
      }
      B2DEBUG(10, "finished event");
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
  if (not m_param_useEventBackup or m_param_workerProcessTimeout == 0) {
    return;
  }

  const std::chrono::milliseconds workerProcTimeout(m_param_workerProcessTimeout);
  int workerID = m_procEvtBackupList.checkForTimeout(workerProcTimeout);
  if (workerID > -1) {
    B2WARNING("Worker process timeout, workerID: " << workerID);
    auto deathMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_killWorkerMessage, std::to_string(workerID));
    m_zmqClient.publish(std::move(deathMessage));

    m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
    m_nextWorker.erase(std::remove(m_nextWorker.begin(), m_nextWorker.end(), workerID), m_nextWorker.end());
  }
}

void ZMQTxInputModule::terminate()
{

  if (not m_zmqClient.isOnline()) {
    return;
  }

  for (unsigned int workerID : m_workers) {
    std::string workerIDString = std::to_string(workerID);
    auto message = ZMQMessageFactory::createMessage(workerIDString, EMessageTypes::c_lastEventMessage);
    m_zmqClient.send(std::move(message));
  }

  const auto multicastAnswer = [this](const auto & socket) {
    const auto multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(socket);
    const std::string& data = multicastMessage->getData();

    if (multicastMessage->isMessage(EMessageTypes::c_confirmMessage) and m_param_useEventBackup) {
      const auto& eventMetaData = EventMetaDataSerialization::deserialize(data);
      m_procEvtBackupList.removeEvent(eventMetaData);
      B2DEBUG(10, "removed event backup.. list size: " << m_procEvtBackupList.size());
      return true;
    } else if (multicastMessage->isMessage(EMessageTypes::c_deleteWorkerMessage) and m_param_useEventBackup) {
      const int workerID = std::atoi(data.c_str());

      B2DEBUG(10, "received worker delete message, workerID: " << workerID);
      m_procEvtBackupList.sendWorkerBackupEvents(workerID, m_zmqClient);
      return true;
    } else if (multicastMessage->isMessage(EMessageTypes::c_helloMessage)) {
      // A new worker? Well, he is quite late... nevertheless, lets tell him to end it
      B2DEBUG(10, "received c_helloMessage from " << data << "... replying with end message");
      auto message = ZMQMessageFactory::createMessage(data, EMessageTypes::c_lastEventMessage);
      m_zmqClient.send(std::move(message));
      return true;
    }
    return true;
  };

  while (m_param_useEventBackup and m_procEvtBackupList.size() > 0) {
    checkWorkerProcTimeout();
    m_zmqClient.pollMulticast(0, multicastAnswer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (m_param_useEventBackup) {
    // this message is especially for the output, all events reached the output
    auto message = ZMQMessageFactory::createMessage(EMessageTypes::c_lastEventMessage);
    m_zmqClient.publish(std::move(message));
  }

  m_zmqClient.terminate();
}
