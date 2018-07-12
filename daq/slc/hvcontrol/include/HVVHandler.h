#ifndef _Belle2_HVVHandler_h
#define _Belle2_HVVHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define HVHANDLER_TEXT(CLASS)           \
  class CLASS : public NSMVHandlerText, HandlerHV {     \
  public:                 \
    CLASS(HVCallback& callback, const std::string& name,    \
          int crate, int slot, int channel)       \
      : NSMVHandlerText(name, true, true),        \
        HandlerHV(callback, crate, slot, channel) {}      \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetText(std::string& val);     \
    virtual bool handleSetText(const std::string& val);     \
  }

#define HVHANDLER_FLOAT(CLASS)            \
  class CLASS : public NSMVHandlerFloat, HandlerHV {      \
  public:                 \
    CLASS(HVCallback& callback, const std::string& name,    \
          int crate, int slot, int channel)       \
      : NSMVHandlerFloat(name, true, true),       \
        HandlerHV(callback, crate, slot, channel) {}      \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetFloat(float& val);        \
    virtual bool handleSetFloat(float val);       \
  }

namespace Belle2 {

  class HVCallback;

  class HandlerHV {
  public:
    HandlerHV(HVCallback& callback, int crate, int slot, int channel)
      : m_callback(callback), m_crate(crate), m_slot(slot), m_channel(channel) {}
  protected:
    HVCallback& m_callback;
    int m_crate;
    int m_slot;
    int m_channel;
    float m_val_f;
    std::string m_val_s;
  };

  HVHANDLER_TEXT(NSMVHandlerHVSwitch);
  HVHANDLER_FLOAT(NSMVHandlerHVRampUpSpeed);
  HVHANDLER_FLOAT(NSMVHandlerHVRampDownSpeed);
  HVHANDLER_FLOAT(NSMVHandlerHVVoltageDemand);
  HVHANDLER_FLOAT(NSMVHandlerHVVoltageLimit);
  HVHANDLER_FLOAT(NSMVHandlerHVCurrentLimit);
  HVHANDLER_TEXT(NSMVHandlerHVState);
  HVHANDLER_FLOAT(NSMVHandlerHVVoltageMonitor);
  HVHANDLER_FLOAT(NSMVHandlerHVCurrentMonitor);

}

#endif
