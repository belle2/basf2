#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>


using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput)



void ZMQRxOutputModule::createSocket()
{
  B2DEBUG(100, "Creating socket for pull: " << m_param_socketName);
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PULL));
}


void ZMQRxOutputModule::writeEvent(const std::unique_ptr<ZMQNoIdMessage>& message)
{
  message->toDataStore(m_streamer, m_randomgenerator);
}


void ZMQRxOutputModule::event()
{
  try {
    if (m_firstEvent) {
      initializeObjects(true);
      m_firstEvent = false;
    }

    proceedMulticast();
    B2DEBUG(100, "reading in an event.");
    //TODO: why while?
    while (true) {
      B2DEBUG(100, m_param_socketName << ": receiving message");
      const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);
      if (not message) {
        B2INFO("OutputRX fromSocket timeout");
        return;
      }

      if (message->isMessage(c_MessageTypes::c_eventMessage)) {
        B2DEBUG(100, "Received end event Message");
        writeEvent(message);
        break;
      }
      // In the other case we go on to listen to the next message - which is either another "goodbye" message or
      // a real event
    }

    B2DEBUG(100, "finished reading in an event.");

  } catch (zmq::error_t& ex) {
    B2ERROR("There was an error during the Rx output event: " << ex.what());
  }
}


void ZMQRxOutputModule::proceedMulticast()
{
  while (ZMQHelper::pollSocket(m_subSocket, 0)) {
    const auto& multicastMessage = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_subSocket);
    if (multicastMessage->isMessage(c_MessageTypes::c_endMessage)) {

    }

  }

}
