#include <daq/hlt/HLTReceiver.h>

using namespace Belle2;

HLTReceiver::HLTReceiver(unsigned int port)
{
  m_port = port;
  m_buffer = NULL;
}

HLTReceiver::~HLTReceiver()
{
}

EHLTStatus HLTReceiver::init()
{
  if (create() != c_Success)
    return c_InitFailed;

  if (bind(m_port) != c_Success)
    return c_InitFailed;

  flushInternalBuffer();

  return c_Success;
}

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

  while (status == c_Success) {
    int givenMessageSizes[gBufferArray];
    char** givenMessages;
    givenMessages = (char**)malloc(sizeof(char*) * gBufferArray);
    for (int i = 0; i < gBufferArray; i++) {
      givenMessageSizes[i] = 0;
      givenMessages[i] = (char*)malloc(sizeof(char) * gMaxReceives);
    }

    int size = 0;

    char data[gMaxReceives];
    while ((status = receive(newSocket, data, size)) != c_FuncError) {
      if (size > 0) {
        writeFile("receiver", data, size);
        decodeSingleton(data, (int)size, givenMessages, givenMessageSizes);

        int givenMessageIndex = 0;
        while (givenMessageSizes[givenMessageIndex] > 0) {
          B2INFO("[HLTReceiver] Processing " << givenMessages[givenMessageIndex]);
          if (!strcmp(givenMessages[givenMessageIndex], gTerminate.c_str())) {
            B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
            writeFile("receiverAfterDecode", givenMessages[givenMessageIndex], givenMessageSizes[givenMessageIndex]);
            while (m_buffer->insq((int*)givenMessages[givenMessageIndex],
                                  givenMessageSizes[givenMessageIndex] / 4 + 1) <= 0) {
              usleep(100);
            }
            return c_TermCalled;
          } else {
            writeFile("receiverAfterDecode", givenMessages[givenMessageIndex], givenMessageSizes[givenMessageIndex]);
            while (m_buffer->insq((int*)givenMessages[givenMessageIndex],
                                  givenMessageSizes[givenMessageIndex] / 4 + 1) <= 0) {
              usleep(100);
            }
          }
          givenMessageIndex++;

          if (givenMessageIndex > gBufferArray)
            return c_Success;
        }
      }
    }
  }

  return c_Success;
}

EHLTStatus HLTReceiver::setBuffer()
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

EHLTStatus HLTReceiver::setBuffer(unsigned int key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

EHLTStatus HLTReceiver::decodeSingleton(char* data, int size, char** container, int* sizes)
{
  for (int i = 0; i < gBufferArray; i++) {
    memset(container[i], 0, sizeof(char) * gMaxReceives);
    sizes[i] = 0;
  }

  int eos = findEOS(data, size);

  if (eos < 0) {
    if (m_internalBufferEntries < gBufferArray) {
      memcpy(m_internalBuffer[m_internalBufferWriteIndex], data, size);
      m_internalBufferSizes[m_internalBufferWriteIndex] = size;

      if (m_internalBufferWriteIndex == gBufferArray - 1)
        m_internalBufferWriteIndex = 0;
      else
        m_internalBufferWriteIndex++;

      m_internalBufferEntries++;

      return c_FuncError;
    } else {
      B2ERROR("[HLTReceiver] Internal buffer is full!");
      return c_FuncError;
    }
  } else {
    int containerPointer = 0;

    if (m_internalBufferEntries > 0) {
      B2INFO("[HLTReceiver] Internal buffer exists!");
      int tempPosition = 0;
      for (int i = 0; i < m_internalBufferEntries; i++) {
        memcpy(container[containerPointer] + sizeof(char) * tempPosition,
               m_internalBuffer[i], m_internalBufferSizes[i]);
        tempPosition += m_internalBufferSizes[i];
      }
      flushInternalBuffer();

      memcpy(container[containerPointer] + sizeof(char) * tempPosition,
             data, eos);
      sizes[containerPointer] = tempPosition + eos;
      containerPointer++;
      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - sizes[containerPointer] - gEOSTag.size();
    } else {
      memcpy(container[containerPointer], data, eos);
      sizes[containerPointer] = eos;
      containerPointer++;
      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - sizes[containerPointer] - gEOSTag.size();

      if (size <= 0)
        return c_Success;

      while ((eos = findEOS(data, size)) > 0) {
        memcpy(container[containerPointer], data, eos);
        sizes[containerPointer] = eos;
        containerPointer++;
        data = data + sizeof(char) * (eos + gEOSTag.size());
        size = size - sizes[containerPointer] - gEOSTag.size();

        if (size == 0)
          return c_Success;
      }

      if (size > 0) {
        memcpy(m_internalBuffer[m_internalBufferWriteIndex], data, size);
        m_internalBufferSizes[m_internalBufferWriteIndex] = size;

        if (m_internalBufferWriteIndex == gBufferArray - 1)
          m_internalBufferWriteIndex = 0;
        else
          m_internalBufferWriteIndex++;

        m_internalBufferEntries++;

        return c_FuncError;
      }
    }
  }

  return c_Success;
}

int HLTReceiver::findEOS(char* data, int size)
{
  int eos = 0;
  char* eosTag = (char*)gEOSTag.c_str();

  for (int i = 0; i < size - gEOSTag.size() + 1; i++) {
    if (!memcmp(data + sizeof(char) * i, gEOSTag.c_str(), gEOSTag.size())) {
      return i;
    }
  }
  return -1;
}

EHLTStatus HLTReceiver::flushInternalBuffer()
{
  for (int i = 0; i < gBufferArray; i++) {
    memset(m_internalBuffer[i], 0, gMaxReceives);
    m_internalBufferSizes[i] = 0;
  }

  m_internalBufferWriteIndex = 0;
  m_internalBufferEntries = 0;

  return c_Success;
}

void HLTReceiver::writeFile(char* file, char* data, int size)
{
  FILE* fp;
  fp = fopen(file, "a");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fclose(fp);
}
