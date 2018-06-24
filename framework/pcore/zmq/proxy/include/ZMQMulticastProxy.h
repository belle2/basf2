#pragma once

#include <zmq.hpp>
#include <memory>

namespace Belle2 {

  class ZMQMulticastProxy {
  public:
    ZMQMulticastProxy(std::string xpubProxySocketAddr, std::string xsubProxySocketAddr):
      m_xpubProxySocketAddr(xpubProxySocketAddr), m_xsubProxySocketAddr(xsubProxySocketAddr)
    {
      m_ctx = std::make_unique<zmq::context_t>(1);

      m_xpubSocket = std::make_unique<zmq::socket_t>(*m_ctx, ZMQ_XPUB);
      m_xsubSocket = std::make_unique<zmq::socket_t>(*m_ctx, ZMQ_XSUB);
      // ATTENTION: this is switched on intention!
      m_xpubSocket->bind(m_xsubProxySocketAddr);
      m_xsubSocket->bind(m_xpubProxySocketAddr);

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
    std::unique_ptr<zmq::context_t> m_ctx;
    std::unique_ptr<zmq::socket_t> m_xpubSocket;
    std::unique_ptr<zmq::socket_t> m_xsubSocket;

    std::string m_xpubProxySocketAddr;
    std::string m_xsubProxySocketAddr;

  };


}