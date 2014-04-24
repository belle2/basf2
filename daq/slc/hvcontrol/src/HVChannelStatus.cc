#include "daq/slc/hvcontrol/HVChannelStatus.h"
#include "daq/slc/hvcontrol/hv_status.h"

using namespace Belle2;

HVChannelStatus::HVChannelStatus()
{
  m_state = HVState::OFF_S;
  m_voltage_mon = 0;
  m_current_mon = 0;
}

void HVChannelStatus::set(const void* buf) throw()
{
  const hv_status::channel_status* status = (const hv_status::channel_status*)buf;
  setState(status->state);
  setVoltageMon(status->voltage_mon);
  setCurrentMon(status->current_mon);
}

void HVChannelStatus::get(void* buf) const throw()
{
  hv_status::channel_status* status = (hv_status::channel_status*)buf;
  status->state = getState().getId();
  status->voltage_mon = getVoltageMon();
  status->current_mon = getCurrentMon();
}
