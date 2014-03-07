#ifndef _Belle2_HVChannelStatus_h
#define _Belle2_HVChannelStatus_h

#include "daq/slc/apps/hvcontrold/hv_channel_status.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include <daq/slc/base/Serializable.h>

#include <string>
#include <vector>

namespace Belle2 {

  class HVChannelStatus : public Serializable {

  public:
    static const int MAX_CHANNELS = 20;

  public:
    HVChannelStatus();
    virtual ~HVChannelStatus() throw() {}

  public:
    const hv_channel_status& getStatus() const { return _status; }
    hv_channel_status& getStatus() { return _status; }
    void setStatus(const hv_channel_status& status);
    void setStatus(const hv_channel_status* status);

    HVState getState() const { return HVState(_status.state); }
    float getVoltageMon() const { return _status.voltage_mon; }
    float getCurrentMon() const { return _status.current_mon; }

    void setState(HVState state) { _status.state = state.getId(); }
    void setVoltageMon(float voltage) { _status.voltage_mon = voltage; }
    void setCurrentMon(float current) { _status.current_mon = current; }

  public:
    void writeObject(Writer& writer) const throw(IOException);
    void readObject(Reader& reader) throw(IOException);

  private:
    hv_channel_status _status;

  };
}

#endif
