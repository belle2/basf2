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
  B2DEBUG(100, "Creating socket for deale: " << m_param_socketName);
  // set the worker process id as uniqueID
  const std::string& workerIDAsString = m_uniqueID = std::to_string(ProcHandler::EvtProcID());

  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_DEALER));
  m_socket->setsockopt(ZMQ_IDENTITY, workerIDAsString.data(), workerIDAsString.size());
}



void ZMQRxWorkerModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(false);

      const auto& helloMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_whelloMessage, m_uniqueID);
      helloMessage->toSocket(m_pubSocket);

      // is there reply from input with hello message?
      if (not ZMQHelper::pollSocket(m_socket, 0)) {
        B2ERROR("No hello message returned");
        return;
      }

      for (unsigned int bufferIndex = 0; bufferIndex < m_bufferSize; bufferIndex++) {
        const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
        readyMessage->toSocket(m_socket);
      }

      m_firstEvent = false;
    }

    proceedMulticast();

    B2DEBUG(100, "Start waiting for message");

    if (not ZMQHelper::pollSocket(m_socket, 0)) {
      B2ERROR("ZMQRxWorker fromSocket timeout");
      return;
    }

    const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);

    if (not message->isMessage(c_MessageTypes::c_endMessage)) {
      message->toDataStore(m_streamer, m_randomgenerator);

      const auto& readyMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_readyMessage);
      readyMessage->toSocket(m_socket);
    }
    B2DEBUG(100, "Finished with event");
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Rx input event: " << ex.what());
  }
}

void ZMQRxWorkerModule::proceedMulticast()
{
  while (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (multicastMessage->isMessage(c_MessageTypes::c_endMessage)) {

    }

  }

}
