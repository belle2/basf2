#include <framework/pcore/zeromq/ZeroMQTxOutputModule.h>
#include <framework/pcore/zeromq/ZeroMQMessage.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ZeroMQTxOutput)

void ZeroMQTxOutputModule::createSocket()
{
  B2DEBUG(100, "Creating socket for push: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PUSH));
}

void ZeroMQTxOutputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(false);
      m_firstEvent = false;

      const std::unique_ptr<ZeroMQMessage>& helloMessage = ZeroMQMessage::createHelloMessage();
      helloMessage->toSocket(m_socket);
    }

    setRandomState();

    const std::unique_ptr<ZeroMQMessage>& message = ZeroMQMessage::fromDataStore(m_streamer);
    message->toSocket(m_socket);
  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Tx output event: " << ex.what());
  }
}

void ZeroMQTxOutputModule::terminate()
{
  if (m_firstEvent) {
    initializeObjects(false);
    m_firstEvent = false;
  }

  // If the process is finished, send an end message to the listening socket.
  const std::unique_ptr<ZeroMQMessage>& message = ZeroMQMessage::createEndMessage();
  message->toSocket(m_socket);
}
