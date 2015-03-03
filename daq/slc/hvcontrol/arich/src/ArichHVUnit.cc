#include <daq/slc/hvcontrol/arich/ArichHVUnit.h>

using namespace Belle2;

ArichHVCalib::ArichHVCalib(float vol_offset, float vol_slope,
                           float cur_offset, float cur_slope)
  : m_vol_offset(vol_offset), m_vol_slope(vol_slope),
    m_cur_offset(cur_offset), m_cur_slope(cur_slope) {}

ArichHVCalib::ArichHVCalib()
  : m_vol_offset(0), m_vol_slope(1),
    m_cur_offset(0), m_cur_slope(1) {}

float ArichHVCalib::getVoltageOffset() const
{
  return m_vol_offset;
}

float ArichHVCalib::getVoltageSlop() const
{
  return m_vol_slope;
}

float ArichHVCalib::getCurrentOffset() const
{
  return m_cur_offset;
}

float ArichHVCalib::getCurrentSlop() const
{
  return m_cur_slope;
}

int ArichHVCalib::encodeVoltage(float voltage) const
{
  return (voltage - m_vol_offset) / m_vol_slope;
}

int ArichHVCalib::encodeCurrent(float current) const
{
  return (current - m_cur_offset) / m_cur_slope;
}

float ArichHVCalib::decodeVoltage(int voltage) const
{
  return m_vol_slope * voltage + m_vol_offset;
}

float ArichHVCalib::decodeCurrent(int current) const
{
  return m_cur_slope * current + m_cur_offset;
}
