#ifndef Belle2_HVChannelConfig_h
#define Belle2_HVChannelConfig_h

#include "daq/slc/apps/hvcontrold/hv_channel_config.h"

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/base/Serializable.h>

namespace Belle2 {

  enum HVDemand {
    STANDBY = 0, STANDBY2, STANDBY3, PEAK
  };

  class HVChannelConfig : public Serializable {

  public:
    HVChannelConfig(uint32 master = 0, uint32 crate = 0,
                    uint32 slot = 0, uint32 channel = 0);
    HVChannelConfig(const hv_channel_config& config) {
      setConfig(config);
    }
    virtual ~HVChannelConfig() throw() {}

  public:
    void print() throw();
    const hv_channel_config& getConfig() const { return _config; }
    hv_channel_config& getConfig() { return _config; }
    void setConfig(const hv_channel_config& config);

    uint32 getConfigId() const { return _config.configid; }
    uint32 getMaster() const { return _config.master; }
    uint32 getCrate() const { return _config.crate; }
    uint32 getSlot() const { return _config.slot; }
    uint32 getChannel() const { return _config.channel; }
    bool  isTurnOn() const { return _config.turnon; }
    float getRampUpSpeed() const { return _config.rampup_speed; }
    float getRampDownSpeed() const { return _config.rampdown_speed; }
    float getVoltageDemand(HVDemand i) const { return _config.voltage_demand[i]; }
    float getVoltageLimit() const { return _config.voltage_limit; }
    float getCurrentLimit() const { return _config.current_limit; }
    float getReserved(int i) const { return _config.reserved[i]; }
    size_t getNVoltageDemands() const { return sizeof(_config.voltage_demand) / sizeof(float); }
    size_t getNReserveds() const { return sizeof(_config.reserved) / sizeof(float); }

    void setConfigId(uint32 id) { _config.configid = id; }
    void setMaster(uint32 id) { _config.master = id; }
    void setCrate(uint32 id) { _config.crate = id; }
    void setSlot(uint32 id) { _config.slot = id; }
    void setChannel(uint32 id) { _config.channel = id; }
    void setTurnOn(bool turnon) { _config.turnon = turnon; }
    void setRampUpSpeed(float rampup) { _config.rampup_speed = rampup; }
    void setRampDownSpeed(float rampdown) { _config.rampdown_speed = rampdown; }
    void setVoltageDemand(HVDemand i, float voltage) { _config.voltage_demand[i] = voltage; }
    void setVoltageLimit(float voltage) { _config.voltage_limit = voltage; }
    void setCurrentLimit(float current) { _config.current_limit = current; }
    void setReserved(int i, float reserved) { _config.reserved[i] = reserved; }

  public:
    const std::string toSQLCreate(const std::string& tablename) throw();
    const std::string toSQLInsert(const std::string& tablename, int index) throw();
    void setValues(DBRecord& record) throw();

  public:
    void writeObject(Writer& writer) const throw(IOException);
    void readObject(Reader& reader) throw(IOException);

  private:
    hv_channel_config _config;

  };

}

#endif
