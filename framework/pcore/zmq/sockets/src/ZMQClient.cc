/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    auto multicastMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_terminateMessage, getpid());
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
    const std::string uniqueID = std::to_string(getpid());
    m_socket->set(zmq::sockopt::routing_id, uniqueID);
  }

  m_socket->set(zmq::sockopt::linger, 0);
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
  m_pubSocket->set(zmq::sockopt::linger, 0);

  m_subSocket->connect(subSocketAddress);
  m_subSocket->set(zmq::sockopt::linger, 0);

  B2DEBUG(200, "Having initialized multicast with sub on " << subSocketAddress << " and pub on " << pubSocketAddress);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  m_pollSocketPtrList.clear();
  m_pollSocketPtrList.push_back(m_subSocket.get());
}

void ZMQClient::subscribe(EMessageTypes filter)
{
  B2ASSERT("Can only run this on started clients", m_subSocket);
  char char_filter[2];
  char_filter[0] = static_cast<char>(filter);
  char_filter[1] = 0;
  m_subSocket->set(zmq::sockopt::subscribe, char_filter);
}

void ZMQClient::send(zmq::message_t& message) const
{
  B2ASSERT("Can only run this on started clients", m_socket);
  m_socket->send(message, zmq::send_flags::none);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
int ZMQClient::pollSocketVector(const std::vector<zmq::socket_t*>& socketList, int timeout)
{
  auto start = std::chrono::system_clock::now();
  int return_bitmask = 0;
  assert(socketList.size() <= 2);
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
    } catch (zmq::error_t& error) {
      if (error.num() == EINTR) {
        auto now = std::chrono::system_clock::now();
        timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
      } else {
        // cannot handle, rethrow exception
        throw;
      }
    }
  }
  return 0;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

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
