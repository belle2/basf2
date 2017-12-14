#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"
#include <daq/slc/hvcontrol/arich/ArichHVControlCallback.h>
#include <daq/slc/hvcontrol/arich/zlibstream.h>
#include <daq/slc/hvcontrol/arich/hvinfo.h>

#include <daq/slc/hvcontrol/HVConfig.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/hvcontrol/arich/CAENHVWrapper.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/nsm/NSMVHandler.h>

#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <iostream>
#include <list>

#define Recovery
#define Recovery_Function
#define all_sw
#define check_all


namespace Belle2 {

  class NSMVArichHVClearAlarm : public NSMVHandlerInt {
  public:
    NSMVArichHVClearAlarm(ArichHVControlCallback& callback,
                          const std::string& name, int crate)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback), m_crate(crate) {}
    virtual ~NSMVArichHVClearAlarm() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.clearAlarm(m_crate);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
  };

  class NSMVArichHVAllGuardOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllGuardOn(ArichHVControlCallback& callback,
                          const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_guard_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllGuardOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllGuardOff(ArichHVControlCallback& callback,
                           const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_guard_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllHVOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllHVOn(ArichHVControlCallback& callback,
                       const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_hv_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllHVOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllHVOff(ArichHVControlCallback& callback,
                        const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_hv_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllBiasOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllBiasOn(ArichHVControlCallback& callback,
                         const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_bias_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllBiasOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllBiasOff(ArichHVControlCallback& callback,
                          const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_bias_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllGuardOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllGuardOn(ArichHVControlCallback& callback,
                               const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_guard_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllGuardOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllGuardOff(ArichHVControlCallback& callback,
                                const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_guard_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllHVOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOn(ArichHVControlCallback& callback,
                            const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_hv_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllHVOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOff(ArichHVControlCallback& callback,
                             const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_hv_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllBiasOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOn(ArichHVControlCallback& callback,
                              const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_bias_on();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllBiasOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOff(ArichHVControlCallback& callback,
                               const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.check_all_bias_off();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };


}

using namespace Belle2;

void ArichHVControlCallback::clearAlarm(int crate) throw(IOException)
{

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crate == 0 || crateid == crateid) {
      int ret = -1;
      int handle = m_handle[i];
      std::cout << "clear Alarm" << std::endl;
      if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
        LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
      }  else {
        LogFile::info("Clear Alarm was done: %d", handle);
      }
    }
  }
}


void ArichHVControlCallback::addAll(const HVConfig& config) throw()
{
  //  const HVConfig& config(getConfig());
  HVCallback::addAll(config);
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    //    const HVChannelList& channel_v(crate.getChannels());
    int crateid = crate.getId();
    std::string vname1 = StringUtil::form("crate[%d].calarm", crateid);
    add(new NSMVArichHVClearAlarm(*this, vname1, crateid));

    std::string vname2 = StringUtil::form("allguardon");
    add(new NSMVArichHVAllGuardOn(*this, vname2));
    std::string vname3 = StringUtil::form("allguardoff");
    add(new NSMVArichHVAllGuardOff(*this, vname3));
    std::string vname4 = StringUtil::form("allhvon");
    add(new NSMVArichHVAllHVOn(*this, vname4));
    std::string vname5 = StringUtil::form("allhvoff");
    add(new NSMVArichHVAllHVOff(*this, vname5));
    std::string vname6 = StringUtil::form("allbiason");
    add(new NSMVArichHVAllBiasOn(*this, vname6));
    std::string vname7 = StringUtil::form("allbiasoff");
    add(new NSMVArichHVAllBiasOff(*this, vname7));

    std::string vname8 = StringUtil::form("checkallguardon");
    add(new NSMVArichHVCheckAllGuardOn(*this, vname8));
    std::string vname9 = StringUtil::form("checkallguardoff");
    add(new NSMVArichHVCheckAllGuardOff(*this, vname9));
    std::string vname10 = StringUtil::form("checkallhvon");
    add(new NSMVArichHVCheckAllHVOn(*this, vname10));
    std::string vname11 = StringUtil::form("checkallhvoff");
    add(new NSMVArichHVCheckAllHVOff(*this, vname11));
    std::string vname12 = StringUtil::form("checkallbiason");
    add(new NSMVArichHVCheckAllBiasOn(*this, vname12));
    std::string vname13 = StringUtil::form("checkallbiasoff");
    add(new NSMVArichHVCheckAllBiasOff(*this, vname13));

  }
}

void ArichHVControlCallback::initialize(const HVConfig& hvconf) throw()
{
  /*  const unsigned short *slotlist = 0;
  float *temp;
  if ((ret = CAENHV_GetBdParam(handle, 2, slotlist, "Temp", &temp)) != CAENHV_OK) {
    LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
  }  else {
    LogFile::info("temp is: %d", temp);
  }
  */

  /*
  unsigned short  NrOfSlot;
  unsigned short * NrOfChList;
  char * ModelList;
  char * DescriptionList;
  unsigned short * SerNumList;
  unsigned char * FmwRelMinList;
  unsigned char * FmwRelMaxList;
  CAENHV_GetCrateMap(handle, &NrOfSlot, &NrOfChList, &ModelList, &DescriptionList, &SerNumList, &FmwRelMinList, &FmwRelMaxList);
  */
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  m_handle = std::vector<int>();
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& crate(crate_v[i]);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    //    const std::string host0 = crate.getHost(); //should be used
    //    std::cout << "host0 = " << host0 << std::endl;
    char host[256];
    strcpy(host, "192.168.0.1");
    const char* user = "admin";
    const char* pass = "admin";
    int handle;
    int ret = 0;
    if ((ret = CAENHV_InitSystem(SY4527, LINKTYPE_TCPIP, host, user, pass, &handle)) != CAENHV_OK) {
      LogFile::error("Error on initialization: %d %s", ret, CAENHV_GetError(handle));
    }  else {
      LogFile::info("handler: %d", handle);
    }
    m_handle.push_back(handle);
    if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
      LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
    }  else {
      LogFile::info("Clear Alarm was done: %d", handle);
    }

    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
      paramLock();
      // do something
      paramUnlock();
    }
  }
  LogFile::debug("initialize : done");
  HVControlCallback::configure(hvconf);

}

void ArichHVControlCallback::deinitialize(int handle) throw()
{
  /*
  int ret = 0;
  if ( (ret = CAENHV_DeinitSystem(handle)) != CAENHV_OK) {
    LogFile::error("Error on deinitialization: %d", ret);
  }  else {
    LogFile::info("handler: %d", handle);
  }
  */
}

int g_day = -1;
zlibstream g_zs;

void ArichHVControlCallback::update() throw()
{

#ifdef Recovery
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  int state[10][100][100];
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& c_crate(crate_v[i]);
    const HVChannelList& channel_v(c_crate.getChannels());
    int crate = crate_v[i].getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& ch(*ichannel);
      int slot = ch.getSlot();
      int channel = ch.getChannel();
      int handle = m_handle[i];
      //      unsigned short chan = channel;
      //      unsigned Status;
      paramLock();
      state[crate][slot][channel] = getState(crate, slot, channel);//(int)unit.getStatus().getState();
      paramUnlock();
      //      std::cout<<state[1][0][4]<<std::endl;
      //      int ret = 0;

      if (state[crate][slot][channel] & (1 << 6)) {
        LogFile::warning("%d, %d, %d is external Trip", crate, slot, channel);
        //    return HVMessage::ETRIP;
      }
      if (state[crate][slot][channel] & (1 << 8)) {
        LogFile::notice("%d, %d, %d is external disable", crate, slot, channel);
        int recovered = -1;
        if (recovered != crate) {
          LogFile::debug("Run RecoveryInterlock");
          RecoveryInterlock(handle, crate, slot, channel);
          //    return HVMessage::INTERLOCK;
          recovered = crate;
        }
      }

      //  if(state[crate][slot][channel] & (1<<9)){
      if (state[crate][slot][channel] == 7) {
        LogFile::warning("%d, %d, %d is internal Trip", crate, slot, channel);
        LogFile::debug("Run RecoveryTrip");
        RecoveryTrip(handle, crate, slot, channel);
        //    return HVMessage::TRIP;
      }
    }
  }

  static unsigned long long count = 0;

#endif

  if (count % 5 == 0) {
    Date date;
    if (g_day != date.getDay()) {
      g_zs.close();
      g_zs = zlibstream();
      g_zs.open(StringUtil::form("/disk/data/datfiles/data/%s.dat", date.toString("%Y.%m.%d.%H.%M")).c_str(), "w");
      g_day = date.getDay();
    }
    const HVConfig& config(getConfig());
    const HVCrateList& crate_v(config.getCrates());
    //    int state[10][100][100];
    for (size_t i = 0; i < crate_v.size(); i++) {
      const HVCrate& c_crate(crate_v[i]);
      const HVChannelList& channel_v(c_crate.getChannels());
      int crate = crate_v[i].getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        const HVChannel& ch(*ichannel);
        int slot = ch.getSlot();
        int channel = ch.getChannel();
        //  int handle = m_handle[i];
        hvinfo info;
        info.record_time = (unsigned int)date.get();
        info.crate = crate;
        info.slot = slot;
        info.channel = channel;
        info.switchon = getState(info.crate, info.slot, info.channel);//(int)unit.getStatus().getState();
        info.vdemand = getVoltageDemand(info.crate, info.slot, info.channel);//param.getVoltageDemand();
        info.vmon = getVoltageMonitor(info.crate, info.slot, info.channel);//unit.getStatus().getVoltageMon();
        info.cmon = getCurrentMonitor(info.crate, info.slot, info.channel);//unit.getStatus().getCurrentMon();
        g_zs.write(info);
        if (getDB()) {
          DBInterface& db(*getDB());
          db.connect();
          /*
          db.execute("insert into hvinfo (record_time, crate, "
               "slot, channel, switchon, state, vmon, cmon, rampup, rampdown, vset, vlimit, climit) "
               "values (current_timestamp, %d, %d, %d, %d, %d, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f);",
               m_acomm[i].getId(), ch.getSlot(),
               ch.getChannel(), getSwitch(info.crate, info.slot, info.channel),
               info.switchon, info.vmon, info.cmon, rampup,rampdown, vdemand, vlimit, climit);
          */
          //reply(NSMMessage(NSMCommand::ERROR, "error"));
        }
      }
    }
    g_zs.flush();
  }
  count++;
}


