#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxWorkerModule.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/RandomNumbers.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxWorker)


void ZMQRxWorkerModule::createSocket()
{
  B2DEBUG(100, "Creating socket for RxWorker: " << m_param_socketName);
  // set the worker process id as uniqueID
  const std::string& workerIDAsString = m_uniqueID = std::to_string(ProcHandler::EvtProcID());
  m_socket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_DEALER);
  B2DEBUG(100, "Set dealer socket id to " << workerIDAsString.data());
  m_socket->setsockopt(ZMQ_IDENTITY, workerIDAsString.c_str(), workerIDAsString.length());

  sleep(m_helloMulticastDelay);
  // send out hello with id to multicast
  const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, "worker");
  multicastHelloMsg->toSocket(m_pubSocket);
  B2DEBUG(100, "sent worker c_helloMessage");
}



// ---------------------------------- event ----------------------------------------------

void ZMQRxWorkerModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(false);

      // #########################################################
      // 0. "Connect" with Input
      // #########################################################
      const auto& helloMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_whelloMessage, m_uniqueID);
      B2DEBUG(100, "worker sends c_whelloMessage...");
      sleep(1);
      helloMessage->toSocket(m_pubSocket);

      // is there reply from input with hello message? also listen to multicast
      bool gotInputHello = false;
      int pollReply = 0;
      do {
        pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, m_pollTimeout);
        B2ASSERT("Worker timeout", pollReply > 0);
        if (pollReply & c_subSocket) { //we got message from multicast
          proceedMulticast();
        }
        if (pollReply & c_socket) { //we got message from input
          const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
          B2ASSERT("Worker got unexpected message from input while waiting for hello", message->isMessage(c_MessageTypes::c_helloMessage));
          gotInputHello = true;
        }
      } while (not gotInputHello);

      // send ready msg x buffer size
      for (unsigned int bufferIndex = 0; bufferIndex < m_bufferSize; bufferIndex++) {
        const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
        readyMessage->toSocket(m_socket);
      }
      m_firstEvent = false;
    }

    // #########################################################
    // 1. Check sockets for messages
    // #########################################################
    B2DEBUG(100, "waiting for event message");
    bool gotEventMessage = false;
    int pollReply = 0;
    do {
      pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, m_pollTimeout);
      B2ASSERT("Worker timeout", pollReply > 0);
      if (pollReply & c_subSocket) { //we got message from multicast
        //proceedMulticast();
      }
      if (pollReply & c_socket) { //we got message from input
        const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
        if (message->isMessage(c_MessageTypes::c_eventMessage)) {
          B2DEBUG(100, "received event message... write it to data store");
          message->toDataStore(m_streamer, m_randomgenerator);
          const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
          readyMessage->toSocket(m_socket);
          B2DEBUG(100, "send ready message");
          gotEventMessage = true;
        } else if (message->isMessage(c_MessageTypes::c_endMessage)) {
          B2DEBUG(100, "received end message from input");
          break;
        } else {
          B2DEBUG(100, "received unexpected message from input");
          break;
        }
      }
    } while (not gotEventMessage);

    B2DEBUG(100, "Finished with event");
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Rx worker event: " << ex.what());
    }
  }
}

// -------------------------------------------------------------------------------------

void ZMQRxWorkerModule::proceedMulticast()
{
}
