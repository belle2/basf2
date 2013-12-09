#include "daq/slc/apps/hvcontrold/HVChannelStatus.h"

#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

HVChannelStatus::HVChannelStatus(unsigned int crate, unsigned int slot, unsigned int ch)
  : DataObject("HVChannelStatus", "")
{
  addUInt("crate", crate);
  addUInt("slot", slot);
  addUInt("channel", ch);
  addUInt("status", HVState::OFF_STABLE_S.getId());
  addUInt("voltage_monitored", 0);
  addUInt("current_monitored", 0);
}

HVChannelStatus::~HVChannelStatus() throw()
{

}

std::string HVChannelStatus::print_names()
{
  std::stringstream ss;
  ss << " crate | slot | channel | status | voltage_monitored | current_monitored ";
  return ss.str();
}

std::string HVChannelStatus::print_values()
{
  std::stringstream ss;
  ss << Belle2::form(" %5d |  %3d | %7d |", getUInt("crate"),
                     getUInt("slot"), getUInt("channel"));
  ss << Belle2::form(" %6d | %17d | %16d ", getUInt("status"),
                     getUInt("voltage_monitored"),
                     getUInt("current_monitored"));
  return ss.str();
}