void ArichHVControlCallback::configure(const HVConfig& config) throw(HVHandlerException)
{
  try {
    const HVConfig& config(getConfig());
    const HVCrateList& crate_v(config.getCrates());
    for (HVCrateList::const_iterator icrate = crate_v.begin();
         icrate != crate_v.end(); icrate++) {
      const HVCrate& crate(*icrate);
      const HVChannelList& channel_v(crate.getChannels());
      //      int crateid = crate.getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        //        const HVChannel& channel(*ichannel);
        //        int slot = channel.getSlot();
        //        int ch = channel.getChannel();
      }
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
    throw (HVHandlerException(e.what()));
  }
}


void ArichHVControlCallback::store(int index) throw(IOException)
{
  //  for (size_t i = 0; i < m_acomm.size(); i++) {
  //m_acomm[i].store(index);
  //m_acomm[i].requestValueAll(0, 0);
  //  }
}

void ArichHVControlCallback::recall(int index) throw(IOException)
{
  //  for (size_t i = 0; i < m_acomm.size(); i++) {
  //m_acomm[i].recall(index);
  //m_acomm[i].requestValueAll(0, 0);
  //  }
}

/*
    for (size_t i = 0; i < m_acomm.size(); i++) {
      for (int j = 0; j < m_acomm[i].getNUnits(); j++) {
  ArichHVUnit& unit(m_acomm[i].getUnit(j));
  HVValue& param(unit.getValue());
  HVChannel& ch(unit.getChannel());
  if (unit.getStatus().getState() == 1) {
    std::cout << m_acomm[i].getId() << " "
        << ch.getSlot() << " "
        << ch.getChannel() << " "
        << unit.getStatus().getVoltageMon() << " "
        << unit.getStatus().getCurrentMon() << std::endl;
  }
      }

*/


