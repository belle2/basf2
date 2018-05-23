#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/processModules/ZMQRxOutputModule.h>

#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>


using namespace std;
using namespace Belle2;

REG_MODULE(ZMQRxOutput)



void ZMQRxOutputModule::createSocket()
{
  m_socket.reset(new zmq::socket_t(*m_context, ZMQ_PULL));

  sleep(m_helloMulticastDelay);
  std::string message = "output";
  const auto& multicastHelloMsg = ZMQMessageFactory::createMessage(c_MessageTypes::c_helloMessage, message);
  sleep(0.1);
  multicastHelloMsg->toSocket(m_pubSocket);
  B2DEBUG(100, "output sent hello message... waits for start...");
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

    ZMQHelper::pollSocket(m_socket, -1);

    const auto& message = ZMQMessageFactory::fromSocket<ZMQNoIdMessage>(m_socket);

    if (message->isMessage(c_MessageTypes::c_eventMessage)) {
      B2DEBUG(100, "Received event Message");
      writeEvent(message);

    }

    else if (message->isMessage(c_MessageTypes::c_endMessage)) {
      B2DEBUG(100, "Received end Message");
    } else {
      B2FATAL("Unexpected message");
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
