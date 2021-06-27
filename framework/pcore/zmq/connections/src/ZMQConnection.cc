/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/connections/ZMQConnection.h>

#include <string>
#include <memory>

using namespace Belle2;

bool ZMQConnection::isReady() const
{
  return true;
}

bool ZMQConnection::hasMessage(const ZMQConnection* connection)
{
  // Just poll with 0 timeout and no reaction function. Hacky trick to reduce code duplication
  const auto emptyFunction = []() {};
  return ZMQConnection::poll({{connection, emptyFunction}}, 0);
}

bool ZMQConnection::poll(const std::map<const ZMQConnection*, ZMQConnection::ReactorFunction>& connectionList, int timeout)
{
  std::vector<const ReactorFunction*> socketMapping;
  std::vector<zmq::pollitem_t> pollItems;

  // zmq needs a special format for its polling, so create it here.
  for (const auto& [connection, function] : connectionList) {
    auto sockets = connection->getSockets();
    for (zmq::socket_t* socket : sockets) {
      zmq::pollitem_t pollItem;
      pollItem.socket = static_cast<void*>(*socket);
      pollItem.events = ZMQ_POLLIN;
      pollItem.revents = 0;
      pollItems.push_back(std::move(pollItem));

      // but keep reference to the original function, so we can call the correct one later
      socketMapping.push_back(&function);
    }
  }

  if (pollItems.empty()) {
    return false;
  }

  try {
    zmq::poll(&pollItems[0], pollItems.size(), timeout);

    bool anySocket = false;
    unsigned int counter = 0;
    for (const auto& pollItem : pollItems) {
      if (pollItem.revents & ZMQ_POLLIN) {
        anySocket = true;
        const auto* functionPtr = socketMapping.at(counter);
        const auto function = *functionPtr;
        function();
      }
      counter++;
    }

    return anySocket;
  } catch (zmq::error_t& error) {
    if (error.num() == EINTR) {
      // Could happen if there was an interrupt, return false so the caller knows the time did not pass already
      return false;
    } else {
      // cannot handle, rethrow exception
      throw;
    }
  }
}

ZMQConnectionOverSocket::ZMQConnectionOverSocket(const std::shared_ptr<ZMQParent>& parent) : m_parent(parent)
{
}

std::vector<zmq::socket_t*> ZMQConnectionOverSocket::getSockets() const
{
  return {m_socket.get()};
}

std::string ZMQConnectionOverSocket::getEndPoint() const
{
  std::string endpoint{""};
  if (m_socket) {
    endpoint = m_socket->get(zmq::sockopt::last_endpoint);
  }
  return endpoint;
}
