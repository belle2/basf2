#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQTxWorkerModule.h>

#include <framework/pcore/DataStoreStreamer.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQTxWorker)

void ZMQTxWorkerModule::createSocket()
{
  B2DEBUG(100, "Creating socket for push: " << m_param_socketName);
  m_socket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUSH);
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
    //proceedMulticast();

    // #########################################################
    // 2. Send event to output
    // #########################################################
    const auto& message = ZMQMessageFactory::createMessage(m_streamer);
    B2DEBUG(100, "send event to output");
    message->toSocket(m_socket);
  } catch (zmq::error_t& ex) {
    if (ex.num() != EINTR) {
      B2ERROR("There was an error during the Tx worker event: " << ex.what());
    }
  }
}

// -------------------------------------------------------------------------------------

void ZMQTxWorkerModule::terminate()
{
  const auto& multicastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_terminateMessage, getpid());
  multicastMessage->toSocket(m_pubSocket);


  if (m_socket) {
    m_socket->close();
    m_socket.release();
  }
  if (m_pubSocket) {
    m_pubSocket->close();
    m_pubSocket.release();
  }
  if (m_subSocket) {
    m_subSocket->close();
    m_subSocket.release();
  }
  if (m_context) {
    m_context->close();
    m_context.release();
  }
}
/*
if (m_firstEvent) {
  initializeObjects(false);
  m_firstEvent = false;
}
// If the process is finished, send an end message to the listening socket.
const auto& message = ZMQMessageFactory::createMessage(c_MessageTypes::c_endMessage);
message->toSocket(m_socket);
 */


void ZMQTxWorkerModule::proceedMulticast()
{
}