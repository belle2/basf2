#include "daq/slc/hvcontrol/HVTUINSM.h"
#include "daq/slc/hvcontrol/HVCommand.h"

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <daq/slc/system/PThread.h>

#include <unistd.h>
#include <ncurses.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

using namespace Belle2;

HVTUINSM::HVTUINSM(int crate)
{
  m_crate = crate;
}

void HVTUINSM::initNSM(const std::string& cfilename,
                       const std::string& cuiname)
{
  ConfigFile cfile("slowcontrol", cfilename);
  PostgreSQLInterface db(cfile.get("database.host"),
                         cfile.get("database.dbname"),
                         cfile.get("database.user"),
                         cfile.get("database.password"),
                         cfile.getInt("database.port"));
  const std::string nodename = cfile.get("nsm.nodename");
  LogFile::open("hvcui/" + cuiname, LogFile::ERROR);
  StringList confignames = StringUtil::split(cfile.get("hv.config.standby"), ',');
  const std::string configname = (StringUtil::find(confignames[0], "@")) ?
                                 confignames[0] : StringUtil::toupper(nodename) + "@" + confignames[0];
  const std::string tablename = cfile.get("hv.tablename");
  DBObject obj = DBObjectLoader::load(db, tablename, configname);
  LogFile::notice("Loaded %s from %s", configname.c_str(),
                  tablename.c_str());
  HVConfig config(obj);
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    if (m_crate < 0 || m_crate == crate.getId()) {
      const HVChannelList& channel_v(crate.getChannels());
      int crateid = crate.getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        const HVChannel& channel(*ichannel);
        int slot = channel.getSlot();
        int ch = channel.getChannel();
        addChannel(crateid, slot, ch);
      }
    }
  }

  m_node = NSMNode(cfile.get("nsm.nodename"));
  int interval = cfile.getInt("hv.interval");
  init(false, interval);
  sleep(1);
  m_nsm = new HVTUICallback(NSMNode(cuiname), this);
  m_nsm->setStandbyConfig(config);
  m_nsm->addAll(config);
  PThread(new NSMNodeDaemon(m_nsm, cfile.get("nsm.host"),
                            cfile.getInt("nsm.port")));
}

HVTUINSM::~HVTUINSM() throw()
{
}

void HVTUINSM::store(int index) throw(IOException)
{
  NSMVar var("store", index);
  NSMCommunicator::send(NSMMessage(m_node, var));
  getValueAll();
}

void HVTUINSM::recall(int index) throw(IOException)
{
  NSMVar var("recall", index);
  NSMCommunicator::send(NSMMessage(m_node, var));
  getValueAll();
}

std::string getVname(int crate, int slot, int channel)
{
  return StringUtil::form("crate[%d].slot[%d].channel[%d]", crate, slot, channel);
}

void HVTUINSM::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".rampup", rampup);
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".rampdown", rampdown);
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::setVoltageDemand(int crate, int slot, int channel, float vdemand) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".vdemand", vdemand);
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::setVoltageLimit(int crate, int slot, int channel, float vlimit) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".vlimit", vlimit);
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::setCurrentLimit(int crate, int slot, int channel, float climit) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".climit", climit);
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  NSMVar var(getVname(crate, slot, channel) + ".switch", (switchon ? "ON" : "OFF"));
  NSMCommunicator::send(NSMMessage(m_node, var));
}

void HVTUINSM::requestSwitch(int crate, int slot, int channel) throw(IOException)
{
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, getVname(crate, slot, channel) + ".switch"));
}

void HVTUINSM::requestRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].rampup",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].rampdown",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].vdemand",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].vlimit",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].climit",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestState(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].state",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].vmon",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

void HVTUINSM::requestCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  const std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].cmon",
                                             crate, slot, channel);
  NSMCommunicator::send(NSMMessage(m_node, NSMCommand::VGET, vname));
}

HVMessage HVTUINSM::wait() throw(IOException)
{
  m_mutex.lock();
  while (m_msg_l.empty()) {
    m_cond.wait(m_mutex);
  }
  HVMessage msg = m_msg_l.front();
  m_msg_l.pop_front();
  m_mutex.unlock();
  return msg;
}

void HVTUINSM::HVTUICallback::push(const HVMessage& msg)
{
  m_tui->m_mutex.lock();
  m_tui->m_msg_l.push_back(msg);
  m_tui->m_cond.signal();
  m_tui->m_mutex.unlock();
}

void HVTUINSM::HVTUICallback::init(NSMCommunicator&) throw()
{
  m_tui->getValueAll();
}

void HVTUINSM::HVTUICallback::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::RAMPUP_SPEED, rampup));
}

void HVTUINSM::HVTUICallback::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::RAMPDOWN_SPEED, rampdown));
}

void HVTUINSM::HVTUICallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::VOLTAGE_DEMAND, voltage));
}

void HVTUINSM::HVTUICallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::VOLTAGE_LIMIT, voltage));
}

void HVTUINSM::HVTUICallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::CURRENT_LIMIT, current));
}

void HVTUINSM::HVTUICallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::SWITCH, switchon));
}

void HVTUINSM::HVTUICallback::setState(int crate, int slot, int channel, int state) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::STATE, state));
}

void HVTUINSM::HVTUICallback::setVoltageMonitor(int crate, int slot, int channel, float voltage) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::VOLTAGE_MON, voltage));
}

void HVTUINSM::HVTUICallback::setCurrentMonitor(int crate, int slot, int channel, float current) throw(IOException)
{
  push(HVMessage(crate, slot, channel, HVMessage::CURRENT_MON, current));
}
