#include <daq/hlt/HLTReceiver.h>

using namespace Belle2;

HLTReceiver::HLTReceiver(unsigned int port)
{
  m_port = port;
  m_buffer = NULL;
}

HLTReceiver::~HLTReceiver()
{
  delete m_internalBuffer;
}

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

  //while (status == c_Success) {
  while (1) {
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

        std::string termChecker(data);
        if (termChecker == gTerminate) {
          B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
          return c_TermCalled;
        }

        B2INFO("[HLTReceiver] " << size << " bytes come in");
        int readContents = decodeSingleton(data, (int)size, givenMessages, givenMessageSizes);
        B2INFO("\x1b[32m[HLTReceiver] " << readContents << " data taken!\x1b[0m");

        for (int i = 0; i < readContents; i++) {
          B2INFO("[HLTReceiver] taken " << i << ": (" << givenMessages[i] << ") " << givenMessageSizes[i] << " bytes");
          if (!strcmp(givenMessages[i], gTerminate.c_str())) {
            B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
            return c_TermCalled;
          } else {
            writeFile("receiverAfterDecode", givenMessages[i], givenMessageSizes[i]);
            while (m_buffer->insq((int*)givenMessages[i],
                                  givenMessageSizes[i] / 4 + 1) <= 0) {
              usleep(100);
            }
          }
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

int HLTReceiver::decodeSingleton(char* data, int size, char** container, int* sizes)
{
  for (int i = 0; i < gBufferArray; i++) {
    memset(container[i], 0, sizeof(char) * gMaxReceives);
    sizes[i] = 0;
  }
  int containerPointer = 0;

  int eos = findEOS(data, size);

  if (eos < 0) {
    B2INFO("\x1b[033m[HLTReceiver] EOS not found. Saving to internal buffer...\x1b[0m");
    if (size < gBufferSize - m_internalBufferWriteIndex) {
      memcpy(m_internalBuffer + sizeof(char) * m_internalBufferWriteIndex, data, size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;
      B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes saved in the internal buffer...\x1b[0m");

      return 0;
    } else {
      B2ERROR("[HLTReceiver] Internal buffer is full!");
      return -1;
    }
  } else {
    B2INFO("\x1b[033m[HLTReceiver] EOS found. Checking internal buffer...\x1b[0m");
    if (m_internalBufferEntries > 0) {
      B2INFO("[HLTReceiver] " << m_internalBufferEntries << " chunks ("
             << m_internalBufferWriteIndex << " bytes) are ready to be mergied which is " << m_internalBuffer);

      memcpy(container[containerPointer], m_internalBuffer, m_internalBufferWriteIndex);
      B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes written from internal buffer...\x1b[0m");
      memcpy(container[containerPointer] + sizeof(char) * m_internalBufferWriteIndex, data, eos);
      B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");

      sizes[containerPointer] = m_internalBufferWriteIndex + eos;
      containerPointer++;

      flushInternalBuffer();

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
      B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    } else {
      B2INFO("\x1b[033m[HLTReceiver] No contents in the internal buffer\x1b[0m");
      memcpy(container[containerPointer], data, eos);
      sizes[containerPointer] = eos;
      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
      containerPointer++;
      B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");
      B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    }

    if (size <= 0)
      return containerPointer;

    while ((eos = findEOS(data, size)) > 0) {
      B2INFO("\x1b[033m[HLTReceiver] More EOS found. Storing it...\x1b[0m");
      memcpy(container[containerPointer], data, eos);
      sizes[containerPointer] = eos;
      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
      containerPointer++;
      B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");
      B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    }

    if (size > 0) {
      B2INFO("\x1b[033m[HLTReceiver] Data still remains without EOS. Saving it into the internal buffer...\x1b[0m");
      memcpy(m_internalBuffer, data, size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;
      B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes saved in the internal buffer...\x1b[0m");
    }

    return containerPointer;
  }

  return 0;
}

int HLTReceiver::findEOS(char* data, int size)
{
  if (size > 0) {
    for (unsigned int i = 0; i < size - gEOSTag.size() + 1; i++) {
      if (!memcmp(data + sizeof(char) * i, gEOSTag.c_str(), gEOSTag.size())) {
        return i;
      }
    }
  }

  return -1;
}

EHLTStatus HLTReceiver::flushInternalBuffer()
{
  memset(m_internalBuffer, 0, gBufferSize);
  m_internalBufferWriteIndex = 0;
  m_internalBufferEntries = 0;

  return c_Success;
}

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
