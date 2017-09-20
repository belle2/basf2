#include "daq/slc/hvcontrol/bklm/BklmHVControlCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/nsm/NSMNodeDaemon.h"

#include <daq/slc/system/PThread.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/hvcontrol/HVConfig.h>
#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include "daq/slc/system/LogFile.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <cstring>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
//#include "daq/slc/hvcontrol/bklm/caen/MainWrapp.h"
#include "daq/slc/hvcontrol/bklm/caen/CAENHVWrapper.h"
//#include "daq/slc/hvcontrol/bklm/caen/console.h"
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <ios>
#include <fstream>
#include <iostream>
#include <vector>

using namespace Belle2;
using std::setprecision;
using std::cout;
using std::ios;
using std::endl;
using std::vector;

// constructor
/* BklmHVControlCallback::BklmHVControlCallback() throw() : HVControlCallback(NSMNode())
   {
    link = LINKTYPE_TCPIP;
   }
*/

void BklmHVControlCallback::initialize(const HVConfig& hvconf) throw()
{
  ConfigFile config("slowcontrol", "hvcontrol/bklm");

  m_ChNum = 1; //always perform on one channel
  m_intlock = false;
  m_sysHndl = -1; // For crate#
  // link = LINKTYPE_TCPIP;
  // sysType = CAENHV_SYSTEM_TYPE_t(0);// SY1527:0; SY2527:1; SY4527:2; SY5527:3;

  int ncrt = config.getInt(StringUtil::form("hv.ncrate"));
  std::cout << "\n" << ncrt << " crates to be initialized!!! " << endl;
  if (ncrt > 100)std::cout << "crate numer >100!!! Need to modify the BklmHVControlCallback.h" << endl;

  m_logsave = false; //true: save log file; false: not save
  std::string log = config.get(StringUtil::form("hv.log"));
  std::string sv("save");
  if (log.compare(sv) == 0) m_logsave = true;
  m_tdiff = config.getInt(StringUtil::form("hv.tmdif")); // record every tdiff seconds.
  m_tmwind = config.getInt(StringUtil::form("hv.tmwind"));

  // m_ttt[0]=-1;
  for (int i = 1; i < ncrt + 1; i++) {
    std::string crateip = StringUtil::form("crate[%i].host", i);
    std::string crate_ip = config.get(crateip.c_str());
    strcpy(m_ipadd, crate_ip.c_str());

    std::cout << "\nCrate#" << i << ", IP = " << m_ipadd << std::endl;

    int checksame = 0; // check whether same physical crate(s) used, i.e., with the same IP

    for (int j = 1; j < i; j++) {
      std::string crateip0 = StringUtil::form("crate[%i].host", j);
      std::string crate_ip0 = config.get(crateip0.c_str());
      if (crate_ip == crate_ip0) {
        std::cout << "Same IP with Crate #" << j << endl;
        checksame = 1;
        m_ttt[i] = m_ttt[j];
      }
    }
    if (checksame == 1)continue;
    std::string cratetype = StringUtil::form("crate[%i].type", i);
    std::string crate_type = config.get(cratetype.c_str());
    m_sysType = CAENHV_SYSTEM_TYPE_t(atoi(crate_type.c_str()));

    std::string crateuser = StringUtil::form("crate[%i].user", i);
    std::string crate_user = config.get(crateuser.c_str());
    strcpy(m_userName, crate_user.c_str());

    std::string cratepw = StringUtil::form("crate[%i].user", i);
    std::string crate_pw = config.get(cratepw.c_str());
    strcpy(m_passwd, crate_pw.c_str());

    m_ret = CAENHV_InitSystem(m_sysType, m_link, m_ipadd, m_userName, m_passwd, &m_sysHndl);

    printf("\nCAENHV_InitSystem: %s (num. %d)\n\n", CAENHV_GetError(m_sysHndl), m_ret);
    int nm = m_sysHndl;

    m_ttt[i] = nm;
  }
  if (m_logsave) {
    m_start_t = time(0);   // get time now
    time_t t = time(0);
    struct tm* now = localtime(& t);
    std::ofstream ofs;
    strftime(m_buffer, 80, "monitor-log/%Y-%m-%d-%I-%M%p.log", now);
    std::cout << "Save Volts and Currents to " << m_buffer << std::endl;
    std::cout << "Vol and Cur will be recorded in every " << m_tdiff << " seconds, in the window of "
              << m_tmwind << " seconds." << endl;

    ofs.open(m_buffer, std::ofstream::out | std::ofstream::app);
    // ofs.open ("test.txt", std::ofstream::out | std::ofstream::app);
    ofs << "\n***** start HV at " << asctime(now) << "*****" << "\n"
        << " Crate	Slot	Channel	Time(m)		Volt	Current" << std::endl;
    ofs.close();
  }

  // Let's do some check
  // myconfigure();
  // check();
  // if(check_all_switch(1)) {LogFile::debug("initialization status: all hv now is on.");}
  // else {
  // if(check_all_switch(0)) {LogFile::debug("initialization status: all hv now is off.");}
  // else {LogFile::debug("initialization status: some channels are on and some are off.");}
  // }

  //add channel name as NSM variable
  const HVCrateList& crate_v(hvconf.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    int crateid = crate.getId();
    for (size_t i = 0; i < channel_v.size(); i++) {
      const HVChannel& channel(channel_v[i]);
      int slot = channel.getSlot();
      int ch = channel.getChannel();
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d]", crateid, slot, ch);
      add(new NSMVHandlerText(vname + ".name", true, false, getChName(crateid, slot, ch)));
    }
  }
}

