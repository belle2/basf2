#include <zmq.hpp>
#include <framework/pcore/zmq/proxy/ZMQMulticastProxy.h>
#include <framework/logging/LogMethod.h>
#include <unistd.h>
#include <iostream>

using namespace Belle2;

void ZMQMulticastProxy::start()
{
  try {
    zmq::proxy(*m_xpubSocket, *m_xsubSocket, nullptr);
  } catch (zmq::error_t error) {
    if (error.num() == EINTR) {
      return;
    } else {
      throw error;
    }
  }
}


void ZMQMulticastProxy::shutdown()
{
  std::cout << "Proxy shutdown << std::endl";
  if (m_xsubSocket) {
    m_xsubSocket->close();
    m_xsubSocket.release();
  }
  if (m_xpubSocket) {
    m_xpubSocket->close();
    m_xpubSocket.release();
  }
  if (m_ctx) {
    m_ctx->close();
    m_ctx.release();
  }
}