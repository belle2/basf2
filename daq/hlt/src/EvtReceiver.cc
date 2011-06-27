/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/EvtReceiver.h>

using namespace Belle2;

/// @brief EvtReceiver class constructor
///        Creates, binds, and listens the socket
/// @param port Port number for the communication
EvtReceiver::EvtReceiver(int port)
    : m_port(port)
{
}

/// @brief EvtReceiver class destructor
EvtReceiver::~EvtReceiver()
{
}

EStatus EvtReceiver::init()
{
  if (B2Socket::create() != c_Success) {
    B2ERROR("Could not create receiving socket.");
    return c_InitFailed;
  }

  if (B2Socket::bind(m_port) != c_Success) {
    B2ERROR("Could not bind to port " << m_port);
    return c_InitFailed;
  }

  return c_Success;
}

EStatus EvtReceiver::init(HLTBuffer* buffer)
{
  m_buffer = buffer;
  return init();
}

EStatus EvtReceiver::listen()
{
  if (B2Socket::listen() != c_Success) {
    B2ERROR("Could not listen to socket.");
    return c_FuncError;
  }

  return c_Success;
}

int EvtReceiver::recv(char* data)
{
  int receivedSize = B2Socket::recv(data);
  B2INFO("[EvtReceiver] " << receivedSize << " bytes received");
  B2INFO("[EvtReceiver] " << data << " received");

  return receivedSize;
}

/// @brief EvtReceiver << operator
///        Sends string to socket
/// @param s String to send
const EvtReceiver& EvtReceiver::operator << (const std::string& s) const
{
  if (B2Socket::send(s) != c_Success)
    throw B2SocketException("Could not write to socket.");

  return *this;
}

/// @brief EvtReceiver >> operator
///        Retrieves string from socket
/// @param s Buffer to take string from socket
const EvtReceiver& EvtReceiver::operator >> (std::string& s) const
{
  if (!B2Socket::recv(s))
    throw(B2SocketException("Could not read from socket."));

  return *this;
}

/// @brief Accepts socket
/// @param sock EvtReceiver to accept communication
EStatus EvtReceiver::accept(EvtReceiver& sock)
{
  if (B2Socket::accept(sock) != c_Success) {
    B2ERROR("Data transmission hasn't been accepted.");
    return c_FuncError;
  }

  return c_Success;
}
