#include <topcaf/dataobjects/CamacData.h>

using namespace Belle2;

CamacData::CamacData()
{

  unsigned short default_val = 0;

  m_evt_num = 0;

  // timing data
  m_raw_tdc = m_raw_rf0 = m_raw_rf1 = m_raw_rf2 = m_raw_rf3 = default_val;

  // trigger paddle data
  m_raw_trigS_tdc0 = m_raw_trigM_tdc0 = m_raw_trigS_tdc1 = m_raw_trigM_tdc1 = default_val;
  m_raw_timing_tdc = m_raw_ratemon = default_val;
  m_raw_trigS_adc0 = m_raw_trigM_adc0 = m_raw_trigS_adc1 = m_raw_trigM_adc1 = default_val;
  m_raw_timing_adc = m_raw_veto0 = m_raw_veto1 = default_val;

  // LEPS marker word
  m_raw_leps_marker_word = default_val;

}

CamacData::~CamacData()
{
}


