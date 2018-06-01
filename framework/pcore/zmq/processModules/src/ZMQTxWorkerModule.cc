#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQTxWorkerModule.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/UniqueEventId.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQTxWorker)

void ZMQTxWorkerModule::createSocket()
{
  B2DEBUG(100, "Creating socket for push: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PUSH));
}

// ---------------------------------- event ----------------------------------------------

void ZMQTxWorkerModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(false);
      m_firstEvent = false;
    }

    setRandomState();

    // #########################################################
    // 1. Check multicast for messages
    // #########################################################
    proceedMulticast();

    // #########################################################
    // 2. Send event to output
    // #########################################################
    const auto& message = ZMQMessageFactory::createMessage(m_streamer);
    message->toSocket(m_socket);
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the TxWorker event: " << ex.what());
  }
}

// -------------------------------------------------------------------------------------

void ZMQTxWorkerModule::terminate()
{
  if (m_firstEvent) {
    initializeObjects(false);
    m_firstEvent = false;
  }
  // If the process is finished, send an end message to the listening socket.
  const auto& message = ZMQMessageFactory::createMessage(c_MessageTypes::c_endMessage);
  message->toSocket(m_socket);
}


void ZMQTxWorkerModule::proceedMulticast()
{
  while (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& broadcastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (broadcastMessage->isMessage(c_MessageTypes::c_endMessage)) {
      B2RESULT("received end message... dont know what to do yet");
    }
  }
}