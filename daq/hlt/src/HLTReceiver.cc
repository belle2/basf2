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
}

/// @brief HLTReceiver destructor
HLTReceiver::~HLTReceiver()
{
  //delete m_internalBuffer;
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

  m_internalBuffer = (char*)malloc(sizeof(char) * gBufferSize);
  flushInternalBuffer();

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

/// @brief Accepting data from assigned port
/// @return c_Success Receiving data done
/// @return c_InitFailed Accepting socket failed
/// @return c_FuncError Buffer wasn't set
/// @return c_TermCalled Termination requested
EHLTStatus HLTReceiver::listening()
{
  if (m_buffer == NULL) {
    B2ERROR("[HLTReceiver] Buffer wasn't set!");
    return c_FuncError;
  }
  EHLTStatus status = c_Success;

  int newSocket = 0;
  if (accept(newSocket) != c_Success)
    return c_InitFailed;

  while (1) {
    int size = 0;

    char* data = new char [gMaxReceives];
    while ((status = receive(newSocket, data, size)) != c_FuncError) {
      std::vector<int> givenMessageSizes;
      char* givenMessages = new char [gBufferSize];
      givenMessageSizes.clear();
      memset(givenMessages, 0, sizeof(char) * gBufferSize);

      if (size > 0) {
        std::string termChecker(data);
        if (termChecker == gTerminate) {
          B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
          if (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
            usleep(100);
          }
          return c_TermCalled;
        }

        int readContents = decodeSingleton(data, (int)size, givenMessages, givenMessageSizes);

        int givenMessageIndex = 0;
        for (int i = 0; i < readContents; i++) {
          char* givenMessage = new char [gMaxReceives];
          memset(givenMessage, 0, sizeof(char) * gMaxReceives);
          if (i == 0)
            memcpy(givenMessage, givenMessages, sizeof(char) * givenMessageSizes[i]);
          else
            memcpy(givenMessage, givenMessages + sizeof(char) * givenMessageIndex, givenMessageSizes[i]);

          givenMessageIndex += givenMessageSizes[i];
          if (!strcmp(givenMessage, gTerminate.c_str())) {
            B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
            if (m_buffer->insq((int*)gTerminate.c_str(), gTerminate.size() / 4 + 1) <= 0) {
              usleep(100);
            }
            return c_TermCalled;
            /*
            m_nSources--;
            if (m_nSources == 0) {
              B2INFO ("[HLTReceiver] All sources down. Terminating...");
              return c_TermCalled;
            }
            else {
              B2INFO ("[HLTReceiver] A source down. " << m_nSources << " more sources left.");
              return c_Success;
            }
            */
          } else {
            while (m_buffer->insq((int*)givenMessage,
                                  givenMessageSizes[i] / 4 + 1) <= 0) {
              usleep(100);
            }
          }

          delete givenMessage;
        }
      }

      delete givenMessages;
    }

    delete data;
  }

  return c_Success;
}

/// @brief Set buffer for data communication with predefined buffer key
/// @return c_Success Set buffer succeeded
EHLTStatus HLTReceiver::setBuffer()
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

/// @brief Set buffer for data communication with a specific buffer key
/// @param key Key value for the buffer
/// @return c_Success Set buffer succeeded
EHLTStatus HLTReceiver::setBuffer(unsigned int key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

EHLTStatus HLTReceiver::setBuffer(std::string key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key.c_str(), gBufferSize);

  return c_Success;
}

/// @brief Decode received data
/// @param data Taken data
/// @param size Size of the taken data
/// @param container Container for decoded data (the data can be multiple)
/// @param sizes Container for size of decoded data
/// @return The number of decoded data (this might be redundant)
int HLTReceiver::decodeSingleton(char* data, int size, char* container, std::vector<int>& sizes)
{
  int containerIndex = 0;
  int eos = findEOS(data, size);

  if (eos < 0) {
    if (size < gBufferSize - m_internalBufferWriteIndex) {
      memcpy(m_internalBuffer + sizeof(char) * m_internalBufferWriteIndex, data, sizeof(char) * size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;

      return 0;
    } else {
      B2ERROR("[HLTReceiver] Internal buffer is full!");
      return -1;
    }
  } else {
    if (m_internalBufferEntries > 0) {
      memcpy(container, m_internalBuffer, sizeof(char) * m_internalBufferWriteIndex);
      memcpy(container + sizeof(char) * m_internalBufferWriteIndex, data, sizeof(char) * eos);
      sizes.push_back(m_internalBufferWriteIndex + eos);
      containerIndex += m_internalBufferWriteIndex + eos;

      flushInternalBuffer();

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
    } else {
      memcpy(container, data, sizeof(char) * eos);
      sizes.push_back(eos);
      containerIndex += eos;

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
    }

    if (size <= 0)
      return sizes.size();

    while ((eos = findEOS(data, size)) > 0) {
      memcpy(container + sizeof(char) * containerIndex, data, sizeof(char) * eos);
      sizes.push_back(eos);
      containerIndex += eos;

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
    }

    if (size > 0) {
      memcpy(m_internalBuffer, data, sizeof(char) * size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;
    }

    return sizes.size();
  }

  return 0;
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

/// @brief Flush internal buffer that is supposed to contain part of data
/// @return c_Success The internal buffer is flushed
EHLTStatus HLTReceiver::flushInternalBuffer()
{
  memset(m_internalBuffer, 0, gBufferSize);
  m_internalBufferWriteIndex = 0;
  m_internalBufferEntries = 0;

  return c_Success;
}

/// @brief Write a data into a file (development purpose only)
/// @param file File name
/// @param data Data to be written
/// @param size Size of the data
void HLTReceiver::writeFile(char* file, char* data, int size)
{
  FILE* fp;
  fp = fopen(file, "a");
  fprintf(fp, "[Start a run] ");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fprintf(fp, " [End a run]\n");
  fclose(fp);
}
