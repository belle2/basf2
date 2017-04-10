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
  load(getConfig(), false, true);
  load(getConfig(), false, false);
}

void HVControlCallback::turnoff() throw(HVHandlerException)
{
  load(getConfig(), true, false);
}

void HVControlCallback::configure(const HVConfig& config) throw(HVHandlerException)
{
  load(config, false, true);
}

void HVControlCallback::standby() throw(HVHandlerException)
{
  load(getConfig(), false, true);
}

void HVControlCallback::shoulder() throw(HVHandlerException)
{
  if (getConfig().getName().size() > 0) {
    load(getConfig(), false, true);
  }
}

void HVControlCallback::peak() throw(HVHandlerException)
{
  load(getConfig(), false, true);
}

void HVControlCallback::load(const HVConfig& config,
                             bool alloff, bool loadpars)
throw(HVHandlerException)
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
        std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, ch);
        if (alloff) {
          paramLock();
          setSwitch(crateid, slot, ch, false);
          paramUnlock();
          set(vname + "switch", "OFF");
        } else if (loadpars) {
          paramLock();
          setSwitch(crateid, slot, ch, false);
          set(vname + "switch", "OFF");
          ///*
          if (channel.isTurnOn()) {
            if (m_state_demand.isOn() && !getSwitch(crateid, slot, ch)) {
              setSwitch(crateid, slot, ch, true);
              set(vname + "switch", "ON");
            }
          } else {
            if (getSwitch(crateid, slot, ch)) {
              setSwitch(crateid, slot, ch, false);
              set(vname + "switch", "OFF");
            }
          }
          //*/
          setRampUpSpeed(crateid, slot, ch, channel.getRampUpSpeed());
          setRampDownSpeed(crateid, slot, ch, channel.getRampDownSpeed());
          LogFile::debug("voltage.demand : %f ", channel.getVoltageDemand());
          // setVoltageDemand(crateid, slot, ch, (channel.isTurnOn()) ? channel.getVoltageDemand() : 0);
          setVoltageDemand(crateid, slot, ch, channel.getVoltageDemand());
          setVoltageLimit(crateid, slot, ch, channel.getVoltageLimit());
          setCurrentLimit(crateid, slot, ch, channel.getCurrentLimit());
          paramUnlock();
          set(vname + "rampup", channel.getRampUpSpeed());
          set(vname + "rampdown", channel.getRampDownSpeed());
          //set(vname + "vdemand", (channel.isTurnOn()) ? channel.getVoltageDemand() : -1);
          set(vname + "vdemand", channel.getVoltageDemand());
          set(vname + "vlimit", channel.getVoltageLimit());
          set(vname + "climit", channel.getCurrentLimit());
        } else {
          paramLock();
          setSwitch(crateid, slot, ch, channel.isTurnOn());
          paramUnlock();
          set(vname + "switch", channel.isTurnOn() ? "ON" : "OFF");
        }
      }
    }
  } catch (const IOException& e) {
    LogFile::error("Loading failed %s:%d %s", __FILE__, __LINE__, e.what());
    throw (HVHandlerException(e.what()));
  }
  LogFile::debug("Loading... done");
}

void HVControlCallback::init(NSMCommunicator&) throw()
{
  getNode().setState(HVState::OFF_S);
  dbload(m_config_standby, m_configname_standby);
  dbload(m_config_shoulder, m_configname_shoulder);
  dbload(m_config_peak, m_configname_peak);
  m_config = FLAG_STANDBY;
  addAll(getConfig());
  configure(getConfig());
  initialize(getConfig());
  PThread(new HVNodeMonitor(this));
}

void HVControlCallback::timeout(NSMCommunicator&) throw()
{
}

void HVControlCallback::monitor() throw()
{
  lock();
  try {
    update();
  } catch (const HVHandlerException& e) {
    log(LogFile::ERROR, e.what());
  }
  try {
    const HVConfig& config(getConfig());
    const HVCrateList& crate_v(config.getCrates());
    bool isstable = true;
    for (size_t i = 0; i < crate_v.size(); i++) {
      const HVCrate& crate(crate_v[i]);
      const HVChannelList& channel_v(crate.getChannels());
      int crateid = crate.getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        const HVChannel& channel(*ichannel);
        int slot = channel.getSlot();
        int ch = channel.getChannel();
        paramLock();
        int state = getState(crateid, slot, ch);
        std::string state_s = HVMessage::getStateText((HVMessage::State)state);
        float vmon = getVoltageMonitor(crateid, slot, ch);
        float cmon = getCurrentMonitor(crateid, slot, ch);
        paramUnlock();
        std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, ch);
        if (m_state_demand == HVState::OFF_S) {
          if (state != HVMessage::OFF) {
            isstable = false;
          }
        } else  if ((!channel.isTurnOn() && state != HVMessage::OFF) ||
                    (channel.isTurnOn() && state != HVMessage::ON)) {
          isstable = false;
        }
        LogFile::debug("istrunon = %s state = %s", (channel.isTurnOn() ? "ON" : "OFF"), state_s.c_str());
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
    }
    if (isstable && m_state_demand != getNode().getState()) {
      log(LogFile::NOTICE, "State transit : %s", m_state_demand.getLabel());
      setHVState(m_state_demand);
      reply(NSMMessage(NSMCommand::OK, m_state_demand.getLabel()));
    }
  } catch (const std::exception& e) {
    log(LogFile::ERROR, "%s:%d %s", __FILE__, __LINE__, e.what());
  }
  unlock();
}

void HVControlCallback::dbload(HVConfig& config,
                               const std::string& configname_in)
throw(IOException)
{
  if (m_db) {
    DBInterface& db(*m_db);
    const std::string confname = (StringUtil::find(configname_in, "@")) ?
                                 configname_in : getNode().getName() + "@" + configname_in;
    LogFile::info("config : " + confname);
    if (confname.size() > 0) {
      try {
        DBObject obj = DBObjectLoader::load(db, m_table, confname);
        config.set(obj);
      } catch (const DBHandlerException& e) {
        LogFile::debug("DB access error:%s", e.what());
        db.close();
        m_mutex.unlock();
        throw (e);
      }
    }
    db.close();
  }
}

void HVControlCallback::HVNodeMonitor::run()
{
  while (true) {
    sleep(m_callback->getTimeout());
    m_callback->monitor();
  }
}
