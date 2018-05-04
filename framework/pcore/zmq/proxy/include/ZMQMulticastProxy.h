#pragma once

#include <zmq.hpp>

namespace Belle2 {

  class ZMQMulticastProxy {
  public:
    ZMQMulticastProxy(std::string xpubProxySocketAddr, std::string xsubProxySocketAddr):
      m_xpubProxySocketAddr(xpubProxySocketAddr), m_xsubProxySocketAddr(xsubProxySocketAddr)
    {
      m_ctx = new zmq::context_t(1);

      m_xpubSocket = new zmq::socket_t(*m_ctx, ZMQ_XPUB);
      m_xsubSocket = new zmq::socket_t(*m_ctx, ZMQ_XSUB);
      m_xpubSocket->bind(m_xpubProxySocketAddr);
      m_xsubSocket->bind(m_xsubProxySocketAddr);

      m_xsubSocket->setsockopt(ZMQ_LINGER, 0);
      m_xpubSocket->setsockopt(ZMQ_LINGER, 0);
    }
    ~ZMQMulticastProxy()
    {
      shutdown();
    }

    void start();
    void shutdown();

  private:
    zmq::context_t* m_ctx;
    zmq::socket_t* m_xpubSocket;
    zmq::socket_t* m_xsubSocket;

    std::string m_xpubProxySocketAddr;
    std::string m_xsubProxySocketAddr;

  };


}