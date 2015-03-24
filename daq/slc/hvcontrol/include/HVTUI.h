#ifndef _Belle2_HVTUI_h
#define _Belle2_HVTUI_h

#include <daq/slc/base/IOException.h>

#include <daq/slc/hvcontrol/HVMessage.h>

#include <vector>
#include <string>

namespace Belle2 {

  class HVTUI {

    friend class Monitor;

  public:
    HVTUI() {}
    virtual ~HVTUI() throw();

  public:
    void run();
    void addChannel(int crate, int slot, int ch)
    {
      Raw raw;
      raw.crate = crate;
      raw.slot = slot;
      raw.channel = ch;
      m_table.push_back(raw);
    }
    int getNChannels() { return m_table.size(); }
    int getIndex(int crate, int slot, int channel);
    void getChannel(int index, int& crate, int& slot, int& channel);

  protected:
    void init(int m_mode, int m_interval);
    virtual void setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException) = 0;
    virtual void setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException) = 0;
    virtual void setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException) = 0;
    virtual void setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException) = 0;
    virtual void setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException) = 0;
    virtual void setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException) = 0;
    virtual void store(int index) throw(IOException) = 0;
    virtual void recall(int index) throw(IOException) = 0;
    virtual void requestSwitch(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestRampUpSpeed(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestRampDownSpeed(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestVoltageDemand(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestVoltageLimit(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestCurrentLimit(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestVoltageMonitor(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestCurrentMonitor(int crate, int slot, int channel) throw(IOException) = 0;
    virtual void requestState(int crate, int slot, int channel) throw(IOException) = 0;
    virtual HVMessage wait() throw(IOException) = 0;

  protected:
    int getx(int ic);
    int gety(int ir);
    void setValue(int ic, int ir, const std::string& value);
    void setNumber(int ic, int ir, float v,
                   const char* format = "%.0f");
    void setSwitchOn(int i, int state);
    int add(int xs, const std::string& label);
    void getValueAll();

  private:
    struct Cell {
      std::string label;
      int xs;
      int width;
      std::string value;
    };

    struct Raw {
      std::vector<Cell> c;
      int ys;
      int crate;
      int slot;
      int channel;
    };

    class Listener {
    private:
      HVTUI* m_tui;
    public:
      Listener(HVTUI* tui) : m_tui(tui) {}
    public:
      void run();
    };

    class Monitor {
    private:
      const int m_interval;
      HVTUI* m_tui;
    public:
      Monitor(int interval, HVTUI* tui)
        : m_interval(interval), m_tui(tui) {}
    public:
      void run();
    };

  public:
    int getMode() { return m_mode; }

  private:
    int m_mode;
    int m_interval;
    std::vector<struct Raw> m_table;
    bool m_allon;
    int m_nstore;
    int m_nrecall;
    struct Raw m_rhead;
    int m_ir;
    int m_ic;
    int m_it;
    bool m_edit;
    void* m_pad;
    int m_pad_y;

  };

}

#endif
