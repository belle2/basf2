/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTReceiver.h>

using namespace Belle2;

/// @brief HLTReceiver constructor
/// @param port Port number for data communication
HLTReceiver::HLTReceiver(unsigned int port, unsigned int nSources)
{
  m_port = port;
  m_nSources = nSources;
  m_buffer = NULL;
  m_tempBufferSize = 0;
}

/// @brief HLTReceiver destructor
HLTReceiver::~HLTReceiver()
{
}

/// @brief Initialize the HLTReceiver
/// @return c_Success Initialization done
/// @return c_InitFailed Initialization failed
EHLTStatus HLTReceiver::init()
{
  if (create() != c_Success)
    return c_InitFailed;

  if (bind(m_port) != c_Success)
    return c_InitFailed;

  return c_Success;
}

/// @brief Create a static connection through the assigned port
/// @return c_Success Connection established
/// @return c_InitFailed Initialization of socket or starting to listen falied
EHLTStatus HLTReceiver::createConnection()
{
  if (init() != c_Success)
    return c_InitFailed;

  if (listen() != c_Success)
    return c_InitFailed;

  B2INFO("\x1b[34m[HLTReceiver] Connection established through " << m_port
         << "\x1b[0m");

  return c_Success;
}

/// @brief Listening data from source
/// @return c_InitFailed if socket rejected
/// @return c_Success is listening success
EHLTStatus HLTReceiver::listening()
{
  int newSocket = 0;

  if (accept(newSocket) != c_Success)
    return c_InitFailed;

  int sizeToReceive = 0;
  int sizeToReceiveSize = 0;

  int size = 0;
  int totalSize = 0;

  // TCP provides the length of data but it transfers sometimes separately so it should wait for remaining
  // data if it wasn't completely delivered
  while (receive(newSocket, (char*)&sizeToReceive, sizeof(sizeToReceive), sizeToReceiveSize)) {
    B2DEBUG(150, "....." << sizeToReceive << " bytes to be received");

    char* data = new char [sizeToReceive];

    receive(newSocket, data, sizeToReceive, size);
    B2DEBUG(150, "[HLTReceiver::listening ()] \x1b[34mData taken size = " << size << "\x1b[0m");
    totalSize += size;
    while (m_buffer->insq((int*)data, size / 4 + 1) <= 0) {
      usleep(100);
    }

    delete[] data;
  }

  B2DEBUG(100, "[HLTReceiver] Data all taken!");

  return c_Success;
}

/// @brief Get data from source
/// @return c_InitFailed if socket rejected
/// @return Total received size, otherwise
int HLTReceiver::getData()
{
  int newSocket = 0;

  if (accept(newSocket) != c_Success)
    return c_InitFailed;

  int sizeToReceive = 0;
  int sizeToReceiveSize = 0;

  int size = 0;
  int totalSize = 0;

  // TCP provides the length of data but it transfers sometimes separately so it should wait for remaining
  // data if it wasn't completely delivered
  while (receive(newSocket, (char*)&sizeToReceive, sizeof(sizeToReceive), sizeToReceiveSize)) {
    B2DEBUG(100, "....." << sizeToReceive << " bytes to be received");

    char* data = new char [sizeToReceive];

    receive(newSocket, data, sizeToReceive, size);
    B2DEBUG(100, "[HLTReceiver] \x1b[34mData taken size = " << size << "\x1b[0m");
    totalSize += size;

    delete[] data;
  }

  B2DEBUG(100, "[HLTReceiver] Data all taken!");

  return totalSize;
}

/// @brief Set buffer for data communication with predefined buffer key
/// @return c_Success Set buffer succeeded
EHLTStatus HLTReceiver::setBuffer()
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for data communication with a specific integer buffer key
/// @param key Key value for the buffer
/// @return c_Success Set buffer succeeded
EHLTStatus HLTReceiver::setBuffer(unsigned int key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for data communication with a specific string buffer key
/// @param key Key value for the buffer
/// @return c_Success Set buffer succeeded
EHLTStatus HLTReceiver::setBuffer(std::string key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key.c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set node type
/// @param nodeType Node type
void HLTReceiver::setMode(EHLTNodeType nodeType)
{
  if (nodeType == c_ManagerNode) {
    m_tempBufferSize = gControlMaxReceives;
  } else {
    m_tempBufferSize = gDataMaxReceives;
  }
}

/// @brief Set port number for data transfer
/// @param port Port number for data transfer
/// @return c_Success for success
EHLTStatus HLTReceiver::setPort(int port)
{
  m_port = port;
  return c_Success;
}

/// @brief Find EOS tag that indicates the end of an individual data
/// @param data Taken data chunk
/// @param size Size of taken data
/// @return -1 No EOS found
/// @return otherwise Position of EOS tag
int HLTReceiver::findEOS(char* data, int size)
{
  if (size > 0) {
    for (unsigned int i = 0; i < size - gEOSTag.size() + 1; i++) {
      if (!memcmp(data + sizeof(char) * i, gEOSTag.c_str(), sizeof(char) * gEOSTag.size())) {
        return i;
      }
    }
  }

  return -1;
}
