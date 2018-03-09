#ifndef _Belle2_ArichHVHandler_h
#define _Belle2_ArichHVHandler_h

#include <daq/slc/hvcontrol/HVControlCallback.h>
#include <daq/slc/hvcontrol/HVVHandler.h>

#include <vector>

#define ARICHHVHANDLER_TEXT(CLASS)          \
  class CLASS : public NSMVHandlerText, ArichHandlerHV {    \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int crate, int slot, int channel)       \
      : NSMVHandlerText(name, true, true),        \
        ArichHandlerHV(arichcallback, crate, slot, channel) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetText(std::string& val);     \
    virtual bool handleSetText(const std::string& val);     \
  }

#define ARICHHVHANDLER_FLOAT(CLASS)         \
  class CLASS : public NSMVHandlerFloat, ArichHandlerHV {   \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int crate, int slot, int channel)       \
      : NSMVHandlerFloat(name, true, true),       \
        ArichHandlerHV(arichcallback, crate, slot, channel) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetFloat(float& val);        \
    virtual bool handleSetFloat(float val);       \
  }

#define ARICHHVHANDLER_INT(CLASS)         \
  class CLASS : public NSMVHandlerInt, ArichHandlerHV {     \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int crate, int slot, int channel)       \
      : NSMVHandlerInt(name, true, true),         \
        ArichHandlerHV(arichcallback, crate, slot, channel) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetInt(int& val);        \
    virtual bool handleSetInt(int val);         \
  }


#define ARICHHVHANDLERHAPD_TEXT(CLASS)          \
  class CLASS : public NSMVHandlerText, ArichHandlerHVHAPD {    \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int modid, int ch_typeid)           \
      : NSMVHandlerText(name, true, true),        \
        ArichHandlerHVHAPD(arichcallback, modid, ch_typeid) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetText(std::string& val);     \
    virtual bool handleSetText(const std::string& val);     \
  }


#define ARICHHVHANDLERHAPD_FLOAT(CLASS)         \
  class CLASS : public NSMVHandlerFloat, ArichHandlerHVHAPD {   \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int modid, int ch_typeid)           \
      : NSMVHandlerFloat(name, true, true),       \
        ArichHandlerHVHAPD(arichcallback, modid, ch_typeid) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetFloat(float& val);        \
    virtual bool handleSetFloat(float val);       \
  }

#define ARICHHVHANDLERHAPD_INT(CLASS)         \
  class CLASS : public NSMVHandlerInt, ArichHandlerHVHAPD {   \
  public:               \
    CLASS(ArichHVControlCallback& arichcallback, const std::string& name, \
          int modid, int ch_typeid)           \
      : NSMVHandlerInt(name, true, true),         \
        ArichHandlerHVHAPD(arichcallback, modid, ch_typeid) {}    \
    virtual ~CLASS() throw() {}           \
    virtual bool handleGetInt(int& val);        \
    virtual bool handleSetInt(int val);         \
  }



namespace Belle2 {

  class ArichHVControlCallback;

  class ArichHandlerHV {
  public:
    ArichHandlerHV(ArichHVControlCallback& callback, int crate, int slot, int channel)
      : m_callback(callback), m_crate(crate), m_slot(slot), m_channel(channel) {}
  protected:
    ArichHVControlCallback& m_callback;
    int m_crate;
    int m_slot;
    int m_channel;
  };

  class ArichHandlerHVHAPD {
  public:
    ArichHandlerHVHAPD(ArichHVControlCallback& callback, int modid, int ch_typeid)
      : m_callback(callback), m_modid(modid), m_ch_typeid(ch_typeid) {}
  protected:
    ArichHVControlCallback& m_callback;
    int m_modid;
    int m_ch_typeid;
  };



  Mutex g_mutex_arich;

