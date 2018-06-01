#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>


using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput)



void ZMQRxOutputModule::createSocket()
{
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PULL));

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
      m_firstEvent = false;
    }

    // #########################################################
    // 1. Check sockets for messages
    // #########################################################
    bool gotEventMessage = false;
    int pollReply = 0;
    do {
      pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, m_pollTimeout);
      B2ASSERT("Worker timeout", pollReply > 0);
      if (pollReply & c_subSocket) { //we got message from multicast
        proceedMulticast();
      }
      if (pollReply & c_socket) { //we got message from input
        const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
        if (message->isMessage(c_MessageTypes::c_eventMessage)) {
          B2DEBUG(100, "received event message");
          writeEvent(message); // write back to data store
          sleep(2);
          UniqueEventId evtId(m_eventMetaData->getEvent(),
                              m_eventMetaData->getRun(),
                              m_eventMetaData->getExperiment(),
                              time(NULL));
          B2DEBUG(100, "received event " << m_eventMetaData->getEvent());
          const auto& confirmMessage = ZMQMessageFactory::createMessage(evtId);
          confirmMessage->toSocket(m_pubSocket);
          gotEventMessage = true;
        } else if (message->isMessage(c_MessageTypes::c_endMessage)) {
          B2DEBUG(100, "received end message from input");
          break;
        } else { B2DEBUG(100, "received unexpected message from input"); break;}
      }
    } while (not gotEventMessage);

    B2DEBUG(100, "finished reading in an event.");
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Rx output event: " << ex.what());
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
      B2WARNING("got event backup");
    }
    if (multicastMessage->isMessage(c_MessageTypes::c_endMessage)) {

    }

  }

}
