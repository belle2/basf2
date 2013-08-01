/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTSender.h>

using namespace Belle2;

/// @brief HLTSender constructor
/// @param destination IP address for the host
/// @param port Port number for data communication
HLTSender::HLTSender(const std::string destination, unsigned int port)
{
  m_destination = destination;
  m_port = port;

  m_buffer = NULL;
}

/// @brief HLTSender destructor
HLTSender::~HLTSender()
{
}

/// @brief Initialize the HLTSender
/// @return c_Success Initialization done
/// @return c_InitFailed No port number is assigned or creation of a socket failed
EHLTStatus HLTSender::init()
{
  if (m_port == 0) {
    B2ERROR("[HLTSender] No node information received!");
    return c_InitFailed;
  } else {
    if (create() != c_Success)
      return c_InitFailed;

    return c_Success;
  }
}

/// @brief Create a static connection to assigned host. If it fails to connect, try again after 1 sec
/// @return c_Success Creation of connection succeeded
/// @return c_InitFailed Socket initialization failed
EHLTStatus HLTSender::createConnection()
{
  EHLTStatus status = c_InitFailed;
  if (init() != c_Success) {
    B2ERROR("[HLTSender] Initializing the connection failed!");
    return c_InitFailed;
  }
  while (status != c_Success) {
    status = connect(m_destination, m_port);
    usleep(100);
  }

  B2INFO("[HLTSender] \x1b[34mConnection established to " << m_destination
         << " (" << m_port << ")\x1b[0m");

  return c_Success;
}

/// @brief Send data to the assigned node
/// @return c_Success Sending data succeeded or no data to be sent
/// @return c_TermCalled Termination requested
EHLTStatus HLTSender::broadcasting()
{
  if (m_buffer->numq() <= 0) {
    return c_Success;
  }
  B2DEBUG(100, "[HLTSender] Start to send data to " << m_destination);

  B2DEBUG(100, "[HLTSender] \x1b[34mSet buffer size = " << m_tempBufferSize + gEOSTag.size() << "\x1b[0m");
  char* temp = new char [m_tempBufferSize + gEOSTag.size()];

  int bufferStatus = 0;
  while ((bufferStatus = m_buffer->remq((int*)temp)) <= 0) {
    usleep(100);
  }
  B2DEBUG(100, "[HLTSender] \x1b[34mdata size = " << bufferStatus * 4 << "\x1b[0m");

  EHLTStatus status = sendData(temp, bufferStatus * 4);

  if (status == c_TermCalled) {
    while (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
      usleep(100);
    }
    return c_TermCalled;
  }

  B2DEBUG(100, "[HLTSender] Data sent done, no termination...");

  delete[] temp;

  return c_Success;
}

/// @brief Send a simple string to the assigned node
/// @param data String data to be sent
/// @return c_Success Sending succeeded
/// @return c_FuncError Sending failed
EHLTStatus HLTSender::broadcasting(std::string data)
{
  sendData((char*)data.c_str(), data.size());

  return c_Success;
}

/// @brief Send serialized data
/// @param data Serialized data
/// @param size Size of the data to be sent
/// @return c_FuncError Sending data failed
/// @return c_TermCalled Termination requested
/// @return c_Success Successful transfer
EHLTStatus HLTSender::sendData(char* data, int size)
{
  int sizeSent;
  if (send((char*)&size, sizeof(size), sizeSent) == c_FuncError)
    return c_FuncError;

  sizeSent = 0;

  if (send(data, size, sizeSent) == c_FuncError)
    return c_FuncError;
  else {
    B2DEBUG(100, "[HLTSender] \x1b[34mData sent (size = " << sizeSent << ")\x1b[0m");
  }

  if (!strcmp(data, gTerminate.c_str()))
    return c_TermCalled;

  sizeSent = 0;

  return c_Success;
}

/// @brief Set buffer for the data communication with predefined buffer key
/// @return c_Success Set buffer done
EHLTStatus HLTSender::setBuffer()
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for the data communication with specific integer buffer key
/// @return c_Success Set buffer done
EHLTStatus HLTSender::setBuffer(unsigned int key)
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for the data communication with specific string buffer key
/// @return c_Success Set buffer done
EHLTStatus HLTSender::setBuffer(std::string key)
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key.c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set node type
/// @param nodeType Node type
void HLTSender::setMode(EHLTNodeType nodeType)
{
  // Control flow doesn't require much memory while data flow does
  // so the size of buffer has to be treated differently
  if (nodeType == c_ManagerNode) {
    m_tempBufferSize = gControlMaxReceives;
  } else {
    m_tempBufferSize = gDataMaxReceives;
  }
}

/// @brief Set port number for data transfer
/// @param port Port number for data transfer
/// @return c_Success Success
EHLTStatus HLTSender::setPort(int port)
{
  m_port = port;

  return c_Success;
}

/// @brief Encode a simple string
/// @param data Data to be encoded
/// @return Encoded data
/// Obsolete?
std::string HLTSender::makeSingleton(std::string data)
{
  std::string tempMessage(data);
  tempMessage += gEOSTag;

  return tempMessage;
}

/// @brief Encode a general data
/// @param data Data to be encoded
/// @param size Size of the data
/// @return c_Success Encoding succeeded
/// Obsolete?
EHLTStatus HLTSender::makeSingleton(char* data, int size)
{
  memcpy(data + sizeof(char) * size, gEOSTag.c_str(), sizeof(char) * gEOSTag.size());

  return c_Success;
}
