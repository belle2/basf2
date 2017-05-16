#ifndef _Belle2_PyHVControlCallback_h
#define _Belle2_PyHVControlCallback_h

#include <daq/slc/pyb2daq/DAQDBObject.h>

#include <daq/slc/hvcontrol/HVControlCallback.h>

#include <boost/python.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/ref.hpp>

namespace Belle2 {

  class PyHVControlCallback {

  public:
    PyHVControlCallback() {}
    ~PyHVControlCallback() {}

  public:
    void setCallback(HVControlCallback* callback)
    {
      m_callback = callback;
    }

  public:
    virtual void run(const char* confname);
    virtual void initialize(const DAQDBObject& obj) {}
    virtual void setSwitch(int, int, int , bool) {}
    virtual void setRampUpSpeed(int, int, int , float) {}
    virtual void setRampDownSpeed(int, int, int , float) {}
    virtual void setVoltageDemand(int, int, int , float) {}
    virtual void setVoltageLimit(int, int, int , float) {}
    virtual void setCurrentLimit(int, int, int , float) {}
    virtual void setState(int, int, int , int) {}
    virtual void setVoltageMonitor(int, int, int , float) {}
    virtual void setCurrentMonitor(int, int, int , float) {}

    virtual bool getSwitch(int, int, int) { return false; }
    virtual float getRampUpSpeed(int, int, int) { return 0; }
    virtual float getRampDownSpeed(int, int, int) { return 0; }
    virtual float getVoltageDemand(int, int, int) { return 0; }
    virtual float getVoltageLimit(int, int, int) { return 0; }
    virtual float getCurrentLimit(int, int, int) { return 0; }
    virtual int getState(int, int, int) { return 0; }
    virtual float getVoltageMonitor(int, int, int) { return 0; }
    virtual float getCurrentMonitor(int, int, int) { return 0; }

