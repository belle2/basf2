#include <framework/pcore/zeromq/ZeroMQTxInputModule.h>

#include <chrono>
#include <thread>

using namespace std;
using namespace Belle2;

REG_MODULE(ZeroMQTxInput)

void ZeroMQTxInputModule::createSocket()
{
  B2DEBUG(100, "Creating socket for push: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_ROUTER));
}

std::unique_ptr<ZeroMQRoutedMessage> ZeroMQTxInputModule::readEvent()
{
  // Prepare a message with the content of the data store. Set the worker to 0, as we do not now better in this moment.
  return ZeroMQRoutedMessage::fromDataStore(m_streamer, 0);
}

void ZeroMQTxInputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(true);
      m_firstEvent = false;
    }

    setRandomState();

    // Get all workers ready messages, but do not block
    getWorkersReadyMessages(false);

    const std::unique_ptr<ZeroMQRoutedMessage>&& message = readEvent();

    if (not message->empty()) {
      // Get the ID of the next worker to process
      const unsigned int nextWorkerID = getNextWorker();
      B2DEBUG(100, "Next worker is " << nextWorkerID);

      // Edit the worker ID of the message.
      message->setWorkerId(nextWorkerID);

      // Send the message
      message->toSocket(m_socket);
      B2DEBUG(100, "Having send message to worker.");
    }
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Tx input event: " << ex.what());
  }
}

void ZeroMQTxInputModule::terminate()
{
  for (unsigned int workerID : m_workers) {
    const std::unique_ptr<ZeroMQRoutedMessage>& message = ZeroMQRoutedMessage::createEndMessage(workerID);
    message->toSocket(m_socket);
  }
}

unsigned int ZeroMQTxInputModule::getNextWorker()
{
  if (m_nextWorker.empty()) {
    getWorkersReadyMessages(true);
  }

  const unsigned int& nextWorker = m_nextWorker.front();
  m_nextWorker.pop_front();
  return nextWorker;
}

bool poll(std::unique_ptr<zmq::socket_t>& socket, int timeout)
{
  zmq::pollitem_t items [] = {
    { static_cast<void*>(*socket), 0, ZMQ_POLLIN, 0 }
  };
  zmq::poll(&items[0], 1, timeout);
  return static_cast<bool>(items [0].revents & ZMQ_POLLIN);
}

void ZeroMQTxInputModule::getWorkersReadyMessages(bool blocking)
{
  B2DEBUG(100, "Start getting workers messages");
  while ((not blocking and poll(m_socket, 0)) or (blocking and m_nextWorker.empty() and poll(m_socket, -1))) {
    const std::unique_ptr<ZeroMQRoutedMessage>& message = ZeroMQRoutedMessage::fromSocket(m_socket);

    if (message->isReadyMessage()) {
      m_nextWorker.push_back(message->getWorkerID());
    } else if (message->isHelloMessage()) {
      const unsigned int workerID = message->getWorkerID();

      m_workers.push_back(workerID);

      const std::unique_ptr<ZeroMQRoutedMessage>& answer = ZeroMQRoutedMessage::createHelloMessage();
      answer->setWorkerId(workerID);
      answer->toSocket(m_socket);
    } else {
      B2FATAL("Invalid message from worker");
    }
  }
  B2DEBUG(100, "End getting workers messages");
}
