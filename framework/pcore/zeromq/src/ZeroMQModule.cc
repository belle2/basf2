#include <framework/pcore/zeromq/ZeroMQModule.h>
#include <framework/core/Environment.h>
#include <TSystem.h>

using namespace std;
using namespace Belle2;

void ZeroMQModule::initializeObjects(bool bindToEndPoint)
{
  gSystem->Load("libdataobjects");
  m_streamer.reset(new DataStoreStreamer(m_param_compressionLevel, m_param_handleMergeable));

  if ((Environment::Instance().getStreamingObjects()).size() > 0) {
    m_streamer->setStreamingObjects(Environment::Instance().getStreamingObjects());
    B2INFO("Tx: Streaming objects limited : " << (Environment::Instance().getStreamingObjects()).size() << " objects");
  }

  m_context.reset(new zmq::context_t(1));

  createSocket();

  int linger = 0;
  m_socket->setsockopt(ZMQ_LINGER, &linger, sizeof linger);
  if (bindToEndPoint) {
    m_socket->bind(m_param_socketName.c_str());
  } else {
    m_socket->connect(m_param_socketName.c_str());
  }
  B2DEBUG(100, "Created socket: " << m_param_socketName);
}