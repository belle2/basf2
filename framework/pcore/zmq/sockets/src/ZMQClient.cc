/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/zmq/sockets/ZMQClient.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace Belle2;

template <int AZMQType>
void ZMQClient<AZMQType>::terminate()
{
  const auto& multicastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_terminateMessage, getpid());
  multicastMessage->toSocket(m_pubSocket);

  if (m_socket) {
    m_socket->close();
    m_socket.release();
  }
  if (m_pubSocket) {
    m_pubSocket->close();
    m_pubSocket.release();
  }
  if (m_subSocket) {
    m_subSocket->close();
    m_subSocket.release();
  }
  if (m_context) {
    m_context->close();
    m_context.release();
  }
}

template <int AZMQType>
void ZMQClient<AZMQType>::initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                     const std::string& socketAddress, bool bind)
{
  m_context = std::make_unique<zmq::context_t>(1);
  m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
  m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);

  m_pubSocket->connect(pubSocketAddress);
  m_pubSocket->setsockopt(ZMQ_LINGER, 0);

  m_subSocket->connect(subSocketAddress);
  m_subSocket->setsockopt(ZMQ_LINGER, 0);

  B2DEBUG(200, "Having initialized multicast with sub on " << subSocketAddress << " and pub on " << pubSocketAddress);

  m_socket = std::make_unique<zmq::socket_t>(*m_context, AZMQType);

  m_socket->setsockopt(ZMQ_LINGER, 0);
  if (bind) {
    m_socket->bind(socketAddress.c_str());
  } else {
    m_socket->connect(socketAddress.c_str());
  }

  // Give the sockets some time to start
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  B2DEBUG(100, "Created socket: " << socketAddress);

  m_pollSocketPtrList.clear();
  m_pollSocketPtrList.push_back(m_subSocket.get());
  m_pollSocketPtrList.push_back(m_socket.get());
}

template <int AZMQType>
void ZMQClient<AZMQType>::subscribe(c_MessageTypes filter)
{
  const char char_filter = static_cast<char>(filter);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &char_filter, 1);
}

template class Belle2::ZMQClient<ZMQ_PUSH>;
template class Belle2::ZMQClient<ZMQ_PULL>;