void ArichHVControlCallback::turnon() throw(HVHandlerException)
{
  LogFile::debug("start turn on");

  for (int i = 0; i < 16; i++)setVoltageDemand(1, 3, i, 1000); // for debug

  while (!(check_all_switch("guard", 1) && check_all_switch("hv", 1) && check_all_switch("bias", 1))) {
    if (check_all_switch("guard", 0) && check_all_switch("hv", 0) && check_all_switch("bias", 0)) all_switch("guard", true);
    else if (check_all_switch("guard", 1) && check_all_switch("hv", 0) && check_all_switch("bias", 0)) all_switch("hv", true);
    else if (check_all_switch("guard", 1) && check_all_switch("hv", 1) && check_all_switch("bias", 0)) all_switch("bias", true);
    else {
      //  LogFile::debug("you should check");
    }
    wait(5);
  }
  LogFile::debug("finish turn on");

}


void ArichHVControlCallback::turnoff() throw(HVHandlerException)
{
  LogFile::debug("start turn off");

  while (!(check_all_switch("guard", 0) && check_all_switch("hv", 0) && check_all_switch("bias", 0))) {
    if (check_all_switch("guard", 1) && check_all_switch("hv", 1) && check_all_switch("bias", 1)) all_switch("bias", false);
    else if (check_all_switch("guard", 1) && check_all_switch("hv", 1) && check_all_switch("bias", 0)) all_switch("hv", false);
    else if (check_all_switch("guard", 1) && check_all_switch("hv", 0) && check_all_switch("bias", 0)) all_switch("guard", false);
    else {
      //  LogFile::debug("you should check");
    }

    wait(5);
  }
  LogFile::debug("finish turn off");
}

