#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/zmq/processModules/ZMQModule.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/zmq/messages/ZMQIdMessage.h>
#include <TSystem.h>

using namespace std;
using namespace Belle2;


void ZMQModule::initializeObjects(bool bindToEndPoint)
{
  gSystem->Load("libdataobjects");
  m_streamer.reset(new DataStoreStreamer(m_param_compressionLevel, m_param_handleMergeable));

  if ((Environment::Instance().getStreamingObjects()).size() > 0) {
    m_streamer->setStreamingObjects(Environment::Instance().getStreamingObjects());
    B2INFO("Tx: Streaming objects limited : " << (Environment::Instance().getStreamingObjects()).size() << " objects");
  }
  // context for all the zmq communication
  m_context = std::make_unique<zmq::context_t>(1);

  initMulticast();
  B2DEBUG(100, "multicast is online");

  createSocket(); // This socket is for the direct communication between neighbor rx/tx modules

  m_socket->setsockopt(ZMQ_LINGER, 0);
  if (bindToEndPoint) {
    m_socket->bind(m_param_socketName.c_str());
  } else {
    m_socket->connect(m_param_socketName.c_str());
  }
  B2DEBUG(100, "Created socket: " << m_param_socketName);

  m_pollSocketPtrList.push_back(m_subSocket.get());   // c_subSocket = 1 -> 000001
  m_pollSocketPtrList.push_back(m_socket.get());      // c_socket = 2    -> 000010

}



void ZMQModule::initMulticast()
{
  m_context = std::make_unique<zmq::context_t>(1);
  m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
  m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);

  m_pubSocket->connect(m_param_xpubProxySocketName);
  m_subSocket->connect(m_param_xsubProxySocketName);

  // how long should message which have not been send yet stay in the memory after disconnecting socket
  m_pubSocket->setsockopt(ZMQ_LINGER, 0);
  m_subSocket->setsockopt(ZMQ_LINGER, 0);

  B2DEBUG(200, "Having initialized multicast with sub on " << m_param_xsubProxySocketName << " and pub on " <<
          m_param_xpubProxySocketName);
}



void ZMQModule::subscribeMulticast(const c_MessageTypes filter)
{
  const char char_filter = static_cast<char>(filter);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &char_filter, 1);
}