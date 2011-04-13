/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/EvtSender.h>

using namespace Belle2;

/// @brief EvtSender constructor
///        Creates and connects to the socket
/// @param host Host IP address to send data
/// @param port Port number for the communication
EvtSender::EvtSender(std::string host, int port)
{
  m_host = host;
  m_port = port;
}

/// @brief EvtSender destructor
EvtSender::~EvtSender()
{
}

EStatus EvtSender::init()
{
  //B2INFO("Initializing EvtSender...");

  if (B2Socket::create() != c_Success) {
    B2ERROR("Could not create sending socket.");
    return c_InitFailed;
  }

  return c_Success;
}

EStatus EvtSender::init(HLTBuffer* buffer)
{
  m_buffer = buffer;
  return init();
}

void EvtSender::setDestination(std::string dest)
{
  m_host = dest;
}

EStatus EvtSender::connect()
{
  if (m_port != 35000)
    B2INFO("EvtSender: Trying to connect to " << m_host << ":" << m_port);
  if (B2Socket::connect(m_host, m_port) != c_Success) {
    if (m_port != 35000)
      B2ERROR("Unable to connect to the destination (" << m_host << ":" << m_port << ")");
    return c_FuncError;
  } else {
    B2INFO("EvtSender: Connection to " << m_host << ":" << m_port << " established!");
    return c_Success;
  }
}

EStatus EvtSender::broadCasting()
{
  if (m_buffer->numq() > 0) {
    if (connect() == c_Success) {
      char* tmp = new char[MAXPACKETSIZE];
      m_buffer->remq((int*)tmp);
      std::string input(tmp);

      if (B2Socket::send(input) == c_Success) {
        if (input == "EOF") {
          B2INFO("EvtSender: EOF taken");
          return c_TermCalled;
        } else {
          B2INFO("EvtSender: Sending data to " << m_host << " size=" << input.size());
          return c_Success;
        }
      } else {
        B2ERROR("Sending data failed");
        // If error occurs in data sending, put back the data into the ring buffer
        m_buffer->insq((int*)(input.c_str()), input.size());
        return c_FuncError;
      }
    } else {
      return c_FuncError;
    }
  }

  return c_Success;
}

/// @brief EvtSender << operator
///        Sends string to socket
/// @param s String to send
const EvtSender& EvtSender::operator << (const std::string& s) const
{
  if (B2Socket::send(s) != c_Success)
    throw B2SocketException("Could not write to socket.");

  return *this;
}

/// @brief EvtSender >> operator
///        Retrieves string from socket
/// @param s Buffer to take string from socket
const EvtSender& EvtSender::operator >> (std::string& s) const
{
  if (!B2Socket::recv(s))
    throw B2SocketException("Could not read from socket.");

  return *this;
}
