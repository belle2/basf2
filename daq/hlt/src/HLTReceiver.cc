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
    std::vector<std::string> givenMessages;
    std::string data;
    data.clear();
    int size = 0;
    status = c_FuncError;

    while ((status = receive(newSocket, data, size)) != c_FuncError) {
      if (decodeSingleton(data, givenMessages) == c_Success) {
        for (std::vector<std::string>::const_iterator i = givenMessages.begin();
             i != givenMessages.end(); ++i) {
          if ((*i) == "Terminate") {
            B2INFO("\x1b[31m[HLTReceiver] Terminate tag met\x1b[0m");
            m_buffer->insq((int*)(*i).c_str(), (*i).size() / 4 + 1);
            return c_TermCalled;
          } else
            m_buffer->insq((int*)(*i).c_str(), (*i).size() / 4 + 1);
        }
        break;
      }
    }
  }

  return c_Success;
}

EHLTStatus HLTReceiver::setBuffer()
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(m_port);

  return c_Success;
}

EHLTStatus HLTReceiver::setBuffer(unsigned int key)
{
  B2INFO("[HLTReceiver] \x1b[32mRing buffer initializing...\x1b[0m");
  m_buffer = new RingBuffer(key);

  return c_Success;
}

EHLTStatus HLTReceiver::decodeSingleton(std::string data, std::vector<std::string>& container)
{
  container.clear();
  std::string returnData;
  size_t eos = data.find(gEOSTag);

  if (eos == std::string::npos) {
    m_internalBuffer.push_back(data);
    return c_FuncError;
  } else {
    while ((eos = data.find(gEOSTag)) != std::string::npos) {
      if (m_internalBuffer.size() > 0) {
        std::string reconstructed;
        for (std::vector<std::string>::const_iterator i = m_internalBuffer.begin();
             i != m_internalBuffer.end(); ++i)
          reconstructed += (*i);
        reconstructed += data.substr(0, eos);
        container.push_back(reconstructed);

        m_internalBuffer.clear();
      } else
        container.push_back(data.substr(0, eos));
      data = data.substr(eos + gEOSTag.size(), std::string::npos);
    }

    if (data.size() != 0) {
      m_internalBuffer.push_back(data);
      return c_FuncError;
    }
  }

  return c_Success;
}
