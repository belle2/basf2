#include <framework/pcore/zmq/processModules/ZMQModule.h>
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

  initBroadcast();
  subscribeBroadcast(c_MessageTypes::c_broadcastMessage);
  createSocket();

  m_socket->setsockopt(ZMQ_LINGER, 0);
  if (bindToEndPoint) {
    m_socket->bind(m_param_socketName.c_str());
  } else {
    m_socket->connect(m_param_socketName.c_str());
  }
  B2DEBUG(100, "Created socket: " << m_param_socketName);

}



void ZMQModule::initBroadcast()
{
  m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
  m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);

  m_pubSocket->connect(m_param_xsubProxySocket);
  m_subSocket->connect(m_param_xpubProxySocket);

  m_pubSocket->setsockopt(ZMQ_LINGER, 0);
  m_subSocket->setsockopt(ZMQ_LINGER, 0);
}



void ZMQModule::subscribeBroadcast(const c_MessageTypes filter)
{
  const char char_filter = static_cast<char>(filter);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &char_filter, 1);
}



ZMQModule::~ZMQModule()
{
  std::cout << "Destroy ZMQ" << std::endl;
  if (m_socket) {
    std::cout << "Destroy socket " << m_param_socketName << std::endl;
    m_socket->close();
    m_socket.reset();
  }
  if (m_subscribeSocket) {
    std::cout << "Destroy socket " << m_param_subscribeSocketName << std::endl;
    m_subscribeSocket->close();
    m_subscribeSocket.reset();

  }
  if (m_context) {
    m_context->close();
    m_context.reset();
  }
}