void ArichHVControlCallback::standby() throw(HVHandlerException)
{
  std::cout << "start standby" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish standby" << std::endl;
}

void ArichHVControlCallback::shoulder() throw(HVHandlerException)
{
  std::cout << "start shoulder" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish shoulder" << std::endl;
}

void ArichHVControlCallback::peak() throw(HVHandlerException)
{
  std::cout << "start peak" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish peak" << std::endl;
}


#ifdef all_sw
void ArichHVControlCallback::all_guard_off() throw(HVHandlerException)
{
  all_switch("guard", false);
}
void ArichHVControlCallback::all_guard_on() throw(HVHandlerException)
{
  all_switch("guard", true);
}
void ArichHVControlCallback::all_bias_off() throw(HVHandlerException)
{
  all_switch("bias", false);
}
void ArichHVControlCallback::all_bias_on() throw(HVHandlerException)
{
  all_switch("bias", true);
}
void ArichHVControlCallback::all_hv_off() throw(HVHandlerException)
{
  all_switch("hv", false);
}
void ArichHVControlCallback::all_hv_on() throw(HVHandlerException)
{
  all_switch("hv", true);
}

void ArichHVControlCallback::all_switch(std::string set_type, bool sw) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  std::string m_type = "";
  //error
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    //error
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type.find(set_type) != std::string::npos) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        setSwitch(crateid, slot, ch, sw);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}

#endif

