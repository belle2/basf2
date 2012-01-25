#ifndef HLTSENDER_H
#define HLTSENDER_H

#include <string>

#include <daq/hlt/HLTDefs.h>
#include <framework/logging/Logger.h>

#include <daq/hlt/B2Socket.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {
  class HLTSender : public B2Socket {
  public:
    HLTSender(const std::string destination, unsigned int port);
    ~HLTSender();

    EHLTStatus createConnection();
    EHLTStatus broadcasting();
    EHLTStatus broadcasting(std::string data);

    EHLTStatus setBuffer();
    EHLTStatus setBuffer(unsigned int key);

    EHLTStatus process(EHLTMessage dataType = c_Termination, std::string data = "");
    std::string makeSingleton(std::string data);

  protected:
    EHLTStatus init();

  private:
    std::string m_destination;
    unsigned int m_port;

    RingBuffer* m_buffer;
  };
}

#endif