/*performed at the interval of hv.interval*/
// void BklmHVControlCallback::update() throw(HVHandlerException)
// {
// }

/*set switch of channel to ON or OFF*/
void BklmHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
// LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
//               crate, slot, channel, (switchon ? "ON" : "OFF"));
// if(m_intlock) {LogFile::debug("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
//               crate, slot, channel, "OFF");}
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];
  int fParVal2 = 0;
  strcpy(m_ParName, "Pw");

  if (switchon) {
    fParVal2 = 1;
  } else {
    fParVal2 = 0;
  }
  if (m_intlock) {
    fParVal2 = 0;
  }

  //const HVConfig& config(getConfig());
  //if(m_sw[crate-1][Slot][m_ChList]==false)
  //{
  //LogFile::debug("crate%i.slot%i.channel%i is set false, do not turn it on. Check your config. table in ", crate, slot,channel); return;
  //}

  //int fParValList2;
  //CAENHV_GetChParam(m_handle, Slot ,"Status", m_ChNum, &m_ChList, &fParValList2);
  //if(switchon&&fParValList2&1<<1) { LogFile::debug("crate%i.slot%i.channel%i is RAMPUP", crate, slot,channel); return; }//ramping up
  //if(switchon&&fParValList2&1<<0) { LogFile::debug("crate%i.slot%i.channel%i is alreday on",crate, slot,channel); return; }
  //if((!switchon)&&!(fParValList2&1<<0)) { LogFile::debug("crate%i.slot%i.channel%i is alreday off",crate, slot,channel); return;}
  //if((!switchon)&&fParValList2&1<<1) { LogFile::debug("crate%i.slot%i.channel%i is RAMPDOWN",crate, slot,channel); return;}
  //if((!switchon)&&fParValList2&1<<0) { LogFile::debug("crate%i.slot%i.channel%i is on, you are turning it off",crate, slot,channel); return;}

  if (fParVal2 == 1) {
    if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &fParVal2) == CAENHV_OK) {
      LogFile::info("switch crate%d.slot%d.channel%d %s ", crate, slot, channel, "ON");
    } else
    { LogFile::error("error in writing Pw crate%d.slot%d.channel%d %s", crate, slot, channel, CAENHV_GetError(m_handle)); }
  } else {
    if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &fParVal2) == CAENHV_OK) {
      LogFile::info("switch crate%d.slot%d.channel%d %s ", crate, slot, channel, "OFF");
    } else
    { LogFile::error("error in writing Pw crate%d.slot%d.channel%i %s", crate, slot, channel, CAENHV_GetError(m_handle)); }
  }
  //sleep(5);
  return;
}

