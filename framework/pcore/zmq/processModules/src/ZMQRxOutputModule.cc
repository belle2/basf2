#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>
#include <chrono>


using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput)



void ZMQRxOutputModule::createSocket()
{
  m_socket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PULL);

  sleep(m_helloMulticastDelay);
  std::string message = "output";
  const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, message);
  sleep(0.1);
  multicastHelloMsg->toSocket(m_pubSocket);
  B2DEBUG(100, "output sent hello message... waits for start...");
}


void ZMQRxOutputModule::writeEvent(const std::unique_ptr<ZMQNoIdMessage>& message)
{
  message->toDataStore(m_streamer, m_randomgenerator);
}

// ---------------------------------- event ----------------------------------------------

void ZMQRxOutputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(true);
      subscribeMulticast(c_MessageTypes::c_eventMessage);
      subscribeMulticast(c_MessageTypes::c_endMessage);
      m_firstEvent = false;
      m_pollTimeout = 20000;
    }


    bool gotEventMessage = false;
    int pollReply = 0;
    if (not m_gotEndMessage) {
      do {
        // #########################################################
        // 1. Check sockets for messages
        // #########################################################
        // TODO: think about the poll timeout... combinate it with the process timeout is useful to detect total worker death or input death
        pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, 2000 + m_workerProcTimeout.count());
        B2ASSERT("Output timeout", pollReply > 0); // input or all worker dead
        if (pollReply & c_subSocket) { //we got message from multicast
          proceedMulticast();

          if (m_gotBackupEvtMessage) {
            m_gotBackupEvtMessage = false;
            B2WARNING("received event backup " << m_eventMetaData->getEvent());
            return;
          }
          if (m_gotEndMessage) {
            B2DEBUG(100, "received end message across multicast");
            return;
          }
        }
        if (pollReply & c_socket && not m_gotEndMessage) { //we got message from input
          const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
          if (message->isMessage(c_MessageTypes::c_eventMessage)) {
            B2DEBUG(100, "received event message");
            writeEvent(message); // write back to data store

            // #########################################################
            // 2. Confirm event message
            // #########################################################
            UniqueEventId evtId(m_eventMetaData->getEvent(),
                                m_eventMetaData->getRun(),
                                m_eventMetaData->getExperiment(),
                                std::chrono::system_clock::now());
            B2DEBUG(100, "received event " << m_eventMetaData->getEvent());
            const auto& confirmMessage = ZMQMessageFactory::createMessage(evtId);
            confirmMessage->toSocket(m_pubSocket);
            gotEventMessage = true;
          } else {
            B2DEBUG(100, "received unexpected message from input");
            break;
          }
        }
      } while (not gotEventMessage && not m_gotEndMessage);
    }

    B2DEBUG(100, "finished reading in an event.");
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Rx output event: " << ex.what());
    }
  }
}

// -------------------------------------------------------------------------------------

void ZMQRxOutputModule::proceedMulticast()
{
  while (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (multicastMessage->isMessage(c_MessageTypes::c_eventMessage)) {
      // TODO: set a flag?
      writeEvent(multicastMessage); // write back to data store
      m_gotBackupEvtMessage = true;
      break;
    } else if (multicastMessage->isMessage(c_MessageTypes::c_endMessage)) {
      m_gotEndMessage = true;
    } else {
      B2ERROR("Undefined message on multicast");
    }

  }

}
