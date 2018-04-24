#include <framework/pcore/zmq/modules/ZMQTxInputModule.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <thread>


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
  return ZMQIdMessage::createMessage(NextWorkerID, c_MessageTypes::c_eventMessage, m_streamer);
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
      std::unique_ptr<ZMQNoIdMessage> brdcst_helloMsg = ZMQNoIdMessage::createMessage(c_MessageTypes::c_helloMessage, message);
      //brdcst_helloMsg->toSocket(m_pubSocket);
    }

    setRandomState();

    // first check the broadcast inbox and process all the messages, here u get all the worker ids from the whelloMessages
    int NUM_WORKER = 4;
    while (m_workers.size() < NUM_WORKER) { // TODO: replace NUM_WORKER
      pollBroadcast();
    }

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


int ZMQTxInputModule::pollBroadcast()
{
  while (pollSocket(m_subSocket, 0)) {
    const std::unique_ptr<ZMQNoIdMessage>& brdcst_message = ZMQNoIdMessage::fromSocket(m_subSocket);
    if (brdcst_message->isMessage(c_MessageTypes::c_whelloMessage)) {
    }

  }

}