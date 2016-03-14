#ifndef _Belle2_COPPERHandler_h
#define _Belle2_COPPERHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define CPRHANDLER_INT_GET(CLASS)     \
  class CLASS : public NSMVHandlerInt, HandlerCPR {     \
  public:                  \
    CLASS(COPPERCallback& callback, const std::string& name,     \
          int hslb = -1, int adr = -1, int size = -1)        \
      : NSMVHandlerInt(name, true, false),           \
        HandlerCPR(callback, hslb, adr, size) {}       \
    virtual ~CLASS() throw() {}              \
    virtual bool handleGetInt(int& val);             \
  }

#define CPRHANDLER_INT_SET(CLASS)     \
  class CLASS : public NSMVHandlerInt, HandlerCPR {     \
  public:                  \
    CLASS(COPPERCallback& callback, const std::string& name,     \
          int hslb = -1, int adr = -1, int size = -1)        \
      : NSMVHandlerInt(name, false, true),           \
        HandlerCPR(callback, hslb, adr, size) {}       \
    virtual ~CLASS() throw() {}              \
    virtual bool handleSetInt(int val);            \
  }

#define CPRHANDLER_INT(CLASS)       \
  class CLASS : public NSMVHandlerInt, HandlerCPR {     \
  public:                  \
    CLASS(COPPERCallback& callback, const std::string& name,     \
          int hslb = -1, int adr = -1, int size = -1)        \
      : NSMVHandlerInt(name, true, true),            \
        HandlerCPR(callback, hslb, adr, size) {}       \
    virtual ~CLASS() throw() {}              \
    virtual bool handleGetInt(int& val);             \
    virtual bool handleSetInt(int val);            \
  }

#define CPRHANDLER_TEXT(CLASS)       \
  class CLASS : public NSMVHandlerText, HandlerCPR {     \
  public:                  \
    CLASS(COPPERCallback& callback, const std::string& name,     \
          int hslb, const std::string& str)        \
      : NSMVHandlerText(name, true, true, str),       \
        HandlerCPR(callback, hslb, -1, -1) {}       \
    virtual ~CLASS() throw() {}              \
    virtual bool handleSetText(const std::string& val); \
  }

namespace Belle2 {

  class COPPERCallback;

  class HandlerCPR {
  public:
    HandlerCPR(COPPERCallback& callback, int hslb, int adr, int size)
      : m_callback(callback), m_hslb(hslb), m_adr(adr), m_size(size) {}
    virtual ~HandlerCPR() throw() {}
  protected:
    COPPERCallback& m_callback;
    int m_hslb;
    int m_adr;
    int m_size;
  };

  CPRHANDLER_INT_GET(NSMVHandlerFifoEmpty);
  CPRHANDLER_INT_GET(NSMVHandlerFifoFull);
  CPRHANDLER_INT_GET(NSMVHandlerLengthFifoFull);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBBelle2LinkDown);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBCOPPERFifoFull);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBCOPPERLengthFifoFull);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBFifoFull);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBCRCError);
  CPRHANDLER_INT_GET(NSMVHandlerTTRXBelle2LinkError);
  CPRHANDLER_INT_GET(NSMVHandlerTTRXLinkUpError);
  CPRHANDLER_INT(NSMVHandlerHSLBUsed);
  CPRHANDLER_INT(NSMVHandlerHSLBRegValue);
  CPRHANDLER_INT(NSMVHandlerHSLBRegFixed);
  CPRHANDLER_INT_SET(NSMVHandlerHSLBLinkFee);
  CPRHANDLER_INT_SET(NSMVHandlerHSLBUnLinkFee);
  CPRHANDLER_INT_SET(NSMVHandlerHSLBTrgOnFee);
  CPRHANDLER_INT_SET(NSMVHandlerHSLBTrgOffFee);
  CPRHANDLER_INT_GET(NSMVHandlerHSLBCheckFee);
  CPRHANDLER_TEXT(NSMVHandlerTTRXFirmware);
  CPRHANDLER_TEXT(NSMVHandlerHSLBFirmware);
  CPRHANDLER_TEXT(NSMVHandlerFEEStream);
  CPRHANDLER_INT_SET(NSMVHandlerFEEBoot);

}

#endif
