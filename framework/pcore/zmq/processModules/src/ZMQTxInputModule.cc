#include <framework/pcore/zmq/processModules/ZMQHelper.h>
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
  B2DEBUG(100, "Creating socket for TxInput: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_ROUTER));

  sleep(m_helloMulticastDelay);
  // send out hello with id to multicast
  std::string message = "input";
  const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, message);
  sleep(0.1);
  multicastHelloMsg->toSocket(m_pubSocket);
  B2DEBUG(100, "sent input c_helloMessage");
}



std::unique_ptr<ZMQIdMessage> ZMQTxInputModule::readEventToMessage(std::string& NextWorkerID)
{
  return ZMQMessageFactory::createMessage(NextWorkerID, c_MessageTypes::c_eventMessage, m_streamer);
}

// ---------------------------------- event ----------------------------------------------

void ZMQTxInputModule::event()
{
  try {
    if (m_firstEvent) {
      B2DEBUG(100, "first TxInput event started...");
      initializeObjects(true);
      m_firstEvent = false;

      // set the message types we listen to on the multicast
      subscribeMulticast(c_MessageTypes::c_confirmMessage);
      subscribeMulticast(c_MessageTypes::c_whelloMessage);

      B2DEBUG(100, "input waits for first worker...");
    }

    setRandomState();

    // #########################################################
    // 1. Check sockets for messages
    // #########################################################
    do {
      int pollReply = 0;
      if (m_nextWorker.empty()) {
        pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, m_pollTimeout);
        B2ASSERT("Input timeout", pollReply > 0);
      } else { // if next worker are available dont waste time
        pollReply = ZMQHelper::pollSockets(m_pollSocketPtrList, 0);
      }

      if (pollReply & c_subSocket) { //we got message from multicast
        proceedMulticast();
      }
      if (pollReply & c_socket) { //we got message from worker
        getWorkersReadyMessages();
      }
    } while (m_nextWorker.empty());

    // #################################################
    // 2. Send workload
    // #################################################
    const unsigned int nextWorkerId = getNextWorker();
    std::string nextWorkerIdString = std::to_string(nextWorkerId);
    B2DEBUG(100, "Next worker is " << nextWorkerId);

    const auto&& message = readEventToMessage(nextWorkerIdString);
    if (not message->isEmpty()) {
      message->toSocket(m_socket);
      B2DEBUG(100, "Having send message to worker " << nextWorkerId);
    }
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Tx input event: " << ex.what());
  }
}

// -------------------------------------------------------------------------------------


unsigned int ZMQTxInputModule::getNextWorker()
{
  const unsigned int nextWorker = m_nextWorker.front();
  m_nextWorker.pop_front();
  return nextWorker;
}


void ZMQTxInputModule::getWorkersReadyMessages()
{
  do {
    const auto& message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);
    if (message->isMessage(c_MessageTypes::c_readyMessage)) {
      B2DEBUG(100, "got worker ready messge");
      m_nextWorker.push_back(std::stoi(message->getIdentity()));
    } else {
      B2ERROR("Invalid message from worker");
    }
  } while (ZMQHelper::pollSocket(m_socket, 0));

  /*B2DEBUG(100, "Start getting workers messages");
  while ((not blocking and ZMQHelper::pollSocket(m_socket, 0)) or (blocking and m_nextWorker.empty()
         and ZMQHelper::pollSocket(m_socket, -1))) {
    const auto& message = ZMQMessageFactory::fromSocket<ZMQIdMessage>(m_socket);

    if (message->isMessage(c_MessageTypes::c_readyMessage)) {
      B2DEBUG(100, "got ready messge");
      m_nextWorker.push_back(std::stoi(message->getIdentity()));
    } else {
      B2FATAL("Invalid message from worker");
    }
  }*/
}


void ZMQTxInputModule::proceedMulticast()
{
  do {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (multicastMessage->isMessage(c_MessageTypes::c_whelloMessage)) {
      std::string workerID = multicastMessage->getData();
      if (m_workers.size() < m_numWorker) {
        m_workers.push_back(std::stoi(workerID));
        //send back hello message to receive worker ready message in next step
        B2DEBUG(100, "received c_whelloMessage from " << workerID << "... replying");
        const auto& replyHelloMessage = ZMQMessageFactory::createMessage(workerID, c_MessageTypes::c_helloMessage);
        replyHelloMessage->toSocket(m_socket);
      } else {
        B2FATAL("m_workers exceeds number of total worker processes");
      }
    }
  } while (ZMQHelper::pollSocket(m_subSocket, 0));
  /*
  while (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (multicastMessage->isMessage(c_MessageTypes::c_whelloMessage)) {
      std::string workerID = multicastMessage->getData();
      m_workers.push_back(std::stoi(workerID));
      if (m_workers.size() > m_numWorker) {
        B2FATAL("m_workers exceeds number of total worker processes");
      } else {
        //send back hello message to receive worker ready message in next step
        B2DEBUG(100, "received c_whelloMessage from " << workerID);
        const auto& replyHelloMessage = ZMQMessageFactory::createMessage(workerID, c_MessageTypes::c_helloMessage);
        B2DEBUG(100, "reply helly to worker " << workerID);
        replyHelloMessage->toSocket(m_socket);
      }
    }
  }*/
}


void ZMQTxInputModule::terminate()
{
  for (unsigned int workerID : m_workers) {
    std::string workerIDString = std::to_string(workerID);

    // TODO: do we need to send endMessages?
    const auto& message = ZMQMessageFactory::createMessage(workerIDString, c_MessageTypes::c_endMessage);
    message->toSocket(m_socket);
  }
}