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
  if (init() != c_Success)
    return c_InitFailed;
  while (status != c_Success) {
    status = connect(m_destination, m_port);
    sleep(1);
  }

  B2INFO("\x1b[34m[HLTSender] Connection established to " << m_destination
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
  //B2INFO ("[HLTSender] Start to send data to " << m_destination);

  char* temp = new char [gMaxReceives + gEOSTag.size()];
  memset(temp, 0, sizeof(char) * (gMaxReceives + gEOSTag.size()));

  int bufferStatus = 0;
  while ((bufferStatus = m_buffer->remq((int*)temp)) <= 0) {
    usleep(100);
  }

  bool termCode = false;

  //writeFile("test.txt", temp, bufferStatus * 4);
  std::string termChecker(temp);
  if (termChecker == gTerminate) {
    B2INFO("\x1b[032m[HLTSender] Termination code taken!\x1b[0m");
    if (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
      usleep(100);
    }
    termCode = true;

    while (broadcasting(gTerminate) == c_FuncError) {
      B2INFO("[HLTSender] \x1b[31mAn error occurred in sending termination. Retrying...\x1b[0m");
      //usleep(100);
      sleep(1);
    }
    B2INFO("[HLTSender] Sending terminate to " << m_destination << " \x1b[034msuccess!\x1b[0m");
    return c_TermCalled;
  }

  int size = bufferStatus * 4;

  makeSingleton(temp, size);
  size += gEOSTag.size();

  if (!termCode) {
    B2INFO("[HLTSender] Sending events to " << m_destination << "...");
    while (send(temp, size) == c_FuncError) {
      B2INFO("[HLTSender] \x1b[31mAn error occurred in sending data. Retrying...\x1b[0m");
      //usleep(100);
      sleep(1);
    }
  }
  /*
  else {
    B2INFO ("[HLTSender] Sending termination to " << m_destination << "...");
    while (broadcasting(gTerminate) == c_FuncError) {
      B2INFO("[HLTSender] \x1b[31mAn error occurred in sending termination. Retrying...\x1b[0m");
      //usleep(100);
      sleep (1);
    }
    return c_TermCalled;
  }
  */

  B2INFO("[HLTSender] Sending events to " << m_destination << " \x1b[034msuccess!\x1b[0m");
  return c_Success;
}

/// @brief Send a simple string to the assigned node
/// @param data String data to be sent
/// @return c_Success Sending succeeded
/// @return c_FuncError Sending failed
EHLTStatus HLTSender::broadcasting(std::string data)
{
  int size = 0;

  std::string sendingMessage = makeSingleton(data);

  if (send(sendingMessage, size) == c_FuncError)
    return c_FuncError;
  else {
    B2INFO("[HLTSender] \x1b[34mData " << sendingMessage << " (size=" << sendingMessage.size()
           << ") has been sent to " << m_destination << "\x1b[0m");
    return c_Success;
  }
}

/// @brief Set buffer for the data communication with predefined buffer key
/// @return c_Success Set buffer done
EHLTStatus HLTSender::setBuffer()
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for the data communication with specific buffer key
/// @return c_Success Set buffer done
EHLTStatus HLTSender::setBuffer(unsigned int key)
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

EHLTStatus HLTSender::setBuffer(std::string key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key.c_str(), gBufferSize);

  return c_Success;
}

/// @brief Encode a simple string
/// @param data Data to be encoded
/// @return Encoded data
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
EHLTStatus HLTSender::makeSingleton(char* data, int size)
{
  memcpy(data + sizeof(char) * size, gEOSTag.c_str(), sizeof(char) * gEOSTag.size());

  return c_Success;
}

/// @brief Write a data into a file (development purpose only)
/// @param file File name
/// @param data Data to be written
/// @param size Size of the data
void HLTSender::writeFile(char* file, char* data, int size)
{
  FILE* fp;
  fp = fopen(file, "a");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fprintf(fp, "\n");
  fclose(fp);
}
