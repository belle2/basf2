#ifndef WaveformEvent_H
#define WaveformEvent_H

#include <topcaf/dataobjects/EventHeaderPacket.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>

namespace Belle2 {

  class WaveformEvent : public TObject {

  public:
    WaveformEvent();
    ~WaveformEvent();

    int GetNumWaveformPackets();
    EventWaveformPacket* GetWaveformPacket(int packet_number);
    EventHeaderPacket*   GetEventHeaderPacket();

    void SetHeaderPacket(EventHeaderPacket*);
    void AddWaveformPacket(EventWaveformPacket*);

  private:

    EventHeaderPacket* m_EventHeaderPacket;
    std::vector<EventWaveformPacket*> m_EventWaveformPackets;
    ClassDef(WaveformEvent, 1);
  }

}
