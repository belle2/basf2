#include <framework/pcore/zmq/processModules/ZMQTxInputModule.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <thread>

int NUM_WORKER = 4;
// TODO: replace NUM_WORKER

using namespace std;
using namespace Belle2;

REG_MODULE(ZMQTxInput)

void ZMQTxInputModule::createSocket()
{
  // the InputTXModule holds the router socket for event message processing
  B2DEBUG(100, "Creating socket for distribution: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_ROUTER));
}



std::unique_ptr<ZMQIdMessage> ZMQTxInputModule::readEventToMessage(std::string& NextWorkerID)
{
  return ZMQMessageFactory::createMessage(NextWorkerID, c_MessageTypes::c_eventMessage, m_streamer);
}



void ZMQTxInputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(true);
      m_firstEvent = false;

      // set the message types we listen to on the broadcast
      subscribeBroadcast(c_MessageTypes::c_broadcastMessage);
      subscribeBroadcast(c_MessageTypes::c_confirmMessage);
      // send out hello with id to broadcast
      std::string message = "input";
      const auto& broadcastHelloMsg = ZMQNoIdMessage::createMessage(c_MessageTypes::c_helloMessage, message);
      broadcastHelloMsg->toSocket(m_pubSocket);
    }

    setRandomState();

    // first check the broadcast inbox and process all the messages, here u get all the worker ids from the whelloMessages datasegment
    proceedBroadcast();

    // Get all workers ready messages, but do not block
    getWorkersReadyMessages(false);

    const unsigned int nextWorkerId = getNextWorker();
    B2DEBUG(100, "Next worker is " << nextWorkerId);

    std::string nextWorkerIdString = std::to_string(nextWorkerId);
    const std::unique_ptr<ZMQIdMessage>&& message = readEventToMessage(nextWorkerIdString);
    if (not message->isEmpty()) {
      message->toSocket(m_socket);
      B2DEBUG(100, "Having send message to worker " << nextWorkerId);
    }


  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Tx input event: " << ex.what());
  }
}


unsigned int ZMQTxInputModule::getNextWorker()
{
  if (m_nextWorker.empty()) {
    getWorkersReadyMessages(true);
  }
  const unsigned int& nextWorker = m_nextWorker.front();
  m_nextWorker.pop_front();
  return nextWorker;
}


void ZMQTxInputModule::getWorkersReadyMessages(bool blocking)
{
  B2DEBUG(100, "Start getting workers messages");
  while ((not blocking and pollSocket(m_socket, 0)) or (blocking and m_nextWorker.empty() and pollSocket(m_socket, -1))) {
    const auto& message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);

    if (message->isMessage(c_MessageTypes::c_readyMessage)) {
      m_nextWorker.push_back(std::stoi(message->getIdentity()));
    } else {
      B2FATAL("Invalid message from worker");
    }
  }
}


void ZMQTxInputModule::proceedBroadcast()
{
  while (pollSocket(m_subSocket, 0)) {
    const auto& broadcastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (broadcastMessage->isMessage(c_MessageTypes::c_whelloMessage)) {
      std::string workerID = broadcastMessage->getData();
      m_workers.push_back(std::stoi(workerID));
      if (m_workers.size() > NUM_WORKER) {
        B2FATAL("m_workers exceeds number of worker processes");
      } else {
        //send back hello message to receive worker ready message in next step
        const auto& replyHelloMessage = ZMQMessageFactory::createMessage(workerID, c_MessageTypes::c_helloMessage);
        replyHelloMessage->toSocket(m_socket);
      }
    }

  }

}


void ZMQTxInputModule::terminate()
{
  for (unsigned int workerID : m_workers) {
    std::string workerIDString = std::to_string(workerID);
    const auto& message = ZMQMessageFactory::createMessage(workerIDString, c_MessageTypes::c_endMessage);
    message->toSocket(m_socket);
  }
}