/*set voltage ramp up speed with unit of [V]*/
void BklmHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  //LogFile::info("setRampup called : crate = %i, slot = %i, channel = %i, voltage: %f",
  //             crate, slot, channel, voltage);

  strcpy(m_ParName, "RUp");
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_fParVal = voltage;
  m_handle = m_ttt[crate];

  if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParVal) == CAENHV_OK) {
    LogFile::info("set rampup speed crate%d.slot%d.channle%d %f [V/sec]", crate, slot, channel, m_fParVal);
  } else {
    LogFile::error("error in writing RUp of crate%d.slot%d.channel%d %s", crate, slot, channel, CAENHV_GetError(m_handle));
  }
  return;
}

/*set voltage ramp down speed with unit of [V]*/
void BklmHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  //LogFile::info("setRampdown called : crate = %i, slot = %i, channel = %i, voltage: %f",
  //             crate, slot, channel, voltage);

  strcpy(m_ParName, "RDWn");
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_fParVal = voltage;
  m_handle = m_ttt[crate];

  if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParVal) == CAENHV_OK) {
    LogFile::info("set rampdown speed of crate%d.slot%d.channel%d %f [V/sec]", crate, slot, channel, m_fParVal);

  } else {
    LogFile::error("error in writing RDWn of crate%d.slot%d.channel%d %s", crate, slot, channel, CAENHV_GetError(m_handle));
  }
  return;
}

/*set demand voltage with unit of [V]*/
void BklmHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  //LogFile::info("setvoltageDemand called : crate = %i, slot = %i, channel = %i, voltage: %f",
  //              crate, slot, channel, voltage);

  strcpy(m_ParName, "V0Set");
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_fParVal = voltage;
  m_handle = m_ttt[crate];

  if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParVal) == CAENHV_OK) {
    LogFile::info("set Vdemand(V0Set) of crate%d.slot%d.channel%d %f [V]", crate, slot, channel, m_fParVal);
  } else {

    LogFile::error("error in writing V0Set of crate%d.slot%d.channel%d to be %f, %s ", crate, slot, channel, m_fParVal,
                   CAENHV_GetError(m_handle));
  }
  return;
}

/*set voltage limit with unit of [V]*/
void BklmHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  //LogFile::info("setvoltagelimit called : crate = %i, slot = %i, channel = %i, voltage: %f",
  //              crate, slot, channel, voltage);
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_fParVal = voltage;
  m_handle = m_ttt[crate];
  strcpy(m_ParName, "SVMax");

  if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParVal) == CAENHV_OK) {
    LogFile::info("set Vlimit(SVMax) of crate%d.slot%d.channel%d to be %f [V]", crate, slot, channel, m_fParVal);
  } else {
    LogFile::error("error in writing SVMax of crate%d.slot%d.channel%d to be %f, %s ", crate, slot, channel, m_fParVal,
                   CAENHV_GetError(m_handle));
  }
  return;
}

/*set current limit with unit of [uA]*/
void BklmHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  //LogFile::info("setcurrentlimit called : crate = %i, slot = %i, channel = %i, voltage: %f",
  //              crate, slot, channel, current);

  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_fParVal = current;
  m_handle = m_ttt[crate];
  strcpy(m_ParName, "I0Set");

  if (CAENHV_SetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParVal) == CAENHV_OK) {
    LogFile::info("set Ilimit(I0Set) of crate%d.slot%d.channel%d to be %f [V]", crate, slot, channel, m_fParVal);
  } else {
    LogFile::error("error in writing I0Set of crate%d.slot%d.channel%d to be %f, %s ", crate, slot, channel, m_fParVal,
                   CAENHV_GetError(m_handle));
  }
  return;
}

