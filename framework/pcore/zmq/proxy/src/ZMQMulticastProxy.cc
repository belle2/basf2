#include <zmq.hpp>
#include <framework/pcore/zmq/proxy/ZMQMulticastProxy.h>
#include <framework/logging/LogMethod.h>
#include <unistd.h>
#include <iostream>

using namespace Belle2;

void ZMQMulticastProxy::start()
{
  sleep(0.1);
  zmq::proxy(*m_xpubSocket, *m_xsubSocket, nullptr);
}


void ZMQMulticastProxy::shutdown()
{
  //std::cout << "Proxy shutdown << std::endl";
  if (m_xsubSocket) {
    m_xsubSocket->close();
    delete m_xsubSocket;
  }
  if (m_xpubSocket) {
    m_xpubSocket->close();
    delete m_xpubSocket;
  }
  if (m_ctx) {
    m_ctx->close();
    delete m_ctx;
  }
}