#include "daq/slc/hvcontrol/HVStatus.h"
#include "daq/slc/hvcontrol/hv_status.h"

using namespace Belle2;

HVStatus::HVStatus()
{
  m_state = OFF;
  m_voltage_mon = 0;
  m_current_mon = 0;
}

void HVStatus::set(const void* buf) throw()
{
  const hv_status::channel_status* status = (const hv_status::channel_status*)buf;
  setState((State)status->state);
  setVoltageMon(status->voltage_mon);
  setCurrentMon(status->current_mon);
}

void HVStatus::get(void* buf) const throw()
{
  hv_status::channel_status* status = (hv_status::channel_status*)buf;
  status->state = (int)getState();
  status->voltage_mon = getVoltageMon();
  status->current_mon = getCurrentMon();
}
