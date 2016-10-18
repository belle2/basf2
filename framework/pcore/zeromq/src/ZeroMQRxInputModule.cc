#include <framework/pcore/zeromq/ZeroMQRxInputModule.h>

#include <framework/pcore/ProcHandler.h>
#include <framework/core/RandomNumbers.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZeroMQRxInput)

void ZeroMQRxInputModule::createSocket()
{
  B2DEBUG(100, "Creating socket for dealer: " << m_param_socketName);

  const std::string& workerIDAsString = m_uniqueID = std::to_string(ProcHandler::EvtProcID());

  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_DEALER));
  m_socket->setsockopt(ZMQ_IDENTITY, workerIDAsString.data(), workerIDAsString.size());
}

void ZeroMQRxInputModule::event()
{
  try {
    // In the beginning of the first event, send as many ready messages as we want in our buffer
    if (m_firstEvent) {
      initializeObjects(false);

      const std::unique_ptr<ZeroMQMessage>& helloMessage = ZeroMQMessage::createHelloMessage();
      helloMessage->toSocket(m_socket);

      // Wait for answer 10 seconds
      zmq::pollitem_t items[] = {
        {static_cast<void*>(*m_socket), 0, ZMQ_POLLIN, 0}
      };
      zmq::poll(&items[0], 1, 10000);
      if (items[0].revents & ZMQ_POLLIN) {
        ZeroMQMessage::fromSocket(m_socket);
      } else {
        B2WARNING("No answer from input process. Closing!");
        return;
      }

      for (unsigned int bufferIndex = 0; bufferIndex < m_bufferSize; bufferIndex++) {
        const std::unique_ptr<ZeroMQMessage>& readyMessage = ZeroMQMessage::createReadyMessage();
        readyMessage->toSocket(m_socket);
      }

      m_firstEvent = false;
    }

    B2DEBUG(100, "Start waiting for message");
    const std::unique_ptr<ZeroMQMessage>& message = ZeroMQMessage::fromSocket(m_socket);

    // Check if message is an end message. If not, read in the event. If yes, just do nothing
    // (will terminate the process, as this is the master module)
    if (not message->isEndMessage()) {
      message->toDataStore(m_streamer, m_randomgenerator);

      const std::unique_ptr<ZeroMQMessage>& readyMessage = ZeroMQMessage::createReadyMessage();
      readyMessage->toSocket(m_socket);
    }
    B2DEBUG(100, "Finished with event");
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Rx input event: " << ex.what());
  }
}
