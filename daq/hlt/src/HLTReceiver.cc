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

  while (1) {
    int size = 0;

    char* data = new char [gMaxReceives];
    while ((status = receive(newSocket, data, size)) != c_FuncError) {
      std::vector<int> givenMessageSizes;
      char* givenMessages = new char [gBufferSize];
      givenMessageSizes.clear();
      memset(givenMessages, 0, sizeof(char) * gBufferSize);

      if (size > 0) {
        //writeFile("receiver", data, size);

        std::string termChecker(data);
        if (termChecker == gTerminate) {
          B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
          return c_TermCalled;
        }

        //B2INFO("[HLTReceiver] " << size << " bytes come in");
        int readContents = decodeSingleton(data, (int)size, givenMessages, givenMessageSizes);
        //B2INFO("\x1b[32m[HLTReceiver] " << readContents << " data taken!\x1b[0m");

        int givenMessageIndex = 0;
        for (int i = 0; i < readContents; i++) {
          char* givenMessage = new char [gMaxReceives];
          memset(givenMessage, 0, sizeof(char) * gMaxReceives);
          if (i == 0)
            memcpy(givenMessage, givenMessages, sizeof(char) * givenMessageSizes[i]);
          else
            memcpy(givenMessage, givenMessages + sizeof(char) * givenMessageIndex, givenMessageSizes[i]);

          givenMessageIndex += givenMessageSizes[i];
          //B2INFO("[HLTReceiver] taken " << i << ": (" << givenMessage << ") " << givenMessageSizes[i] << " bytes");
          if (!strcmp(givenMessage, gTerminate.c_str())) {
            B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
            return c_TermCalled;
          } else {
            //writeFile("receiverAfterDecode", givenMessage, givenMessageSizes[i]);
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

int HLTReceiver::decodeSingleton(char* data, int size, char* container, std::vector<int>& sizes)
{
  int containerIndex = 0;
  int eos = findEOS(data, size);

  if (eos < 0) {
    //B2INFO("\x1b[033m[HLTReceiver] EOS not found. Saving to internal buffer...\x1b[0m");
    if (size < gBufferSize - m_internalBufferWriteIndex) {
      memcpy(m_internalBuffer + sizeof(char) * m_internalBufferWriteIndex, data, sizeof(char) * size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;
      //B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes saved in the internal buffer...\x1b[0m");

      return 0;
    } else {
      B2ERROR("[HLTReceiver] Internal buffer is full!");
      return -1;
    }
  } else {
    //B2INFO("\x1b[033m[HLTReceiver] EOS found. Checking internal buffer...\x1b[0m");
    if (m_internalBufferEntries > 0) {
      //B2INFO("[HLTReceiver] " << m_internalBufferEntries << " chunks ("
      //       << m_internalBufferWriteIndex << " bytes) are ready to be mergied which is " << m_internalBuffer);

      memcpy(container, m_internalBuffer, sizeof(char) * m_internalBufferWriteIndex);
      //B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes written from internal buffer...\x1b[0m");
      memcpy(container + sizeof(char) * m_internalBufferWriteIndex, data, sizeof(char) * eos);
      sizes.push_back(m_internalBufferWriteIndex + eos);
      containerIndex += m_internalBufferWriteIndex + eos;
      //B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");

      flushInternalBuffer();

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());
      //B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    } else {
      //B2INFO("\x1b[033m[HLTReceiver] No contents in the internal buffer\x1b[0m");
      memcpy(container, data, sizeof(char) * eos);
      sizes.push_back(eos);
      containerIndex += eos;

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());

      //B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");
      //B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    }

    if (size <= 0)
      return sizes.size();

    while ((eos = findEOS(data, size)) > 0) {
      //B2INFO("\x1b[033m[HLTReceiver] More EOS found. Storing it...\x1b[0m");
      //B2INFO("\x1b[033m[HLTReceiver] Writing @" << containerIndex << "\x1b[0m");
      memcpy(container + sizeof(char) * containerIndex, data, sizeof(char) * eos);
      sizes.push_back(eos);
      containerIndex += eos;

      data = data + sizeof(char) * (eos + gEOSTag.size());
      size = size - (eos + gEOSTag.size());

      //B2INFO("\x1b[033m[HLTReceiver] " << eos << " bytes written from received...\x1b[0m");
      //B2INFO("\x1b[033m[HLTReceiver] " << size << " bytes still left...\x1b[0m");
    }

    if (size > 0) {
      //B2INFO("\x1b[033m[HLTReceiver] Data still remains without EOS. Saving it into the internal buffer...\x1b[0m");
      memcpy(m_internalBuffer, data, sizeof(char) * size);
      m_internalBufferWriteIndex += size;
      m_internalBufferEntries++;
      //B2INFO("\x1b[033m[HLTReceiver] " << m_internalBufferWriteIndex << " bytes saved in the internal buffer...\x1b[0m");
    }

    return sizes.size();
  }

  return 0;
}

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
