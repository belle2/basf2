#ifndef __BELLE2_TOPFEEHANDLER_H__
#define __BELLE2_TOPFEEHANDLER_H__

#include <daq/slc/nsm/NSMVHandler.h>

#define TOP_HANDLER_INT(CLASS) \
  class CLASS : public NSMVHandlerInt, TOPFEEHandler { \
  public:\
    CLASS(const std::string& name, RCCallback& callback, HSLB& hslb, FEE& fee, int value = 0)\
      : NSMVHandlerInt(name, true, true, value), TOPFEEHandler(callback, hslb, fee) {} \
    virtual ~CLASS() throw() {} \
    virtual bool handleGetInt(int& val);   \
    virtual bool handleSetInt(int val);    \
  }

namespace Belle2 {

  class RCCallback;
  class FEE;
  class HSLB;

  class TOPFEEHandler {
  public:
    TOPFEEHandler(RCCallback& callback, HSLB& hslb, FEE& fee) :
      m_callback(callback), m_hslb(hslb), m_fee(fee) {}
    virtual ~TOPFEEHandler() throw() {}

  protected:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;
  };

  TOP_HANDLER_INT(TOPHandlerLookback);
  TOP_HANDLER_INT(TOPHandlerFEMode);
}

#endif