#ifdef check_all
bool ArichHVControlCallback::check_all_bias_on() throw(HVHandlerException)
{
  if (check_all_switch("bias", 1)) {
    std::cout << "all bias are on" << std::endl;
    return true;
  } else {
    std::cout << "all bias are NOT on" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_bias_off() throw(HVHandlerException)
{
  if (check_all_switch("bias", 0)) {
    std::cout << "all bias are off" << std::endl;
    return true;
  } else {
    std::cout << "all bias are NOT off" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_hv_on() throw(HVHandlerException)
{
  if (check_all_switch("hv", 1)) {
    std::cout << "all hv are on" << std::endl;
    return true;
  } else {
    std::cout << "all hv are NOT on" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_hv_off() throw(HVHandlerException)
{
  if (check_all_switch("hv", 0)) {
    std::cout << "all hv are off" << std::endl;
    return true;
  } else {
    std::cout << "all hv are NOT off" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_guard_on() throw(HVHandlerException)
{
  if (check_all_switch("guard", 1)) {
    std::cout << "all guard are on" << std::endl;
    return true;
  } else {
    std::cout << "all guard are NOT on" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_guard_off() throw(HVHandlerException)
{
  if (check_all_switch("guard", 0)) {
    std::cout << "all guard are off" << std::endl;
    return true;
  } else {
    std::cout << "all guard are NOT off" << std::endl;
    return false;
  }
}

bool ArichHVControlCallback::check_all_switch(std::string set_type, int sw) throw(HVHandlerException)
{
  //  return true;

  //  printf("start check all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  std::string m_type = "";
  //error
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    //error
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type.find(set_type) != std::string::npos) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //        printf("getState(%d, %d, %d) %s = %d\n",crateid,slot,ch,type.c_str(), getState(crateid,slot,ch));
        //  return true;

        if (getState(crateid, slot, ch) == sw) {
          continue;
        } else {
          return false;
        }

      }

    }
  }

  //  printf("finish check all %s switch %d\n",set_type.c_str(),sw);
  return true;
  //  return false;
}
#endif


#ifdef Recovery_Function
void ArichHVControlCallback::RecoveryTrip(int handle, int crate, int slot, int channel) throw(IOException)
{
  int ret = 0;
  bool switchon = 0;
  unsigned sw = switchon;

  int channel0 = 0;
  int channel1 = 1;
  int channel2 = 2;
  int channel3 = 3;


  unsigned short chan0 = channel0;
  unsigned short chan1 = channel1;
  unsigned short chan2 = channel2;
  unsigned short chan3 = channel3;


  //Bias tripped
  //HV tripped
  if (slot == 1 && channel == 0) {

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan0, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel0, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel0, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan1, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel1, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel1, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan2, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel2, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel2, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan3, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel3, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel3, ret, CAENHV_GetError(handle));
    }

    if (getState(1, 0, 0) == 0 && getState(1, 0, 1) == 0 && getState(1, 0, 2) == 0 && getState(1, 0, 3) == 0) {
      std::cout << "Down is done" << std::endl;
    }

  }
  //Guard tripped (Bais are should be killed, HV are ramp)

  if (slot == 0 && channel == 4) {

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan0, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel0, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel0, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan1, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel1, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel1, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan2, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel2, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel2, ret, CAENHV_GetError(handle));
    }

    if ((ret = CAENHV_SetChParam(handle, 0, "Pw", 1, &chan3, &sw)) == CAENHV_OK) {
      LogFile::info("switch %d.%d.%d %s", crate, 0, channel3, "Down");
    } else {
      LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 0, channel3, ret, CAENHV_GetError(handle));
    }


    if (getState(1, 0, 0) == 0 && getState(1, 0, 1) == 0 && getState(1, 0, 2) == 0 && getState(1, 0, 3) == 0) {
      if ((ret = CAENHV_SetChParam(handle, 1, "Pw", 1, &chan0, &sw)) == CAENHV_OK) {
        LogFile::info("switch %d.%d.%d %s", crate, 1, channel0, "Down");
      } else {
        LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, 1, channel0, ret, CAENHV_GetError(handle));
      }

    }
    if (getState(1, 0, 0) == 0 && getState(1, 0, 1) == 0 && getState(1, 0, 2) == 0 && getState(1, 0, 3) == 0
        && getState(1, 1, 0) == 0) {
      std::cout << "Down is done" << std::endl;
    }

  }




}

void ArichHVControlCallback::RecoveryInterlock(int handle, int crate, int slot, int channel) throw(IOException)
{
  int ret = 0;

  if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
    LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
  }  else {
    LogFile::info("Clear Alarm was done: %d", handle);
    std::cout << "recovery interlock" << std::endl;
  }

}

