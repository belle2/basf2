#ifndef _Belle2_ArichHVUnit_h
#define _Belle2_ArichHVUnit_h

#include <daq/slc/hvcontrol/HVValue.h>
#include <daq/slc/hvcontrol/HVStatus.h>
#include <daq/slc/hvcontrol/HVChannel.h>

namespace Belle2 {

  class ArichHVCalib {

  public:
    ArichHVCalib(float vol_offset, float vol_slope,
                 float cur_offset, float cur_slope);
    ArichHVCalib();
    ~ArichHVCalib() {};

  public:
    float getVoltageOffset() const;
    float getVoltageSlop() const;
    float getCurrentOffset() const;
    float getCurrentSlop() const;
    int encodeVoltage(float voltage) const;
    int encodeCurrent(float current) const;
    float decodeVoltage(int voltage) const;
    float decodeCurrent(int current) const;

  private:
    float m_vol_offset;
    float m_vol_slope;
    float m_cur_offset;
    float m_cur_slope;

  };

  class ArichHVUnit {

  public:
    ArichHVUnit() {}
    ArichHVUnit(const ArichHVCalib& calib)
      : m_calib(calib) {}
    ArichHVUnit(const ArichHVCalib& calib,
                const HVValue& value)
      : m_calib(calib), m_value(value) {}
    ArichHVUnit(const ArichHVCalib& calib,
                const HVValue& value,
                const HVChannel& channel)
      : m_calib(calib), m_value(value),
        m_channel(channel) {}
    ArichHVUnit(const ArichHVCalib& calib,
                const HVChannel& channel)
      : m_calib(calib), m_value(),
        m_channel(channel) {}
    ~ArichHVUnit() {}

  public:
    void setCalib(const ArichHVCalib& calib) { m_calib = calib; }
    void setValue(const HVValue& value) { m_value = value; }
    void setStatus(const HVStatus& status) { m_status = status; }
    void setChannel(const HVChannel& channel) { m_channel = channel; }
    ArichHVCalib& getCalib() { return m_calib; }
    HVValue& getValue() { return m_value; }
    HVStatus& getStatus() { return m_status; }
    HVChannel& getChannel() { return m_channel; }
    const ArichHVCalib& getCalib() const { return m_calib; }
    const HVValue& getValue() const { return m_value; }
    const HVStatus& getStatus() const { return m_status; }
    const HVChannel& getChannel() const { return m_channel; }

  private:
    ArichHVCalib m_calib;
    HVValue m_value;
    HVStatus m_status;
    HVChannel m_channel;

  };

}

#endif
