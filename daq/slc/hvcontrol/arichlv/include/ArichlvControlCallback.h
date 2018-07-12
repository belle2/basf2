#ifndef _Belle2_ArichlvControlCallback_h
#define _Belle2_ArichlvControlCallback_h

#include <daq/slc/hvcontrol/HVControlCallback.h>
//#include <daq/slc/hvcontrol/arichlv/ArichlvConfig.h>

#include <vector>
#include <math.h>
namespace Belle2 {
  class ArichlvMpodch {
  public:
    ArichlvMpodch() {};
    ArichlvMpodch(unsigned int t , int st, int on, float v0, float i0, float v, float i) {Set(t , st, on, v0, i0,  v, i);};
    int HasChanged(ArichlvMpodch& p, ArichlvMpodch& l)
    {
      if (
        abs(status - p.status) > l.status ||
        abs(onoff - p.onoff) > l.onoff    ||
        fabs(vset - p.vset) > l.vset      ||
        fabs(iset - p.iset) > l.iset      ||
        fabs(vmon - p.vmon) > l.vmon      ||
        fabs(imon - p.imon) > l.imon
      ) return 1;
      return 0;

    }
    ~ArichlvMpodch() {};
    int Set(ArichlvMpodch& p)
    {
      timestamp = p.timestamp;
      status = p.status;
      onoff = p.onoff;
      vset  = p.vset;
      iset  = p.iset;
      vmon  = p.vmon;
      imon  = p.imon;
      return 1;
    }
    void Set(unsigned int t , int st, int on, float v0, float i0, float v, float i)
    {
      timestamp = t;
      status = st;
      onoff = on ;
      vset  = v0;
      iset  = i0;
      vmon  = v;
      imon  = i;
    };
    void Get(unsigned& t, int& st, int& on, float& v0, float& i0, float& v, float& i)
    {
      t  = timestamp;
      st = status;
      on = onoff;
      v0 = vset;
      i0 = iset;
      v  = vmon;
      i  = imon;
    };
  private:
    unsigned int timestamp;
    int status;
    int onoff;
    float vset;
    float iset;
    float vmon;
    float imon;

  };

  class ArichlvStatus {
  private:
    int IndexOf(int c, int s, int ch) { return (c - 1) * 64 + s * 8 + ch;}
    ArichlvMpodch channels[128];
    ArichlvMpodch deltas[128];

  public :
    ArichlvStatus()
    {
      for (int i = 0; i < 128; i++) {
        // set deltas
        const float dx = 0.01;
        deltas[i].Set(time(NULL), 0, 0, dx, dx, dx, dx);
      }
    };
    ~ArichlvStatus() {};
    int SetDeltaLimits(int crate, int slot, int ch, int status, int onoff, float vset, float iset, float vmon, float imon);
    int Set(int crate, int slot, int ch, int status, int onoff, float vset, float iset, float vmon, float imon)
    {
      unsigned int i = IndexOf(crate, slot, ch);
      if (i >= 128) return 0;
      ArichlvMpodch* p = &channels[i];
      ArichlvMpodch* l = &deltas[i];
      ArichlvMpodch n = ArichlvMpodch(time(NULL), status, onoff, vset, iset, vmon, imon);
      if (p->HasChanged(n, *l)) return p->Set(n);
      return 0;
    };
    unsigned int Get(int crate, int slot, int ch, int& status, int& onoff, float& vset, float& iset, float& vmon, float& imon);
  };

  class ArichlvControlCallback : public HVControlCallback {

  public:
    ArichlvControlCallback() throw()
      : HVControlCallback(NSMNode()) {}
    virtual ~ArichlvControlCallback() throw() {}

  public:
//    virtual void init(NSMCommunicator&) throw();
    //virtual void initialize(const HVConfig&) throw();
    virtual void initialize(const HVConfig& config) throw();
    virtual void update() throw(HVHandlerException);

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

  private:
    int MPODCH(int crate, int slot);
    int checkRange(int crate, int slot, int channel, int linenum);
    ArichlvStatus* m_laststate;
    Mutex m_mutex;

  };

};

#endif
