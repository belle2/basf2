/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/logging/LogConnectionBase.h>
#include <boost/asio.hpp>

namespace Belle2 {
  /**
   * Log Connection to send the log message as JSON to a UDP server
   */
  class LogConnectionUDP final: public LogConnectionBase {
  public:
    /// Create a new UDP log connection with the hostname (as name or IP) and the port to connect to
    LogConnectionUDP(const std::string& hostname, unsigned short port);
    /// Close the socket on destruction
    ~LogConnectionUDP() override;
    /// Send the log message as JSON to the UDP server
    bool sendMessage(const LogMessage& message) final;
    /// there is no way to check if a UDP connection is fine, so we just return True always
    bool isConnected() final;
  private:
    /// asio service for handling the requests
    boost::asio::io_service m_ioservice;
    /// the socket to the UDP server
    boost::asio::ip::udp::socket m_socket;
    /// the remote endpoint we send to
    boost::asio::ip::udp::endpoint m_remoteEndpoint;
  };
}
