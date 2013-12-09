#include "daq/slc/apps/hvcontrold/HVChannelInfo.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

HVChannelInfo::HVChannelInfo(unsigned int crate, unsigned int slot, unsigned int ch)
  : DataObject("HVChannelInfo", "")
{
  addUInt("crate", crate);
  addUInt("slot", slot);
  addUInt("channel", ch);
  addBool("switch_on", false);
  addUInt("rampup_speed", 0);
  addUInt("rampdown_speed", 0);
  addUInt("voltage_demand", 0);
  addUInt("voltage_limit", 0xFFFF);
  addUInt("current_limit", 0xFFFF);
}

HVChannelInfo::~HVChannelInfo() throw()
{

}

std::string HVChannelInfo::print_names()
{
  std::stringstream ss;
  ss << " crate | slot | channel | switch_on | rampup_speed |"
     << " rampdown_speed | voltage_demand | voltage_limit | "
     << " current_limit ";
  return ss.str();
}

std::string HVChannelInfo::print_values()
{
  std::stringstream ss;
  ss << Belle2::form(" %5d |  %3d | %7d | %9d | %12d |",
                     getUInt("crate"), getUInt("slot"), getUInt("channel"),
                     getBool("switch_on"), getUInt("rampup_speed"));
  ss << Belle2::form(" %14d |  %13d | %13d | %14d ",
                     getUInt("rampdown_speed"), getUInt("voltage_demand"),
                     getUInt("voltage_limit"), getUInt("current_limit"));
  return ss.str();
}