/*return name of the channel*/
std::string BklmHVControlCallback::getChName(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  char name[MAX_CH_NAME];
  if (CAENHV_GetChName(m_handle, m_Slot, m_ChNum, &m_ChList, &name) == CAENHV_OK) {
    LogFile::info("get channel name of crate%d.slot%d.channel%d ", crate, slot, channel);
    return name;
  } else {
    LogFile::error("error in reading channel name of crate%d.slot%d.channel%d to be %f, %s ", crate, slot, channel,
                   CAENHV_GetError(m_handle));
    return "";
  }
  return "";
}

/*return voltage ramp up speed with unit of [V]*/
float BklmHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "RUp");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    // LogFile::info("get ramp up speed  of crate%d.slot%d.channel%d ", crate, slot, channel);
    return m_fParValList;
  } else {
    LogFile::error("error in reading RUp of crate%d.slot%d.channel%d, %s ", crate, slot, channel, CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return voltage ramp down speed with unit of [V]*/
float BklmHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "RDWn");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    // LogFile::info("get ramp down speed  of crate%d.slot%d.channel%d ", crate, slot, channel);
    return m_fParValList;
  } else {
    LogFile::error("error in reading RDWn of crate%d.slot%d.channel%d, %s ", crate, slot, channel, CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return demand voltage with unit of [V]*/
float BklmHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "V0Set");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    // LogFile::info("reading V0Set  of crate%d.slot%d.channel%d ", crate, slot, channel);
    return m_fParValList;
  } else {
    LogFile::error("error in reading V0Set of crate%d.slot%d.channel%d, %s ", crate, slot, channel, CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return voltage limit with unit of [V]*/
float BklmHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "SVMax");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    // LogFile::info("reading SVMax  of crate%d.slot%d.channel%d ", crate, slot, channel);
    return m_fParValList;
  } else {
    LogFile::error("error in reading SVMax of crate%d.slot%d.channel%d, %s ", crate, slot, channel, CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return current limit with unit of [uA]*/
float BklmHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "I0Set");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    // LogFile::info("reading I0Set  of crate%d.slot%d.channel%d ", crate, slot, channel);
    return m_fParValList;
  } else {
    LogFile::error("error in reading I0Set of crate%d.slot%d.channel%d, %s ", crate, slot, channel, CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return monitored voltage with unit of [V]*/
float BklmHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;

  strcpy(m_ParName, "VMon");
  m_handle = m_ttt[crate];

  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    LogFile::info("reading VMon of carte%d.slot%d.channel%d, vom %f", crate, slot, channel, m_fParValList);
    if (m_logsave) {
      m_stop_t = time(0);
      m_diff = difftime(m_stop_t, m_start_t);
      float diffm = m_diff / 60.0;
      if (m_diff % m_tdiff > (m_tdiff - m_tmwind)) {
        std::ofstream ofs;
        ofs.open(m_buffer, std::ofstream::out | std::ofstream::app);
        ofs.setf(ios::fixed | ios::showpoint);
        ofs << " " << crate << "	" << slot << "	" << channel << "	 " << setprecision(1) << diffm
            << "		" << setprecision(1) << m_fParValList;
        ofs.close();
      }
    }

    /*if(!m_intlock && slot>12 && m_fParValList>75.0)
    {m_intlock=true;
    struct tm * now = localtime( & m_stop_t );
         std::ofstream ofs;
        ofs.open(m_buffer,std::ofstream::out | std::ofstream::app);
          ofs.setf(ios::fixed|ios::showpoint);
         ofs<<"INTERLOCK!!! \n" <<"Crate#"<<crate<<" m_Slot#"<<slot<<" Channel#"<<channel<<"couse the InterLock at "<<asctime(now);
          LogFile::debug("INTERLOCK!!!, Crate%i slot %i, channel %i cause the InterLock ",crate,slot,channel);
        ofs.close();
            }
          if(m_intlock) LogFile::debug("interlock now is on");
    if(m_intlock)setSwitch(crate,m_Slot,channel,m_intlock);
          */
    //sleep(5);
    return m_fParValList;
  } else {
    LogFile::error("error in reading VMon crate%d.slot%d.channel%d %s", crate, slot, channel,  CAENHV_GetError(m_handle));
    return -1;
  }
  return 0.;
}

