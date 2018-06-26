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

void ZMQClient::terminate(bool sendGoodbye)
{
  if (m_pubSocket and sendGoodbye) {
    auto multicastMessage = ZMQMessageFactory::createMessage(c_MessageTypes::c_terminateMessage, getpid());
    publish(std::move(multicastMessage));
  }

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

void ZMQClient::reset()
{
  m_context.release();
  m_subSocket.release();
  m_pubSocket.release();
  m_socket.release();
}


template <int AZMQType>
void ZMQClient::initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                           const std::string& socketAddress, bool bind)
{
  initialize(pubSocketAddress, subSocketAddress);
  m_socket = std::make_unique<zmq::socket_t>(*m_context, AZMQType);

  if (AZMQType == ZMQ_DEALER) {
    const std::string& uniqueID = std::to_string(getpid());
    m_socket->setsockopt(ZMQ_IDENTITY, uniqueID.c_str(), uniqueID.length());
  }

  m_socket->setsockopt(ZMQ_LINGER, 0);
  if (bind) {
    m_socket->bind(socketAddress.c_str());
  } else {
    m_socket->connect(socketAddress.c_str());
  }

  // Give the sockets some time to start
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  B2DEBUG(100, "Created socket: " << socketAddress);

  m_pollSocketPtrList.push_back(m_socket.get());
}

void ZMQClient::initialize(const std::string& pubSocketAddress, const std::string& subSocketAddress)
{
  m_context = std::make_unique<zmq::context_t>(1);
  m_pubSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_PUB);
  m_subSocket = std::make_unique<zmq::socket_t>(*m_context, ZMQ_SUB);

  m_pubSocket->connect(pubSocketAddress);
  m_pubSocket->setsockopt(ZMQ_LINGER, 0);

  m_subSocket->connect(subSocketAddress);
  m_subSocket->setsockopt(ZMQ_LINGER, 0);

  B2DEBUG(200, "Having initialized multicast with sub on " << subSocketAddress << " and pub on " << pubSocketAddress);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  m_pollSocketPtrList.clear();
  m_pollSocketPtrList.push_back(m_subSocket.get());
}

void ZMQClient::subscribe(c_MessageTypes filter)
{
  B2ASSERT("Can only run this on started clients", m_subSocket);
  const char char_filter = static_cast<char>(filter);
  m_subSocket->setsockopt(ZMQ_SUBSCRIBE, &char_filter, 1);
}

void ZMQClient::send(zmq::message_t& message) const
{
  B2ASSERT("Can only run this on started clients", m_socket);
  m_socket->send(message);
}

int ZMQClient::pollSocketVector(const std::vector<zmq::socket_t*>& socketList, int timeout)
{
  auto start = std::chrono::system_clock::now();
  int return_bitmask = 0;
  zmq::pollitem_t items[socketList.size()];

  for (unsigned int i = 0; i < socketList.size(); i++) {
    items[i].socket = static_cast<void*>(*socketList[i]);
    items[i].events = ZMQ_POLLIN;
    items[i].revents = 0;
  }

  while (timeout >= 0) {
    try {
      zmq::poll(items, socketList.size(), timeout);

      for (unsigned int i = 0; i < socketList.size(); i++) {
        if (static_cast<bool>(items[i].revents & ZMQ_POLLIN)) {
          return_bitmask = return_bitmask | 1 << i;
        }
      }
      return return_bitmask;
    } catch (zmq::error_t error) {
      if (error.num() == EINTR) {
        auto now = std::chrono::system_clock::now();
        timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
      } else {
        throw error;
      }
    }
  }
  return 0;
}

template void Belle2::ZMQClient::initialize<ZMQ_PUSH>(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                                      const std::string& socketAddress, bool bind);
template void Belle2::ZMQClient::initialize<ZMQ_PULL>(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                                      const std::string& socketAddress, bool bind);
template void Belle2::ZMQClient::initialize<ZMQ_DEALER>(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                                        const std::string& socketAddress, bool bind);
template void Belle2::ZMQClient::initialize<ZMQ_ROUTER>(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                                        const std::string& socketAddress, bool bind);
template void Belle2::ZMQClient::initialize<ZMQ_PUB>(const std::string& pubSocketAddress, const std::string& subSocketAddress,
                                                     const std::string& socketAddress, bool bind);