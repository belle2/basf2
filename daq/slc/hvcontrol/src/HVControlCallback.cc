#include "daq/slc/hvcontrol/HVControlCallback.h"
#include "daq/slc/hvcontrol/HVCommand.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <iostream>

using namespace Belle2;

bool g_init = false;
HVCrateList::const_iterator g_icrate;

void HVControlCallback::turnon() throw(HVHandlerException)
{
  load(getConfig(0), false, false);
}

void HVControlCallback::turnoff() throw(HVHandlerException)
{
  load(getConfig(0), true, false);
}

void HVControlCallback::configure(const HVConfig& config) throw(HVHandlerException)
{
  load(config, false, true);
}

void HVControlCallback::standby() throw(HVHandlerException)
{
  load(getConfig(0), false, true);
}

void HVControlCallback::shoulder() throw(HVHandlerException)
{
  if (getConfigs().size() > 1)
    load(getConfig(1), false, true);
}

void HVControlCallback::peak() throw(HVHandlerException)
{
  int index = getConfigs().size() - 1;
  load(getConfig(index), false, true);
}

void HVControlCallback::load(const HVConfig& config,
                             bool alloff, bool loadpars) throw(HVHandlerException)
{
  try {
    const HVCrateList& crate_v(config.getCrates());
    for (HVCrateList::const_iterator icrate = crate_v.begin();
         icrate != crate_v.end(); icrate++) {
      const HVCrate& crate(*icrate);
      const HVChannelList& channel_v(crate.getChannels());
      int crateid = crate.getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        const HVChannel& channel(*ichannel);
        int slot = channel.getSlot();
        int ch = channel.getChannel();
        if (alloff) {
          setSwitch(crateid, slot, ch, false);
        } else if (loadpars) {
          std::cout << crateid << "-" << slot << "-" << ch << " "
                    << channel.getRampUpSpeed() << " "
                    << channel.getRampDownSpeed() << " "
                    << channel.getVoltageLimit() << " "
                    << channel.getCurrentLimit() << std::endl;
          setRampUpSpeed(crateid, slot, ch, channel.getRampUpSpeed());
          setRampDownSpeed(crateid, slot, ch, channel.getRampDownSpeed());
          setVoltageDemand(crateid, slot, ch, (channel.isTurnOn()) ? channel.getVoltageDemand() : -1);
          setVoltageLimit(crateid, slot, ch, channel.getVoltageLimit());
          setCurrentLimit(crateid, slot, ch, channel.getCurrentLimit());
          usleep(500000);
        } else {
          setSwitch(crateid, slot, ch, channel.isTurnOn());
        }
      }
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
    throw (HVHandlerException(e.what()));
  }
}

void HVControlCallback::init(NSMCommunicator&) throw()
{
  getNode().setState(HVState::OFF_S);
  LogFile::debug(m_confignames);
  dbload(m_confignames);
  addAll(getConfig());
  initialize(getConfig());
}

void HVControlCallback::dbload(const std::string& data) throw(IOException)
{
  StringList confs = StringUtil::split(data, ',');
  resetConfigs();
  reset();
  if (m_db) {
    DBInterface& db(*m_db);
    for (size_t i = 0; i < confs.size(); i++) {
      HVConfig config;
      const std::string confname = (StringUtil::find(confs[i], "@")) ?
                                   confs[i] : getNode().getName() + "@" + confs[i];
      if (confname.size() > 0) {
        try {
          DBObject obj = DBObjectLoader::load(db, m_table, confname);
          config.set(obj);
          addConfig(config);
        } catch (const DBHandlerException& e) {
          LogFile::debug("DB access error:%s", e.what());
          db.close();
          m_mutex.unlock();
          throw (e);
        }
      }
    }
    db.close();
  }
}

void HVControlCallback::timeout(NSMCommunicator&) throw()
{
  update();
  HVConfigList::const_iterator iconfig = getConfigs().begin();
  if (iconfig == getConfigs().end()) return;
  const HVConfig& config(*iconfig);
  const HVCrateList& crate_v(config.getCrates());
  int i = 0;
  if (!g_init || g_icrate == crate_v.end()) {
    g_icrate = crate_v.begin();
    g_init = true;
  }
  const HVCrate& crate(*g_icrate);
  const HVChannelList& channel_v(crate.getChannels());
  int crateid = crate.getId();
  bool istable = true;
  for (HVChannelList::const_iterator ichannel = channel_v.begin();
       ichannel != channel_v.end(); ichannel++) {
    const HVChannel& channel(*ichannel);
    int slot = channel.getSlot();
    int ch = channel.getChannel();
    int state = getState(crateid, slot, ch);
    std::string state_s = HVMessage::getStateText((HVMessage::State)state);
    float vmon = getVoltageMonitor(crateid, slot, ch);
    float cmon = getCurrentMonitor(crateid, slot, ch);
    std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, ch);
    if ((m_state_demand == HVState::OFF_S && state != HVMessage::OFF) ||
        (m_state_demand != HVState::OFF_S && state != HVMessage::ON)) {
      istable = false;
    }
    if (m_mon_tmp[crateid][i].state != state) {
      set(vname + "state", state_s);
      m_mon_tmp[crateid][i].state = state;
    }
    if (m_mon_tmp[crateid][i].vmon != vmon) {
      set(vname + "vmon", vmon);
      m_mon_tmp[crateid][i].vmon = vmon;
    }
    if (m_mon_tmp[crateid][i].cmon != cmon) {
      set(vname + "cmon", cmon);
      m_mon_tmp[crateid][i].cmon = cmon;
    }
    i++;
  }
  if (m_state_demand != getNode().getState()) {
    getNode().setState(m_state_demand);
    reply(NSMMessage(NSMCommand::OK, m_state_demand.getLabel()));
  }
  g_icrate++;
}

void HVControlCallback::monitor() throw()
{
}

void HVControlCallback::HVNodeMonitor::run()
{
  while (true) {
    sleep(m_callback->getTimeout());
    m_callback->monitor();
  }
}