/*return monitored current with unit of [uA]*/
float BklmHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "IMon");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &m_fParValList) == CAENHV_OK) {
    LogFile::info("reading IMon of carte%d.slot%d.channel%d, iom %f", crate, slot, channel, m_fParValList);
    //m_stop_t = time(0);
    //diff = difftime (m_stop_t,start_t);
    if (m_logsave) {
      if (m_diff % m_tdiff > (m_tdiff - m_tmwind)) {
        std::ofstream ofs;
        ofs.open(m_buffer, std::ofstream::out | std::ofstream::app);
        ofs.setf(ios::fixed | ios::showpoint);
        ofs << "	 " << setprecision(1) << m_fParValList << std::endl;

        ofs.close();
      }
    }
    return m_fParValList;
  } else {
    LogFile::error("error in reading IMon crate%d.slot%d.channel%d %s", crate, slot, channel,  CAENHV_GetError(m_handle));
    return -1;
  }
  return 0;
}

/*return status of channel swicth (true:ON, false:OFF)*/
bool BklmHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];
  int fParValList2 = -1;
  strcpy(m_ParName, "Pw");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &fParValList2) == CAENHV_OK) {
    LogFile::info("get switch of crate%d.slot%d.channel%d %d", crate, slot, channel, fParValList2);
    return fParValList2;
  } else {
    LogFile::error("error in reading PW of crate%d.slot%d.channle%d %s", crate, slot, channel,  CAENHV_GetError(m_handle));
    return false;
  }
  return false;
}

int BklmHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  /*
    return channel status:
    OFF: power off,
    ON: power on without error
    OCP: trip due to over current
    OVP: trip due to over voltage
    ERR: another error
  */
  int  fParValList2;
  m_Slot = slot - 1;
  m_ChList = channel - 1;
  m_handle = m_ttt[crate];

  strcpy(m_ParName, "Status");
  if (CAENHV_GetChParam(m_handle, m_Slot, m_ParName, m_ChNum, &m_ChList, &fParValList2) == CAENHV_OK) {
    //LogFile::info("read Status of crate%d.slot%d.channel%d", crate, slot, channel );
    if (fParValList2 & 1 << 7) {return HVMessage::ERR;} //in VMax
    else if (fParValList2 & 1 << 6) {return HVMessage::ERR;} //in external trip
    else if (fParValList2 & 1 << 5) {return HVMessage::ERR;} //under Vdemand
    else if (fParValList2 & 1 << 4) {return HVMessage::OVP;} //over voltage
    else if (fParValList2 & 1 << 3) {return HVMessage::OCP;} //overcurrent
    else if (fParValList2 & 1 << 2) {return HVMessage::RAMPDOWN;} //ramping down
    else if (fParValList2 & 1 << 1) {return HVMessage::RAMPUP;} //ramping up
    else if (fParValList2 & 1 << 0) return HVMessage::ON; //On
    else if (!(fParValList2 & 1 << 0)) return HVMessage::OFF; //Off
    else return HVMessage::ERR;
  } else {
    LogFile::error("error in reading Status crate%d.slot%d.channle%d %s", crate, slot, channel,  CAENHV_GetError(m_handle));
  }
  return 0;
}

