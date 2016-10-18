#include <framework/pcore/zeromq/ZeroMQRxOutputModule.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZeroMQRxOutput)

void ZeroMQRxOutputModule::createSocket()
{
  B2DEBUG(100, "Creating socket for pull: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PULL));
}

void ZeroMQRxOutputModule::writeEvent(const std::unique_ptr<ZeroMQMessage>& message)
{
  message->toDataStore(m_streamer, m_randomgenerator);
}

void ZeroMQRxOutputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(true);
      m_firstEvent = false;
    }

    B2DEBUG(100, "reading in an event.");
    while (true) {
      B2DEBUG(100,  m_param_socketName << ": receiving message");
      const std::unique_ptr<ZeroMQMessage>& message = ZeroMQMessage::fromSocket(m_socket);

      // Check if message is an end message
      // TODO: Check not only number of workers, but also their identity
      if (message->isEndMessage()) {
        B2DEBUG(100, "Received end message");
        m_numberOfAliveWorkers--;
        B2DEBUG(100, "Still present " << m_numberOfAliveWorkers);
        if (m_numberOfAliveWorkers <= 0) {
          // We will not write anything into the datastore.
          // This will terminate the process (as this is the master module).
          B2DEBUG(100, "No on is around. I go home.");
          break;
        }
        // In the other case we go on to listen to the next message - which is either another "goodbye" message or
        // a real event
      } else if (message->isHelloMessage()) {
        m_numberOfAliveWorkers++;
      } else {
        // We have received a "normal" event - so lets just write it back into the datastore.
        writeEvent(message);
        break;
      }
    }
    B2DEBUG(100, "finished reading in an event.");

  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Rx output event: " << ex.what());
  }
}
