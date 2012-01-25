#ifndef HLTRECEIVER_H
#define HLTRECEIVER_H

#include <sstream>

#include <daq/hlt/B2Socket.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTReceiver : public B2Socket {
  public:
    HLTReceiver(unsigned int port);
    ~HLTReceiver();

    EHLTStatus createConnection();
    EHLTStatus listening();

    EHLTStatus setBuffer();
    EHLTStatus setBuffer(unsigned int key);

    EHLTStatus decodeSingleton(std::string data, std::vector<std::string>& container);

  protected:
    EHLTStatus init();

  private:
    unsigned int m_port;

    RingBuffer* m_buffer;
    std::vector<std::string> m_internalBuffer;
  };
}

#endif