  public:
    void addInt(const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerInt(name, true, true, val));
    }
    void addFloat(const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerFloat(name, true, true, val));
    }
    void addText(const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->add(new NSMVHandlerText(name, true, true, val));
    }
    void setInt(const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    void setFloat(const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    void setText(const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->set(name, val);
    }
    int getInt(const char* name) throw(NSMHandlerException)
    {
      int val = 0;
      m_callback->get(name, val);
      return val;
    }
    float getFloat(const char* name) throw(NSMHandlerException)
    {
      float val = 0;
      m_callback->get(name, val);
      return val;
    }
    const char* getText(const char* name) throw(NSMHandlerException)
    {
      static std::string val;
      m_callback->get(name, val);
      return val.c_str();
    }
    void setNodeInt(const char* node, const char* name, int val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    void setNodeFloat(const char* node, const char* name, float val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    void setNodeText(const char* node, const char* name, const char* val) throw(NSMHandlerException)
    {
      m_callback->set(NSMNode(node), name, val);
    }
    int getNodeInt(const char* node, const char* name) throw(NSMHandlerException)
    {
      int val = 0;
      m_callback->get(NSMNode(node), name, val);
      return val;
    }
    float getNodeFloat(const char* node, const char* name) throw(NSMHandlerException)
    {
      float val = 0;
      m_callback->get(NSMNode(node), name, val);
      return val;
    }
    const char* getNodeText(const char* node, const char* name) throw(NSMHandlerException)
    {
      static std::string val;
      m_callback->get(NSMNode(node), name, val);
      return val.c_str();
    }
    void log(LogFile::Priority pri, const char* msg) throw(NSMHandlerException)
    {
      m_callback->log(pri, msg);
    }

  protected:
    HVControlCallback* m_callback;

  };

  class PyHVControlCallbackImp : public HVControlCallback {

  public:
    PyHVControlCallbackImp(const NSMNode& node, PyHVControlCallback* py)
      : HVControlCallback(node), m_py(py) {}
    virtual ~PyHVControlCallbackImp() throw() {}

  public:
    virtual void initialize(const HVConfig& hvconf) throw()
    {
      m_py->initialize(DAQDBObject(hvconf.get()));
    }
    virtual void setSwitch(int crate, int slot, int channel, bool value) throw(IOException)
    {
      m_py->setSwitch(crate, slot, channel, value);
    }
    virtual void setRampUpSpeed(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setRampUpSpeed(crate, slot, channel, value);
    }
    virtual void setRampDownSpeed(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setRampDownSpeed(crate, slot, channel, value);
    }
    virtual void setVoltageDemand(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setVoltageDemand(crate, slot, channel, value);
    }
    virtual void setVoltageLimit(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setVoltageLimit(crate, slot, channel, value);
    }
    virtual void setCurrentLimit(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setCurrentLimit(crate, slot, channel, value);
    }
    virtual void setState(int crate, int slot, int channel, int value) throw(IOException)
    {
      m_py->setState(crate, slot, channel, value);
    }
    virtual void setVoltageMonitor(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setVoltageMonitor(crate, slot, channel, value);
    }
    virtual void setCurrentMonitor(int crate, int slot, int channel, float value) throw(IOException)
    {
      m_py->setCurrentMonitor(crate, slot, channel, value);
    }

    virtual bool getSwitch(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getSwitch(crate, slot, channel);
    }
    virtual float getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getRampUpSpeed(crate, slot, channel);
    }
    virtual float getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getRampDownSpeed(crate, slot, channel);
    }
    virtual float getVoltageDemand(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getVoltageDemand(crate, slot, channel);
    }
    virtual float getVoltageLimit(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getVoltageLimit(crate, slot, channel);
    }
    virtual float getCurrentLimit(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getCurrentLimit(crate, slot, channel);
    }
    virtual int getState(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getState(crate, slot, channel);
    }
    virtual float getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getVoltageMonitor(crate, slot, channel);
    }
    virtual float getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
    {
      return m_py->getCurrentMonitor(crate, slot, channel);
    }

  protected:
    PyHVControlCallback* m_py;
  };


  class PyHVControlCallbackWrapper : public PyHVControlCallback,
    public boost::python::wrapper<PyHVControlCallback> {

  public:
    virtual void initialize(const DAQDBObject& db);
    virtual void setSwitch(int crate, int slot, int channel, bool value)
    {
      _set("setSwitch", crate, slot, channel, value);
    }
    virtual void setRampUpSpeed(int crate, int slot, int channel, float value)
    {
      _set("setRampUpSpeed", crate, slot, channel, value);
    }
    virtual void setRampDownSpeed(int crate, int slot, int channel, float value)
    {
      _set("setRampDownSpeed", crate, slot, channel, value);
    }
    virtual void setVoltageDemand(int crate, int slot, int channel, float value)
    {
      _set("setVoltageDemand", crate, slot, channel, value);
    }
    virtual void setVoltageLimit(int crate, int slot, int channel, float value)
    {
      _set("setVoltageLimit", crate, slot, channel, value);
    }
    virtual void setCurrentLimit(int crate, int slot, int channel, float value)
    {
      _set("setCurrentLimit", crate, slot, channel, value);
    }
    virtual void setState(int crate, int slot, int channel, int value)
    {
      _set("setState", crate, slot, channel, value);
    }
    virtual void setVoltageMonitor(int crate, int slot, int channel, float value)
    {
      _set("setVoltageMonitor", crate, slot, channel, value);
    }
    virtual void setCurrentMonitor(int crate, int slot, int channel, float value)
    {
      _set("setCurrentMonitor", crate, slot, channel, value);
    }

    virtual bool getSwitch(int crate, int slot, int channel)
    {
      return _get_bool("getSwitch", crate, slot, channel);
    }
    virtual float getRampUpSpeed(int crate, int slot, int channel)
    {
      return _get_float("getRampUpSpeed", crate, slot, channel);
    }
    virtual float getRampDownSpeed(int crate, int slot, int channel)
    {
      return _get_float("getRampDownSpeed", crate, slot, channel);
    }
    virtual float getVoltageDemand(int crate, int slot, int channel)
    {
      return _get_float("getVoltageDemand", crate, slot, channel);
    }
    virtual float getVoltageLimit(int crate, int slot, int channel)
    {
      return _get_float("getVoltageLimit", crate, slot, channel);
    }
    virtual float getCurrentLimit(int crate, int slot, int channel)
    {
      return _get_float("getCurrentLimit", crate, slot, channel);
    }
    virtual int getState(int crate, int slot, int channel)
    {
      return _get_int("getState", crate, slot, channel);
    }
    virtual float getVoltageMonitor(int crate, int slot, int channel)
    {
      return _get_float("getVoltageMonitor", crate, slot, channel);
    }
    virtual float getCurrentMonitor(int crate, int slot, int channel)
    {
      return _get_float("getCurrentMonitor", crate, slot, channel);
    }

  protected:
    void _set(const char* name, int crate, int slot, int channel, bool value);
    void _set(const char* name, int crate, int slot, int channel, int value);
    void _set(const char* name, int crate, int slot, int channel, float value);
    bool _get_bool(const char* name, int crate, int slot, int channel);
    int _get_int(const char* name, int crate, int slot, int channel);
    float _get_float(const char* name, int crate, int slot, int channel);

  };

}

#endif
