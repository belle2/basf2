/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogConnectionUDP.h>
#include <framework/logging/LogMessage.h>

using namespace boost::asio;
using boost::asio::ip::udp;

using namespace Belle2;

LogConnectionUDP::LogConnectionUDP(const std::string& hostname, unsigned short port) : m_socket(m_ioservice)
{
  ip::udp::resolver resolver(m_ioservice);
  ip::udp::resolver::query udpquery(udp::v4(), hostname, std::to_string(port));

  m_socket.open(ip::udp::v4());
  /// if the hostname can not be resolved, this will throw an exception so dereferencing the result is always safe
  m_remoteEndpoint = *resolver.resolve(udpquery);
}

bool LogConnectionUDP::isConnected()
{
  return true;
}

bool LogConnectionUDP::sendMessage(const LogMessage& message)
{
  const std::string& jsonMessage = message.toJSON(true);

  boost::system::error_code err;
  m_socket.send_to(buffer(jsonMessage, jsonMessage.size()), m_remoteEndpoint, 0, err);
  const bool sent = err.value() != 0;
  return sent;
}

LogConnectionUDP::~LogConnectionUDP()
{
  m_socket.close();
}
