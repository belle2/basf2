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

  char temp[gMaxReceives];
  m_buffer->remq((int*)temp);

  std::string rawMessage(temp);
  std::string sendingMessage = makeSingleton(rawMessage);

  int size = 0;

  if (send(sendingMessage, size) == c_FuncError) {
    B2INFO("[HLTSender] \x1b[31mAn error occurred in sending so the data is put back to the ring buffer\x1b[0m");
    m_buffer->insq((int*)sendingMessage.c_str(), sendingMessage.size() / 4 + 1);
    sleep(1);
    return c_FuncError;
  } else {
    //B2INFO ("[HLTSender] \x1b[34mData " << sendingMessage << " (size=" << sendingMessage.size ()
    //    << ") has been sent to " << m_destination << "\x1b[0m");

    if (rawMessage == "Terminate") {
      m_buffer->insq((int*)rawMessage.c_str(), rawMessage.size() / 4 + 1);
      return c_TermCalled;
    }

    return c_Success;
  }
}

EHLTStatus HLTSender::broadcasting(std::string data)
{
  int size = 0;

  std::string sendingMessage = makeSingleton(data);

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
  m_buffer = new RingBuffer(m_port);

  return c_Success;
}

EHLTStatus HLTSender::setBuffer(unsigned int key)
{
  B2INFO("[HLTSender] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key);

  return c_Success;
}

EHLTStatus HLTSender::process(EHLTMessage dataType, std::string data)
{
  if (dataType == c_Termination) {
    B2INFO("[HLTSender] Terminating...");
    return c_TermCalled;
  } else {
    B2INFO("[HLTSender] Got: " << data);
    return c_Success;
  }
}

std::string HLTSender::makeSingleton(std::string data)
{
  std::string tempMessage(data);
  tempMessage += "EOS";

  return tempMessage;
}