  class NSMVArichHVClearAlarm : public NSMVHandlerInt {
  public:
    NSMVArichHVClearAlarm(ArichHVControlCallback& callback, const std::string& name, int crate)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback), m_crate(crate) {}
    virtual ~NSMVArichHVClearAlarm() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.ClearAlarm(m_crate);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
  };

  class NSMVArichHVAllClearAlarm : public NSMVHandlerInt {
  public:
    NSMVArichHVAllClearAlarm(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllClearAlarm() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.AllClearAlarm();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVGBMdTest : public NSMVHandlerFloat {
  public:
    NSMVArichHVGBMdTest(ArichHVControlCallback& callback, const std::string& name, int crate, int slot)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVGBMdTest() throw() {}
    virtual bool handleSetFloat(float vset)
    {
      if (vset >= 0) {
        try {
          m_callback.GBmdtest(m_crate, m_slot, vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
    int m_slot;
  };

  class NSMVArichHVHVMdTest : public NSMVHandlerFloat {
  public:
    NSMVArichHVHVMdTest(ArichHVControlCallback& callback, const std::string& name, int crate, int slot)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVHVMdTest() throw() {}
    virtual bool handleSetFloat(float vset)
    {
      if (vset >= 0) {
        try {
          m_callback.HVmdtest(m_crate, m_slot, vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
    int m_slot;
  };

  class NSMVArichHVsetAllHV : public NSMVHandlerFloat {
  public:
    NSMVArichHVsetAllHV(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVsetAllHV() throw() {}
    virtual bool handleSetFloat(float vset)
    {
      if (vset >= 0) {
        try {
          m_callback.setAllHV(vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVsetAllBias : public NSMVHandlerFloat {
  public:
    NSMVArichHVsetAllBias(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVsetAllBias() throw() {}
    virtual bool handleSetFloat(float vset)
    {
      if (vset >= 0) {
        try {
          m_callback.setAllBias(vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVsetAllGuard : public NSMVHandlerFloat {
  public:
    NSMVArichHVsetAllGuard(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVsetAllGuard() throw() {}
    virtual bool handleSetFloat(float vset)
    {
      if (vset >= 0) {
        try {
          m_callback.setAllGuard(vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVsetAllBiasNominal : public NSMVHandlerFloat {
  public:
    NSMVArichHVsetAllBiasNominal(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerFloat(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVsetAllBiasNominal() throw() {}
    virtual bool handleSetFloat(float less_vset)
    {
      if (less_vset >= 0) {
        try {
          m_callback.setAllBias_nominal(less_vset);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerFloat::handleSetFloat(less_vset);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVMdAllSwitchOn : public NSMVHandlerInt {
  public:
    NSMVArichHVMdAllSwitchOn(ArichHVControlCallback& callback, const std::string& name, int crate, int slot)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVMdAllSwitchOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.md_all_switch(m_crate, m_slot, true);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
    int m_slot;
  };

  class NSMVArichHVMdAllSwitchOff : public NSMVHandlerInt {
  public:
    NSMVArichHVMdAllSwitchOff(ArichHVControlCallback& callback, const std::string& name, int crate, int slot)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVMdAllSwitchOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.md_all_switch(m_crate, m_slot, false);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
    int m_crate;
    int m_slot;
  };

  class NSMVArichHVAllGuardOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllGuardOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("guard", true);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllGuardOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllGuardOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("guard", false);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllHVOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllHVOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("hv", true);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllHVOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllHVOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("hv", false);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllBiasOn : public NSMVHandlerInt {
  public:
    NSMVArichHVAllBiasOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOn() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("bias", true);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVAllBiasOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllBiasOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOff() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.all_switch("bias", false);
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllGuardOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllGuardOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOff() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllGuardOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllGuardOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOn() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllBiasOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOff() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };
  class NSMVArichHVCheckAllBiasOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOn() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVCheckAllHVOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOff() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };
  class NSMVArichHVCheckAllHVOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOn(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, false),
        m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOn() throw() {}
    virtual bool handleGetInt(int& val);
  private:
    ArichHVControlCallback& m_callback;
  };


  /*
  class NSMVArichHVAllOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllOff(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, false, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVAllOff() throw() {}
    virtual bool handleSetInt(int& val) {
      try {
  m_callback.all_off();
      } catch (const IOException& e) {
  m_callback.log(LogFile::ERROR, e.what());
      }
      NSMVHandlerInt::handleGetInt(val);
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };
  */

  class NSMVArichHVTempSetup : public NSMVHandlerInt {
  public:
    NSMVArichHVTempSetup(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVTempSetup() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.temp_setup();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };
  class NSMVArichHVTest : public NSMVHandlerInt {
  public:
    NSMVArichHVTest(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVTest() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.test();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  class NSMVArichHVPanelTest : public NSMVHandlerInt {
  public:
    NSMVArichHVPanelTest(ArichHVControlCallback& callback, const std::string& name)
      : NSMVHandlerInt(name, true, true),
        m_callback(callback) {}
    virtual ~NSMVArichHVPanelTest() throw() {}
    virtual bool handleSetInt(int val)
    {
      if (val > 0) {
        try {
          m_callback.panel_test();
        } catch (const IOException& e) {
          m_callback.log(LogFile::ERROR, e.what());
        }
        NSMVHandlerInt::handleSetInt(val);
      }
      return true;
    }
  private:
    ArichHVControlCallback& m_callback;
  };

  ARICHHVHANDLER_FLOAT(NSMVHandlerHVTripTime);
  ARICHHVHANDLER_INT(NSMVHandlerHVPOn);
  ARICHHVHANDLER_INT(NSMVHandlerHVPDown);

  ARICHHVHANDLERHAPD_INT(NSMVHandlerHVHAPDSwitch);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDRampUpSpeed);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDRampDownSpeed);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDVoltageDemand);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDVoltageLimit);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDCurrentLimit);

  ARICHHVHANDLERHAPD_INT(NSMVHandlerHVHAPDState);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDVoltageMonitor);
  ARICHHVHANDLERHAPD_FLOAT(NSMVHandlerHVHAPDCurrentMonitor);

  //  ARICHHVHANDLER_INT(NSMVHandlerHVMaskedChannel);
  //  ARICHHVHANDLER_INT(NSMVHandlerHVMaskedHAPD);
  ARICHHVHANDLER_TEXT(NSMVHandlerHVChannelMask);
  ARICHHVHANDLER_TEXT(NSMVHandlerHVHAPDMask);

  /*
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllGuardOn);
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllBiasOn);
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllHVOn);
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllGuardOff);
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllBiasOff);
  ARICHHVHANDLER_INT(NSMVHandlerHVCheckAllHVOff);
  */

};

#endif
