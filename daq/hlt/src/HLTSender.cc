#include <daq/hlt/HLTSender.h>

using namespace Belle2;

HLTSender::HLTSender(const std::string destination, unsigned int port)
{
  m_destination = destination;
  m_port = port;

  m_buffer = NULL;
}

HLTSender::~HLTSender()
{
}

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

EHLTStatus HLTSender::broadcasting()
{
  if (m_buffer->numq() <= 0) {
    return c_Success;
  }

  char temp[gMaxReceives + gEOSTag.size()];
  memset(temp, 0, gMaxReceives + gEOSTag.size());

  int bufferStatus = m_buffer->remq((int*)temp);
  if (bufferStatus < 0)
    return c_FuncError;

  //int size = (bufferStatus - 1) * 4;
  int size = bufferStatus * 4;

  writeFile("sender", temp, size);
  makeSingleton(temp, size);
  size += gEOSTag.size() + 1;

  writeFile("senderAfterEncode", temp, size);

  while (send(temp, size) == c_FuncError) {
    B2INFO("[HLTSender] \x1b[31mAn error occurred in sending so the data is put back to the ring buffer\x1b[0m");
    sleep(1);
  }

  if (!strcmp(temp, gTerminationTag.c_str())) {
    m_buffer->insq((int*)temp, size / 4 + 1);
    return c_TermCalled;
  }

  return c_Success;
}

EHLTStatus HLTSender::broadcasting(std::string data)
{
  int size = 0;

  std::string sendingMessage = makeSingleton(data);

  writeFile("sender.manager", (char*)sendingMessage.c_str(), sendingMessage.size());
  if (send(sendingMessage, size) == c_FuncError)
    return c_FuncError;
  else {
    //B2INFO ("[HLTSender] \x1b[34mData " << sendingMessage << " (size=" << sendingMessage.size ()
    //    << ") has been sent to " << m_destination << "\x1b[0m");
    return c_Success;
  }
}

EHLTStatus HLTSender::setBuffer()
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(m_port)).c_str(), gBufferSize);

  return c_Success;
}

EHLTStatus HLTSender::setBuffer(unsigned int key)
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(boost::lexical_cast<std::string>(static_cast<int>(key)).c_str(), gBufferSize);

  return c_Success;
}

std::string HLTSender::makeSingleton(std::string data)
{
  std::string tempMessage(data);
  tempMessage += gEOSTag;

  return tempMessage;
}

EHLTStatus HLTSender::makeSingleton(char* data, int size)
{
  char* eosTag = (char*)gEOSTag.c_str();
  memcpy(data + sizeof(char) * size, eosTag, gEOSTag.size());

  return c_Success;
}

void HLTSender::writeFile(char* file, char* data, int size)
{
  FILE* fp;
  fp = fopen(file, "a");
  for (int i = 0; i < size; i++)
    fprintf(fp, "%c", data[i]);
  fclose(fp);
}
