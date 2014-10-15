#ifndef TEST_MODULE_H
#define TEST_MODULE_H
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <topcaf/dataobjects/inc/Packet.h>
#include <topcaf/dataobjects/inc/EventHeaderPacket.h>
#include <topcaf/dataobjects/inc/EventWaveformPacket.h>

namespace Belle2 {

  class TestModule : public Module {

  public:
    TestModule();
    void initialize();
    void event();

  private:
    StoreObjPtr<Packet> m_packet_ptr;
    StoreObjPtr<EventHeaderPacket> m_evtheader_ptr;
    StoreObjPtr<EventWaveformPacket> m_evtwave_ptr;

  };
}

#endif
