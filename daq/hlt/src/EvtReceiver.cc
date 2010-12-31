/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cstdlib>

#include <daq/hlt/EvtReceiver.h>
#include <daq/hlt/B2SocketException.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/// @brief EvtReceiver class constructor
///        Creates, binds, and listens the socket
/// @param port Port number for the communication
EvtReceiver::EvtReceiver(int port)
    : m_port(port)
{
}

void EvtReceiver::init()
{
  if (!B2Socket::create()) {
    B2FATAL("Could not create receiving socket.");
    //throw B2SocketException("Could not create receiving socket.");
  }

  if (!B2Socket::bind(m_port))
    B2FATAL("Could not bind to port " << m_port);
  //throw B2SocketException("Could not bind to port.");

  /*
  if(!B2Socket::listen())
          throw B2SocketException("Could not listen to socket.");
          */
}

void EvtReceiver::init(RingBuffer* buffer)
{
  init();
  m_buffer = buffer;
}

void EvtReceiver::listen()
{
  if (!B2Socket::listen())
    B2FATAL("Could not listen to socket.");
  //throw B2SocketException ("Could not listen to socket.");
}

/// @brief EvtReceiver class destructor
EvtReceiver::~EvtReceiver()
{
}

/// @brief EvtReceiver << operator
///        Sends string to socket
/// @param s String to send
const EvtReceiver& EvtReceiver::operator << (const std::string& s) const
{
  if (!B2Socket::send(s))
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
void EvtReceiver::accept(EvtReceiver& sock)
{
  if (!B2Socket::accept(sock))
    throw B2SocketException("Could not accept socket.");
}
