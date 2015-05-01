#ifndef _Belle2_ECLFEEHandler_h
#define _Belle2_ECLFEEHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define ECL_HANDLER_INT(CLASS)          \
  class CLASS : public NSMVHandlerInt, ECLFEEHandler {    \
  public:             \
    CLASS(const std::string& name, RCCallback& callback,    \
          HSLB& hslb, FEE& fee, int adr)     \
      : NSMVHandlerInt(name, true, true),       \
        ECLFEEHandler(callback, hslb, fee, adr) {}  \
    virtual ~CLASS() throw() {}         \
    virtual bool handleGetInt(int& val) { return feeget(val); } \
    virtual bool handleSetInt(int val) { return feeset(val); }  \
  }

namespace Belle2 {

  class RCCallback;
  class FEE;
  class HSLB;

  class ECLFEEHandler {
  public:
    ECLFEEHandler(RCCallback& callback,
                  HSLB& hslb, FEE& fee, int adr)
      : m_callback(callback), m_hslb(hslb), m_fee(fee), m_adr(adr) {}
    virtual ~ECLFEEHandler() throw() {}
  protected:
    bool feeget(int& val);
    bool feeset(int val);
  protected:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;
    int m_adr;
  };

  ECL_HANDLER_INT(ECLShaperMaskLowHandler);
  ECL_HANDLER_INT(ECLShaperMaskHighHandler);
  ECL_HANDLER_INT(ECLTTTrgRareFactorHandler);
  ECL_HANDLER_INT(ECLTTTrgTypeHandler);
  ECL_HANDLER_INT(ECLCalibAmpl0LowHandler);
  ECL_HANDLER_INT(ECLCalibAmpl0HighHandler);
  ECL_HANDLER_INT(ECLCalibAmplStepLowHandler);
  ECL_HANDLER_INT(ECLCalibAmplStepHighHandler);
  ECL_HANDLER_INT(ECLCalibDelay0LowHandler);
  ECL_HANDLER_INT(ECLCalibDelay0HighHandler);
  ECL_HANDLER_INT(ECLCalibDelaytepLowHandler);
  ECL_HANDLER_INT(ECLCalibDelayStepHighHandler);
  ECL_HANDLER_INT(ECLCalibEventPerStepHandler);

}

#endif
