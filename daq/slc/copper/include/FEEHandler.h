#ifndef _Belle2_ECLFEEHandler_h
#define _Belle2_ECLFEEHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define FEE_HANDLER_INT(_FEEHDR_)           \
  class _FEEHDR_ : public FEEHandler {            \
  public:                 \
    _FEEHDR_(const std::string& name, RCCallback& callback,     \
             HSLB& hslb, FEE& fee, int adr)         \
      : FEEHandler(name, callback, hslb, fee, adr) {}       \
    virtual ~_FEEHDR_() throw() {}            \
    virtual bool feeget(int& val);        \
    virtual bool feeset(int val);         \
  }               \

  namespace Belle2 {

    class RCCallback;
    class FEE;
    class HSLB;

    class FEEHandler : public NSMVHandlerInt {
    public:
      FEEHandler(const std::string& name, RCCallback& callback,
                 HSLB& hslb, FEE& fee, int adr)
        : NSMVHandlerInt(name, true, true),
          m_callback(callback), m_hslb(hslb), m_fee(fee), m_adr(adr) {}
      virtual ~FEEHandler() throw() {}
    public:
      virtual bool handleGetInt(int& val) { return feeget(val); }
      virtual bool handleSetInt(int val) { return feeset(val); }
    protected:
      virtual bool feeget(int& val) = 0;
      virtual bool feeset(int val) = 0;
    protected:
      RCCallback& m_callback;
      HSLB& m_hslb;
      FEE& m_fee;
      int m_adr;
    };

    FEE_HANDLER_INT(FEE8Handler);
    FEE_HANDLER_INT(FEE32Handler);

  };

#endif
