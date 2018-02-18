#ifndef _Belle2_ArichHVControlCallback_h
#define _Belle2_ArichHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>
#include <daq/slc/hvcontrol/HVVHandler.h>

#include <vector>


namespace Belle2 {

  class ArichHVControlCallback : public HVControlCallback {

  public:
    ArichHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~ArichHVControlCallback() throw() {}

  public:
    virtual void addAll(const HVConfig& config) throw() ;
    virtual void initialize(const HVConfig& hvconf) throw();
    virtual void deinitialize(int handle) throw();//yone
    virtual void update() throw();
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);//yone
    virtual void standby() throw(HVHandlerException);//yone
    virtual void shoulder() throw(HVHandlerException);//yone
    virtual void peak() throw(HVHandlerException);//yone
    //    virtual void all_sw(bool sw, char* set_type) throw(HVHandlerException);//yone
    virtual void all_switch(std::string set_type, bool sw) throw(HVHandlerException);//yone
    virtual void md_all_switch(int crate, int slot, bool sw) throw(HVHandlerException);//yone
    virtual void GBmdtest(int crate, int slot, float vset) throw(HVHandlerException);//yone
    virtual void HVmdtest(int crate, int slot, float vset) throw(HVHandlerException);//yone
    virtual void setAllHV(float vset) throw(HVHandlerException);//yone
    virtual void setAllGuard(float vset) throw(HVHandlerException);//yone
    virtual void setAllBias(float vset) throw(HVHandlerException);//yone
    virtual void setAllBias_nominal(float less_vset) throw(HVHandlerException);//yone
    virtual void all_off() throw(HVHandlerException);//yone
    virtual void test() throw(HVHandlerException);//yone
    virtual void temp_setup() throw(HVHandlerException);//yone
    virtual void panel_test() throw(HVHandlerException);//yone
    virtual void Trip_down(int trip_crate, int trip_slot, int trip_channel) throw(HVHandlerException);//yone
    virtual bool check_all_switch(std::string set_type, bool sw) throw(HVHandlerException);//yone
    virtual void configure(const HVConfig&) throw(HVHandlerException);

    virtual void HAPD_on(int modid) throw(HVHandlerException);//yone
    virtual void HAPD_off(int modid) throw(HVHandlerException);//yone
    virtual bool check_HAPD_on(int modid) throw(HVHandlerException);//yone
    virtual bool check_HAPD_off(int modid) throw(HVHandlerException);//yone


  public:
    virtual void RecoveryTrip(int handle, int crate, int slot, int channel) throw(IOException);
    virtual void RecoveryInterlock(int handle, int crate, int slot, int channel) throw(IOException);
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);
    virtual void setTripTime(int crate, int slot, int channel, float triptime) throw(IOException);//yone
    virtual void setPOn(int crate, int slot, int channel, bool pon) throw(IOException);//yone
    virtual void setPDown(int crate, int slot, int channel, bool pdown) throw(IOException);//yone

    virtual bool getSwitch(int crate, int slot, int channel) throw(IOException);
    virtual float getRampUpSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getRampDownSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageDemand(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageLimit(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentLimit(int crate, int slot, int channel) throw(IOException);
    virtual int getState(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentMonitor(int crate, int slot, int channel) throw(IOException);
    virtual float getTripTime(int crate, int slot, int channel) throw(IOException);//yone
    virtual bool getPOn(int crate, int slot, int channel) throw(IOException);//yone
    virtual bool getPDown(int crate, int slot, int channel) throw(IOException);//yone
    virtual float getBdTemp(int crate, int slot) throw(IOException);//yone
    virtual float getBdHVMax(int crate, int slot) throw(IOException);//yone

    virtual void setHAPDSwitch(int modid, int type_id, bool switchon) throw(IOException);
    virtual void setHAPDRampUpSpeed(int modid, int type_id, float rampup) throw(IOException);
    virtual void setHAPDRampDownSpeed(int modid, int type_id, float rampdown) throw(IOException);
    virtual void setHAPDVoltageDemand(int modid, int type_id, float voltage) throw(IOException);
    virtual void setHAPDVoltageLimit(int modid, int type_id, float voltage) throw(IOException);
    virtual void setHAPDCurrentLimit(int modid, int type_id, float current) throw(IOException);

    virtual bool getHAPDSwitch(int modid, int type_id) throw(IOException);
    virtual float getHAPDRampUpSpeed(int modid, int type_id) throw(IOException);
    virtual float getHAPDRampDownSpeed(int modid, int type_id) throw(IOException);
    virtual float getHAPDVoltageDemand(int modid, int type_id) throw(IOException);
    virtual float getHAPDVoltageLimit(int modid, int type_id) throw(IOException);
    virtual float getHAPDCurrentLimit(int modid, int type_id) throw(IOException);
    virtual int getHAPDState(int modid, int type_id) throw(IOException);
    virtual float getHAPDVoltageMonitor(int modid, int type_id) throw(IOException);
    virtual float getHAPDCurrentMonitor(int modid, int type_id) throw(IOException);

    /*
    virtual void setMaskedChannel(int crate, int slot, int channel, bool masked) throw(IOException);
    virtual bool getMaskedChannel(int crate, int slot, int channel) throw(IOException);
    virtual void setMaskedHAPD(int modid, bool masked) throw(IOException);
    virtual bool getMaskedHAPD(int modid) throw(IOException);

    */

    virtual void setChannelMask(int crate, int slot, int channel, std::string mask) throw(IOException);
    virtual std::string getChannelMask(int crate, int slot, int channel) throw(IOException);
    virtual void setHAPDMask(int modid, std::string mask) throw(IOException);
    virtual std::string getHAPDMask(int modid) throw(IOException);


    virtual void fill_num() throw();
    virtual void store(int index) throw(IOException);
    virtual void recall(int index) throw(IOException);
    void clearAlarm(int crate) throw(IOException);

  private:
    std::vector<int> m_handle;

  };

  int hapd2CrateNumber[420][6];
  int hapd2SlotNumber[420][6];
  int hapd2ChannelNumber[420][6];

  int ps2modid[7][15][48];
  int ps2typeid[7][15][48];
  int ps2sector[7][15][48];

  //  bool masked_ch[7][16][50];
  //  bool masked_hapd[420];
  //  bool turnon_ch[7][15][48];
  //  std::vector<std::vector<std::vector<std::string> > > ch_mask;
  //  std::vector<std::string> hapd_mask;
  std::string ch_mask[7][15][48];
  std::string hapd_mask[420];
  //  std::string turnon_ch[7][16][50];

};

#endif
