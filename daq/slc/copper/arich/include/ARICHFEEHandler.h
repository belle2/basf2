#ifndef _Belle2_ARICHFEEHandler_h
#define _Belle2_ARICHFEEHandler_h

#include <daq/slc/nsm/NSMVHandler.h>

#define ARICH_HANDLER_INT(CLASS)          \
  class CLASS : public NSMVHandlerInt, ARICHFEEHandler {    \
  public:             \
    CLASS(const std::string& name, RCCallback& callback,    \
          HSLB& hslb, FEE& fee, int index = -1)     \
      : NSMVHandlerInt(name, true, true),       \
        ARICHFEEHandler(callback, hslb, fee), m_index(index) {} \
    virtual ~CLASS() throw() {}         \
    virtual bool handleGetInt(int& val);        \
    virtual bool handleSetInt(int val);        \
  private:              \
    int m_index;              \
  }

#define ARICH_HANDLER_FLOAT(CLASS)          \
  class CLASS : public NSMVHandlerFloat, ARICHFEEHandler {    \
  public:             \
    CLASS(const std::string& name, RCCallback& callback,    \
          HSLB& hslb, FEE& fee, int index = -1)     \
      : NSMVHandlerFloat(name, true, true),       \
        ARICHFEEHandler(callback, hslb, fee), m_index(index) {} \
    virtual ~CLASS() throw() {}         \
    virtual bool handleGetFloat(float& val);        \
    virtual bool handleSetFloat(float val);        \
  private:              \
    int m_index;              \
  }

#define ARICH_HANDLER_ASIC_INT(CLASS)          \
  class CLASS : public NSMVHandlerInt, ARICHFEEHandler {    \
  public:             \
    CLASS(const std::string& name, RCCallback& callback,    \
          HSLB& hslb, FEE& fee, int index = -1, int chip = -1, int ch = -1)  \
      : NSMVHandlerInt(name, true, true),       \
        ARICHFEEHandler(callback, hslb, fee), m_index(index), m_chip(chip), m_ch(ch){} \
    virtual ~CLASS() throw() {}         \
    virtual bool handleGetAsicInt(int& val);        \
    virtual bool handleSetAsicInt(int val);        \
  private:              \
    int m_index;              \
    int m_chip;              \
    int m_ch;              \
  }

#define ARICH_HANDLER_TEXT(CLASS)           \
  class CLASS : public NSMVHandlerText, ARICHFEEHandler {     \
  public:               \
    CLASS(const std::string& name, RCCallback& callback,      \
          HSLB& hslb, FEE& fee)           \
      : NSMVHandlerText(name, true, true),        \
        ARICHFEEHandler(callback, hslb, fee) {}       \
    virtual ~CLASS() throw() {}           \
    virtual bool handleSetText(const std::string& val);       \
  };

namespace Belle2 {

  class RCCallback;
  class FEE;
  class HSLB;

  class ARICHFEEHandler {
  public:
    ARICHFEEHandler(RCCallback& callback, HSLB& hslb, FEE& fee)
      : m_callback(callback), m_hslb(hslb), m_fee(fee) {}
    virtual ~ARICHFEEHandler() throw() {}
  protected:
    RCCallback& m_callback;
    HSLB& m_hslb;
    FEE& m_fee;
  };

  ARICH_HANDLER_INT(ARICHHandlerGo);
  ARICH_HANDLER_TEXT(ARICHHandlerLoadParam);
  ARICH_HANDLER_FLOAT(ARICHHandlerThreshold);
  ARICH_HANDLER_INT(ARICHHandlerThIndex);
  ARICH_HANDLER_INT(ARICHHandlerReset);
  ARICH_HANDLER_INT(ARICHHandlerPTM1);
  ARICH_HANDLER_INT(ARICHHandlerHDCycle);
  ARICH_HANDLER_INT(ARICHHandlerTrgDelay);
  //ASIC global param.
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicPhasecmps);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicGain);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicShapingTime);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicComparator);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicVRDrive);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicMonitor);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicId);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicMasked);
  //ASIC channel param.
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicDecayTime);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicOffset);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicFineadj_Unipol);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicFineadj_Diff);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicTpenb);
  ARICH_HANDLER_ASIC_INT(ARICHHandlerAsicKill);

}

#endif