#endif

void ArichHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned sw = switchon;
      if (switchon) {
        if ((ret = CAENHV_SetChParam(handle, slot, "Pw", 1, &chan, &sw)) == CAENHV_OK) {
          LogFile::info("switch %d.%d.%d %s", crate, slot, channel, "ON");
        } else {
          LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, slot, channel, ret, CAENHV_GetError(handle));
        }
      } else {
        if ((ret = CAENHV_SetChParam(handle, slot, "Pw", 1, &chan, &sw)) == CAENHV_OK) {
          LogFile::info("switch %d.%d.%d %s", crate, slot, channel, "OFF");
        } else {
          LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, slot, channel, ret, CAENHV_GetError(handle));
        }
      }

    }
  }

}

void ArichHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, RUp: %f", crate, slot, channel, rampup);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      //      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "RUp", 1, &chan, &rampup) != CAENHV_OK) {
        LogFile::error("error in writing RUp %f %s", rampup, CAENHV_GetError(handle));
      }

      LogFile::info("rampup %d.%d.%d %f [V/sec]", crate, slot, channel, rampup);

    }
  }

}


void ArichHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, RDwn: %f", crate, slot, channel, rampdown);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      //      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "RDWn", 1, &chan, &rampdown) != CAENHV_OK) {
        LogFile::error("error in writing RDwn %f %s", rampdown, CAENHV_GetError(handle));
      }
      LogFile::info("rampdown %d.%d.%d %f [V/sec]", crate, slot, channel, rampdown);

    }
  }

}

void ArichHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, vdemand: %f", crate, slot, channel, voltage);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      //      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "V0Set", 1, &chan, &voltage) != CAENHV_OK) {
        LogFile::error("error in writing V0Set %f %s", voltage, CAENHV_GetError(handle));
      }
      LogFile::info("Vdemand %d.%d.%d %f [V]", crate, slot, channel, voltage);

    }
  }

}

void ArichHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, SVMAx: %f", crate, slot, channel, voltage);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      //      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "SVMax", 1, &chan, &voltage) != CAENHV_OK) {
        LogFile::error("error in writing SVMax %f %s", voltage, CAENHV_GetError(handle));
      }
      LogFile::info("Vlimit %d.%d%.%d %f [V] ", crate, slot, channel, voltage);

    }
  }

}

void ArichHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, I0Set: %f", crate, slot, channel, current);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      //      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "I0Set", 1, &chan, &current) != CAENHV_OK) {
        LogFile::error("error in writing I0Set %f %s", current, CAENHV_GetError(handle));
      }
      LogFile::info("Ilimit %d.%d.%d %f [uA]", crate, slot, channel, current);

    }
  }

}

float ArichHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Rup;
      //CAENHV_GetChParam(handle,slot,"Rup",channel,&chan,&Rupspeed);
      if ((ret = CAENHV_GetChParam(handle, slot, "RUp", 1, &chan, &Rup)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, Rup);
        return Rup;
      } else {
        LogFile::error("error in reading Rup %d.%d.%d %d %f %s", crate, slot, channel, ret, Rup, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float RDwn;
      //CAENHV_GetChParam(handle,slot,"RDwn",channel,&chan,&RDwnspeed);
      if ((ret = CAENHV_GetChParam(handle, slot, "RDWn", 1, &chan, &RDwn)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, RDwn);
        return RDwn;
      } else {
        LogFile::error("error in reading RDwn %d.%d.%d %d %f %s", crate, slot, channel, ret, RDwn, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float V0Set;
      //CAENHV_GetChParam(handle,slot,"Vmon",channel,&chan,&Vmon);
      if ((ret = CAENHV_GetChParam(handle, slot, "V0Set", 1, &chan, &V0Set)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, V0Set);
        return V0Set;
      } else {
        LogFile::error("error in reading V0Set %d.%d.%d %d %f %s", crate, slot, channel, ret, V0Set, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float SVMax;
      //CAENHV_GetChParam(handle,slot,"SVMax",channel,&chan,&SVMax);
      if ((ret = CAENHV_GetChParam(handle, slot, "SVMax", 1, &chan, &SVMax)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, SVMax);
        return SVMax;
      } else {
        LogFile::error("error in reading SVMax %d.%d.%d %d %f %s", crate, slot, channel, ret, SVMax, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float I0Set;
      //CAENHV_GetChParam(handle,slot,"Trip",channel,&chan,&Trip);
      if ((ret = CAENHV_GetChParam(handle, slot, "I0Set", 1, &chan, &I0Set)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, Trip);
        return I0Set;
      } else {
        LogFile::error("error in reading Trip %d.%d.%d %d %f %s", crate, slot, channel, ret, I0Set, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Vmon;
      if ((ret = CAENHV_GetChParam(handle, slot, "VMon", 1, &chan, &Vmon)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, Vmon);
        return Vmon;
      } else {
        LogFile::error("error in reading Vmon %d.%d.%d %d %f %s", crate, slot, channel, ret, Vmon, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

float ArichHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Imon;
      if ((ret = CAENHV_GetChParam(handle, slot, "IMon", 1, &chan, &Imon)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d %f", crate, slot, channel, Imon);
        return Imon;
      } else {
        LogFile::error("error in reading Imon %d.%d.%d %d %f %s", crate, slot, channel, ret, Imon, CAENHV_GetError(handle));
        return -1;
      }

    }
  }
  return 0;
}

bool ArichHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned PW;
      if ((ret = CAENHV_GetChParam(handle, slot, "Pw", 1, &chan, &PW)) == CAENHV_OK) {
        //  LogFile::info("getswitch %d.%d.%d %u", crate, slot, channel, PW);
        //  std::cout<<"switch = "<<PW<<std::endl;
        return PW == 1;
      } else {
        LogFile::error("error in reading PW %d.%d.%d %d %u %s", crate, slot, channel, ret, PW, CAENHV_GetError(handle));
        return false;
      }

    }
  }
  return false;
}

int ArichHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned Status = 0;
      if ((ret = CAENHV_GetChParam(handle, slot, "Status", 1, &chan, &Status)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d 0x%x", crate, slot, channel, Status);
        if (Status & (1 << 1)) {
          return HVMessage::RAMPUP;
        } else if (Status & (1 << 2)) {
          return HVMessage::RAMPDOWN;
        } else if (Status & (1 << 3)) {
          LogFile::error("%d, %d, %d is over current", crate, slot, channel);
          return HVMessage::OCP;
        } else  if (Status & (1 << 4)) {
          LogFile::error("%d, %d, %d is over voltage", crate, slot, channel);
          return HVMessage::OVP;
        } else if (Status & (1 << 5)) {
          LogFile::error("%d, %d, %d is under voltage", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 6)) {
          return HVMessage::ETRIP;
        } else  if (Status & (1 << 7)) {
          LogFile::error("%d, %d, %d is Max V %d", crate, slot, channel, Status);
          return HVMessage::ERR;
        } else if (Status & (1 << 8)) {
          return HVMessage::INTERLOCK;
        } else if (Status & (1 << 9)) {
          return HVMessage::TRIP;
        } else if (Status & (1 << 10)) {
          LogFile::error("%d, %d, %d is calibration error", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 11)) {
          LogFile::fatal("%d, %d, %d is unplugged", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 12)) {
          LogFile::error("%d, %d, %d is error", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 13)) {
          LogFile::error("%d, %d, %d is Over Voltage Protection", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 14)) {
          LogFile::error("%d, %d, %d is Power Fail", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 15)) {
          LogFile::error("%d, %d, %d is Temperature Error", crate, slot, channel);
          return HVMessage::ERR;
        } else if (Status & (1 << 0)) {
          return HVMessage::ON;
        } else if (!(Status & (1 << 0))) {
          return HVMessage::OFF;
        }
      } else {
        LogFile::error("error in reading Status %d.%d.%d %d %u %s", crate, slot, channel, ret, Status, CAENHV_GetError(handle));
      }

    }
  }
  return 0;
}
