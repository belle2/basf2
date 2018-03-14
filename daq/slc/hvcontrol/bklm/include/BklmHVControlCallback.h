/****************************************************************************
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Xiaolong Wang, Yinghui Guan                              *
*
* This software is provided "as is" without any warranty.                *
***************************************************************************/

#ifndef _Belle2_BklmHVControlCallback_h
#define _Belle2_BklmHVControlCallback_h

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/nsm/NSMNodeDaemon.h"
#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/hvcontrol/HVControlCallback.h>
#include <daq/slc/hvcontrol/bklm/caen/CAENHVWrapper.h>
#include <ctime>
#include <vector>
#include <iomanip>

namespace Belle2 {

  class DBInterface;

  class BklmHVControlCallback : public HVControlCallback {

  public:
    //! constructor
    BklmHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {};
    //! destructor
    virtual ~BklmHVControlCallback() throw() {}

  public:
    /* initialize CAEN system*/
    virtual void initialize(const HVConfig& hvconfig) throw();

  public:
    /*set channel switch (true:ON, false:OFF)*/
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
    /*set voltage ramp up speed with unit of [V]*/
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
    /*set voltage ramp down speed with unit of [V]*/
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
    /*set demand voltage with unit of [V]*/
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
    /*set voltage limit with unit of [V]*/
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
    /*set current limit with unit of [uA]*/
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);

    /* get the channel name */
    std::string getChName(int crate, int slot, int channel) throw(IOException);
    /*return status of channel switch (true:ON, false:OFF)*/
    virtual bool getSwitch(int crate, int slot, int channel) throw(IOException);
    /*return voltage ramp up speed with unit of [V]*/
    virtual float getRampUpSpeed(int crate, int slot, int channel) throw(IOException);
    /*return voltage ramp down speed with unit of [V]*/
    virtual float getRampDownSpeed(int crate, int slot, int channel) throw(IOException);
    /*return demand voltage with unit of [V]*/
    virtual float getVoltageDemand(int crate, int slot, int channel) throw(IOException);
    /*return voltage limit with unit of [V]*/
    virtual float getVoltageLimit(int crate, int slot, int channel) throw(IOException);
    /*return current limit with unit of [uA]*/
    virtual float getCurrentLimit(int crate, int slot, int channel) throw(IOException);
    /*return state of channel (ERR, OVP, OCP, RAMPUP, RAMPDOWN, ON, OFF)*/
    virtual int getState(int crate, int slot, int channel) throw(IOException);
    /*return monitored voltage with unit of [V]*/
    virtual float getVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    /*return monitored current with unit of [uA]*/
    virtual float getCurrentMonitor(int crate, int slot, int channel) throw(IOException);

  private:
    //! IP address, username, password for the HV crates
    char m_ipadd[30], m_userName[30], m_passwd[30];
    //! link type
    static const int m_link = LINKTYPE_TCPIP;
    //! system handler
    int m_sysHndl;
    //! CAEN crates type SY1527:0; SY2527:1; SY4527:2; SY5527:3;
    CAENHV_SYSTEM_TYPE_t m_sysType;

    //! return result of get functions of CAEN system.
    CAENHVRESULT m_ret;

    //! save log file or not
    bool m_logsave;
    // int start_s;
    // int stop_s;
    //! start time
    time_t m_start_t;
    //! the time when make record for Voltage
    time_t m_stop_t;
    //! diff between start_t and stop_t
    time_t m_diff;
    //! make a record every tdiff seconds.
    int m_tdiff;
    //! time window between each record.
    int m_tmwind;

    //! enable/disalbe interlock
    bool m_intlock;

    //! std::ofstream ofs;
    char m_buffer [80];
    //! set slot number, number of channels (in fact always 1) and channel number when talking to CAEN crate
    unsigned short m_Slot, m_ChNum, m_ChList;
    //! Val that will be set
    float m_fParVal;
    //! Val that will be get
    float m_fParValList;
    //! Val name that will be set/get
    char m_ParName[30];
    //! crate handler
    int m_handle;
    //! handler  of crates
    int m_ttt[100];

    //protected:
    //! number of crate
    // int ncrate;
    //! index are crate, 10 crates at maximum
    // int nchannel[10];
    //! index are [crate] [channel] which are 0->1->2->3... continuous numbers
    //! m_slot itself is the slot number on HV hardware crate, could be not continuous numbers
    // int m_slot[10][100];
    //! index are [crate] [channel], m_channel itself is the channel numbe on HV hardware crate, could be not continuous numbers
    // int m_channel[10][100];
    //! index are crate, slot, channel
    // bool m_sw[10][20][20];
    // float m_vup[10][20][20];
    // float m_vdown[10][20][20];
    // float m_vdemand[10][20][20];
    // float m_vlimit[10][10][20];
    // float m_climit[10][10][20];

  };

};

#endif
