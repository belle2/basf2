#include "HVChannelInfo.h"

#include "base/StringUtil.h"

using namespace Belle2;

HVChannelInfo::HVChannelInfo(unsigned int crate, unsigned int slot, unsigned int ch)
  : DataObject("HVChannelInfo", "")
{
  setName(Belle2::form("crate%02d_slot%02d_ch%02d", crate, slot, ch));
  addUInt("crate", crate);
  addUInt("slot", slot);
  addUInt("channel", ch);
  addBool("switch_on", false);
  addUInt("rampup_speed", 0);
  addUInt("rampdown_speed", 0);
  addUInt("voltage_demand", 0);
  addUInt("voltage_limit", 0);
  addUInt("current_limit", 0);
  addUInt("voltage_monitored", 0);
  addUInt("current_monitored", 0);
}

HVChannelInfo::~HVChannelInfo() throw()
{

}
