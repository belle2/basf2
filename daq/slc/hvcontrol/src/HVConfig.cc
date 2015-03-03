#include "daq/slc/hvcontrol/HVConfig.h"

#include <daq/slc/system/LogFile.h>

#include <stdexcept>

using namespace Belle2;

void HVConfig::reset() throw()
{
  m_crate_v = HVCrateList();
}

HVChannel& HVConfig::getChannel(int crateid, int slot, int ch)
{
  HVCrateList& crate_v(getCrates());
  for (HVCrateList::iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    HVCrate& crate(*icrate);
    HVChannelList& channel_v(crate.getChannels());
    if (crate.getId() != crateid) continue;
    for (HVChannelList::iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      HVChannel& channel(*ichannel);
      if (slot == channel.getSlot() &&
          ch == channel.getChannel()) {
        return channel;
      }
    }
  }
  throw (std::out_of_range("No channel found"));
}

HVChannel& HVConfig::getChannel(int crateid, int index)
{
  HVCrateList& crate_v(getCrates());
  for (HVCrateList::iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    HVCrate& crate(*icrate);
    HVChannelList& channel_v(crate.getChannels());
    if (crate.getId() != crateid) continue;
    if (index < (int)channel_v.size()) {
      return channel_v[index];
    }
  }
  throw (std::out_of_range("No channel found"));
}

void HVConfig::set(const DBObject& obj) throw()
{
  reset();
  m_obj = obj;
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {
      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getBool("turnon"));
      ch.setRampUpSpeed(c_ch.getFloat("rampup"));
      ch.setRampDownSpeed(c_ch.getFloat("rampdown"));
      ch.setVoltageDemand(c_ch.getFloat("vdemand"));
      ch.setVoltageLimit(c_ch.getFloat("vlimit"));
      ch.setCurrentLimit(c_ch.getFloat("climit"));
      crate.addChannel(ch);
    }
    m_crate_v.push_back(crate);
  }
}
