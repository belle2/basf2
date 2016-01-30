#ifndef _Belle2_CDCFEEHandler_h
#define _Belle2_CDCFEEHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define CDC_HANDLER_INT(CLASS)          \
  class CLASS : public NSMVHandlerInt, CDCFEEHandler {    \
  public:             \
    CLASS(const std::string& name, RCCallback& callback,    \
          HSLB& hslb, FEE& fee, int index = -1)     \
      : NSMVHandlerInt(name, true, true),       \
        CDCFEEHandler(callback, hslb, fee), m_index(index) {} \
    virtual ~CLASS() throw() {}         \
    virtual bool handleGetInt(int& val);        \
  private:              \
    int m_index;              \
  }

#define CDC_HANDLER_TEXT(CLASS)           \
  class CLASS : public NSMVHandlerText, CDCFEEHandler {     \
  public:               \
    CLASS(const std::string& name, RCCallback& callback,      \
          HSLB& hslb, FEE& fee)           \
      : NSMVHandlerText(name, true, true),        \
        CDCFEEHandler(callback, hslb, fee) {}       \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetText(std::string& val);       \
  };

namespace Belle2 {

  class RCCallback;
  class FEE;
  class HSLB;

  class CDCFEEHandler {
  public:
    CDCFEEHandler(RCCallback& callback, HSLB& hslb, FEE& fee)
      : m_callback(callback), m_hslb(hslb), m_fee(fee) {}
    virtual ~CDCFEEHandler() throw() {}
  protected:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;
  };

  CDC_HANDLER_TEXT(CDCDateHandler);
  CDC_HANDLER_INT(CDCFirmwareHandler);
  CDC_HANDLER_TEXT(CDCDataFormatHandler);
  CDC_HANDLER_INT(CDCWindowHandler);
  CDC_HANDLER_INT(CDCDelayHandler);
  CDC_HANDLER_INT(CDCADCThresholdHandler);
  CDC_HANDLER_INT(CDCTDCThresholdHandler);
  CDC_HANDLER_INT(CDCIndirectADCAccessHandler);
  CDC_HANDLER_INT(CDCDACControlHandler);
  CDC_HANDLER_INT(CDCIndirectMonitorAccessHandler);
  CDC_HANDLER_INT(CDCPedestalHandler);

}

#endif
