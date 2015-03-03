#ifndef _Belle2_ArichHVControlCallback_h
#define _Belle2_ArichHVControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>
#include <daq/slc/hvcontrol/arich/ArichHVCommunicator.h>

#include <vector>

namespace Belle2 {

  class ArichHVControlCallback : public HVControlCallback {

  public:
    ArichHVControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~ArichHVControlCallback() throw() {}

  public:
    virtual void initialize(const HVConfig& hvconf) throw();
    virtual void timeout() throw();

  public:
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException);
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException);
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException);
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException);
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException);

    virtual bool getSwitch(int crate, int slot, int channel) throw(IOException);
    virtual float getRampUpSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getRampDownSpeed(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageDemand(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageLimit(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentLimit(int crate, int slot, int channel) throw(IOException);
    virtual int getState(int crate, int slot, int channel) throw(IOException);
    virtual float getVoltageMonitor(int crate, int slot, int channel) throw(IOException);
    virtual float getCurrentMonitor(int crate, int slot, int channel) throw(IOException);

    virtual void store(int index) throw(IOException);
    virtual void recall(int index) throw(IOException);

  public:
    int getNAComms() const { return m_acomm.size(); }
    ArichHVCommunicator& getAComm(int ic) { return m_acomm[ic]; }

  private:
    class Listener {
    private:
      ArichHVCommunicator& m_acomm;
    public:
      Listener(ArichHVCommunicator& comm) : m_acomm(comm) {}
      void run();
    };

  private:
    std::vector<ArichHVCommunicator> m_acomm;

  };

};

#endif
