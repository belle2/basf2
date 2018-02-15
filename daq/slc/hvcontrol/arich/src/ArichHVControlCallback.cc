//#include "daq/slc/hvcontrol/arich/ArichHVControlCallback.h"
#include <daq/slc/hvcontrol/arich/ArichHVControlCallback.h>
#include <daq/slc/hvcontrol/arich/ArichHVHandler.h>
#include <daq/slc/hvcontrol/arich/zlibstream.h>
#include <daq/slc/hvcontrol/arich/hvinfo.h>

#include <daq/slc/hvcontrol/HVConfig.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/hvcontrol/arich/CAENHVWrapper.h>

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/nsm/NSMVHandler.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <unistd.h>
#include <iostream>
#include <list>

#define Recovery
#define Recovery_Function

//#define parallel_HAPD
#define whole_HAPD

/*
namespace Belle2 {

  Mutex g_mutex_arich;

  class NSMVArichHVClearAlarm : public NSMVHandlerInt {
  public:
    NSMVArichHVClearAlarm(ArichHVControlCallback& callback,
        const std::string& name, int crate)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback), m_crate(crate) {}
    virtual ~NSMVArichHVClearAlarm() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    m_callback.clearAlarm(m_crate);
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

  class NSMVArichHVGBMdTest : public NSMVHandlerFloat {
  public:
    NSMVArichHVGBMdTest(ArichHVControlCallback& callback,
      const std::string& name, int crate, int slot)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVGBMdTest() throw() {}
    virtual bool handleSetFloat(float vset) {
      if (vset >= 0) {
  try {
    m_callback.GBmdtest(m_crate,m_slot,vset);
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
    NSMVArichHVHVMdTest(ArichHVControlCallback& callback,
      const std::string& name, int crate, int slot)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVHVMdTest() throw() {}
    virtual bool handleSetFloat(float vset) {
      if (vset >= 0) {
  try {
    m_callback.HVmdtest(m_crate,m_slot,vset);
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
    NSMVArichHVsetAllHV(ArichHVControlCallback& callback,
      const std::string& name)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback){}
    virtual ~NSMVArichHVsetAllHV() throw() {}
    virtual bool handleSetFloat(float vset) {
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
    NSMVArichHVsetAllBias(ArichHVControlCallback& callback,
      const std::string& name)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback){}
    virtual ~NSMVArichHVsetAllBias() throw() {}
    virtual bool handleSetFloat(float vset) {
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
    NSMVArichHVsetAllGuard(ArichHVControlCallback& callback,
      const std::string& name)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback){}
    virtual ~NSMVArichHVsetAllGuard() throw() {}
    virtual bool handleSetFloat(float vset) {
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
    NSMVArichHVsetAllBiasNominal(ArichHVControlCallback& callback,
      const std::string& name)
      : NSMVHandlerFloat(name, true, true),
  m_callback(callback){}
    virtual ~NSMVArichHVsetAllBiasNominal() throw() {}
    virtual bool handleSetFloat(float less_vset) {
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
    NSMVArichHVMdAllSwitchOn(ArichHVControlCallback& callback,
         const std::string& name, int crate, int slot)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVMdAllSwitchOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    m_callback.md_all_switch(m_crate,m_slot,true);
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
    NSMVArichHVMdAllSwitchOff(ArichHVControlCallback& callback,
         const std::string& name, int crate, int slot)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback), m_crate(crate), m_slot(slot) {}
    virtual ~NSMVArichHVMdAllSwitchOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    m_callback.md_all_switch(m_crate,m_slot,false);
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
    NSMVArichHVAllGuardOn(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_guard_on();
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
    NSMVArichHVAllGuardOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllGuardOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_guard_off();
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
    NSMVArichHVAllHVOn(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_hv_on();
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
    NSMVArichHVAllHVOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllHVOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_hv_off();
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
    NSMVArichHVAllBiasOn(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_bias_on();
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
    NSMVArichHVAllBiasOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllBiasOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.all_bias_off();
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

  class NSMVArichHVCheckAllGuardOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllGuardOn(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_guard_on();
    m_callback.check_all_switch("guard", true);
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
    NSMVArichHVCheckAllGuardOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllGuardOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_guard_off();
    m_callback.check_all_switch("guard", false);
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

  class NSMVArichHVCheckAllHVOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOn(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_hv_on();
    m_callback.check_all_switch("hv", true);
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

  class NSMVArichHVCheckAllHVOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllHVOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllHVOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_hv_off();
    m_callback.check_all_switch("hv", false);
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

  class NSMVArichHVCheckAllBiasOn : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOn(ArichHVControlCallback& callback,
            const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOn() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_on();
    m_callback.check_all_switch("bias", true);
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

  class NSMVArichHVCheckAllBiasOff : public NSMVHandlerInt {
  public:
    NSMVArichHVCheckAllBiasOff(ArichHVControlCallback& callback,
             const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVCheckAllBiasOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_off();
    m_callback.check_all_switch("bias", false);
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

  class NSMVArichHVAllOff : public NSMVHandlerInt {
  public:
    NSMVArichHVAllOff(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVAllOff() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_off();
    m_callback.all_off();
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

  class NSMVArichHVTempSetup : public NSMVHandlerInt {
  public:
    NSMVArichHVTempSetup(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVTempSetup() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_off();
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
    NSMVArichHVTest(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVTest() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_off();
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
    NSMVArichHVPanelTest(ArichHVControlCallback& callback,
        const std::string& name)
      : NSMVHandlerInt(name, true, true),
  m_callback(callback) {}
    virtual ~NSMVArichHVPanelTest() throw() {}
    virtual bool handleSetInt(int val) {
      if (val > 0) {
  try {
    //    m_callback.check_all_bias_off();
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

}
*/

using namespace Belle2;

void ArichHVControlCallback::clearAlarm(int crate) throw(IOException)
{

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      //    if (crate == 0 || crateid == crateid) {
      int ret = -1;
      int handle = m_handle[i];
      std::cout << "clear Alarm" << std::endl;
      if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
        LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
      } else {
        LogFile::info("Clear Alarm was done: %d", handle);
      }
    }
  }
  g_mutex_arich.unlock();
}

#define HV_HANDLE_PRE       \
  try {

#define HV_HANDLE_POST          \
  } catch (const IOException& e) {          \
    LogFile::error(e.what());         \
    m_callback.reply(NSMMessage(NSMCommand::ERROR, e.what()));  \
    return false;           \
  }               \
  return true

bool NSMVHandlerHVTripTime::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getTripTime(m_crate, m_slot, m_channel);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVTripTime::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setTripTime(m_crate, m_slot, m_channel, val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVPOn::handleGetInt(int& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getPOn(m_crate, m_slot, m_channel);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVPOn::handleSetInt(int val)
{
  HV_HANDLE_PRE;
  m_callback.setPOn(m_crate, m_slot, m_channel, val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVPDown::handleGetInt(int& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getPDown(m_crate, m_slot, m_channel);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVPDown::handleSetInt(int val)
{
  HV_HANDLE_PRE;
  m_callback.setPDown(m_crate, m_slot, m_channel, val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDSwitch::handleGetInt(int& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDSwitch(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDSwitch::handleSetInt(int val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDSwitch(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDRampUpSpeed::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDRampUpSpeed(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDRampUpSpeed::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDRampUpSpeed(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDRampDownSpeed::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDRampDownSpeed(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDRampDownSpeed::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDRampDownSpeed(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDVoltageDemand::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDVoltageDemand(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDVoltageDemand::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDVoltageDemand(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDVoltageLimit::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDVoltageLimit(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDVoltageLimit::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDVoltageLimit(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDCurrentLimit::handleGetFloat(float& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDCurrentLimit(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDCurrentLimit::handleSetFloat(float val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDCurrentLimit(ps2modid[m_crate][m_slot][m_channel], ps2typeid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

/*
bool NSMVHandlerHVMaskedChannel::handleGetInt(int& val)
{
  HV_HANDLE_PRE;
  val = (int)m_callback.getMaskedChannel(m_crate, m_slot, m_channel);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVMaskedChannel::handleSetInt(int val)
{
  HV_HANDLE_PRE;
  m_callback.setMaskedChannel(m_crate, m_slot, m_channel, (bool)val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVMaskedHAPD::handleGetInt(int& val)
{
  HV_HANDLE_PRE;
  val = (int)m_callback.getMaskedHAPD(ps2modid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVMaskedHAPD::handleSetInt(int val)
{
  HV_HANDLE_PRE;
  m_callback.setMaskedHAPD(ps2modid[m_crate][m_slot][m_channel], (bool)val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
*/

bool NSMVHandlerHVChannelMask::handleGetText(std::string& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getChannelMask(m_crate, m_slot, m_channel);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVChannelMask::handleSetText(const std::string& val)
{
  HV_HANDLE_PRE;
  m_callback.setChannelMask(m_crate, m_slot, m_channel, val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}

bool NSMVHandlerHVHAPDMask::handleGetText(std::string& val)
{
  HV_HANDLE_PRE;
  val = m_callback.getHAPDMask(ps2modid[m_crate][m_slot][m_channel]);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}
bool NSMVHandlerHVHAPDMask::handleSetText(const std::string& val)
{
  HV_HANDLE_PRE;
  m_callback.setHAPDMask(ps2modid[m_crate][m_slot][m_channel], val);
  m_callback.set(m_name, val);
  HV_HANDLE_POST;
}




void ArichHVControlCallback::addAll(const HVConfig& config) throw()
{
  //  const HVConfig& config(getConfig());
  HVCallback::addAll(config);
  const HVCrateList& crate_v(config.getCrates());

  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    int crateid = crate.getId();
    std::string vname1 = StringUtil::form("crate[%d].calarm", crateid);
    add(new NSMVArichHVClearAlarm(*this, vname1, crateid));

    // want to add new nsm value

    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& ch(*ichannel);
      int slot = ch.getSlot();
      int channel = ch.getChannel();
      std::string vname01 = StringUtil::form("crate[%d].slot[%d].mdallon", crateid, slot);
      add(new NSMVArichHVMdAllSwitchOn(*this, vname01, crateid, slot));
      std::string vname02 = StringUtil::form("crate[%d].slot[%d].mdalloff", crateid, slot);
      add(new NSMVArichHVMdAllSwitchOff(*this, vname02, crateid, slot));
      std::string vname03 = StringUtil::form("crate[%d].slot[%d].gbmdtest", crateid, slot);
      add(new NSMVArichHVGBMdTest(*this, vname03, crateid, slot));
      std::string vname04 = StringUtil::form("crate[%d].slot[%d].hvmdtest", crateid, slot);
      add(new NSMVArichHVHVMdTest(*this, vname04, crateid, slot));

      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d]", crateid, slot, channel);
      add(new NSMVHandlerHVTripTime(*this, vname + ".trip", crateid, slot, channel));
      add(new NSMVHandlerHVPOn(*this, vname + ".pon", crateid, slot, channel));
      add(new NSMVHandlerHVPDown(*this, vname + ".pdown", crateid, slot, channel));
      add(new NSMVHandlerHVChannelMask(*this, vname + ".mask", crateid, slot, channel));


    }


    std::string vname2 = StringUtil::form("allguardon");
    add(new NSMVArichHVAllGuardOn(*this, vname2));
    std::string vname3 = StringUtil::form("allguardoff");
    add(new NSMVArichHVAllGuardOff(*this, vname3));
    std::string vname4 = StringUtil::form("allhvon");
    add(new NSMVArichHVAllHVOn(*this, vname4));
    std::string vname5 = StringUtil::form("allhvoff");
    add(new NSMVArichHVAllHVOff(*this, vname5));
    std::string vname6 = StringUtil::form("allbiason");
    add(new NSMVArichHVAllBiasOn(*this, vname6));
    std::string vname7 = StringUtil::form("allbiasoff");
    add(new NSMVArichHVAllBiasOff(*this, vname7));

    std::string vname8 = StringUtil::form("checkallguardon");
    add(new NSMVArichHVCheckAllGuardOn(*this, vname8));
    std::string vname9 = StringUtil::form("checkallguardoff");
    add(new NSMVArichHVCheckAllGuardOff(*this, vname9));
    std::string vname10 = StringUtil::form("checkallhvon");
    add(new NSMVArichHVCheckAllHVOn(*this, vname10));
    std::string vname11 = StringUtil::form("checkallhvoff");
    add(new NSMVArichHVCheckAllHVOff(*this, vname11));
    std::string vname12 = StringUtil::form("checkallbiason");
    add(new NSMVArichHVCheckAllBiasOn(*this, vname12));
    std::string vname13 = StringUtil::form("checkallbiasoff");
    add(new NSMVArichHVCheckAllBiasOff(*this, vname13));

    std::string vname14 = StringUtil::form("alloff");
    add(new NSMVArichHVAllOff(*this, vname14));

    std::string vname15 = StringUtil::form("test");
    add(new NSMVArichHVTest(*this, vname15));
    std::string vname16 = StringUtil::form("paneltest");
    add(new NSMVArichHVPanelTest(*this, vname16));
    std::string vname17 = StringUtil::form("tempsetup");
    add(new NSMVArichHVTest(*this, vname17));

    std::string vname05 = StringUtil::form("setallhv");
    add(new NSMVArichHVsetAllHV(*this, vname05));
    std::string vname06 = StringUtil::form("setallbias");
    add(new NSMVArichHVsetAllBias(*this, vname06));
    std::string vname07 = StringUtil::form("setallguard");
    add(new NSMVArichHVsetAllGuard(*this, vname07));
    std::string vname08 = StringUtil::form("setallbiasnominal");
    add(new NSMVArichHVsetAllBiasNominal(*this, vname08));


  }



  // NSM from DB
  //  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }

    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      int modid = c_ch.getInt("modid");
      //      int sector = c_ch.getInt("sector");

      int crateid = crate.getId();
      int slot =  c_ch.getInt("slot");
      int channel =  c_ch.getInt("channel");

      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d]", crateid, slot, channel);
      add(new NSMVHandlerText(vname + ".type", true, false, type));
      add(new NSMVHandlerInt(vname + ".modid", true, false, modid));
      //      add(new NSMVHandlerInt(vname + ".sector", true, false, sector));
    }

  }


  // NSM only monitor
  for (int i = 0; i < 420; i++) {
    for (int j = 0; j < 6; j++) {
      int state = getHAPDState(i + 1, j + 1);
      float vmon = getHAPDVoltageMonitor(i + 1, j + 1);
      float cmon = getHAPDCurrentMonitor(i + 1, j + 1);

      std::string vnamehapd = StringUtil::form("hapd[%d].typeid[%d]", i + 1, j + 1);
      add(new NSMVHandlerInt(vnamehapd + ".state", true, false, state));
      add(new NSMVHandlerFloat(vnamehapd + ".vmon", true, false, vmon));
      add(new NSMVHandlerFloat(vnamehapd + ".cmon", true, false, cmon));

      std::string vname_hapd = StringUtil::form("hapd[%d].typeid[%d]", i + 1, j + 1);
      add(new NSMVHandlerHVHAPDSwitch(*this, vname_hapd + ".switch", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                      hapd2ChannelNumber[i][j]));
      add(new NSMVHandlerHVHAPDRampUpSpeed(*this, vname_hapd + ".rampup", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                           hapd2ChannelNumber[i][j]));
      add(new NSMVHandlerHVHAPDRampDownSpeed(*this, vname_hapd + ".rampdown", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                             hapd2ChannelNumber[i][j]));
      add(new NSMVHandlerHVHAPDVoltageDemand(*this, vname_hapd + ".vdemand", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                             hapd2ChannelNumber[i][j]));
      add(new NSMVHandlerHVHAPDVoltageLimit(*this, vname_hapd + ".vlim", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                            hapd2ChannelNumber[i][j]));
      add(new NSMVHandlerHVHAPDCurrentLimit(*this, vname_hapd + ".clim", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                            hapd2ChannelNumber[i][j]));

      std::string vname_hapd2 = StringUtil::form("hapd[%d]", i + 1);
      add(new NSMVHandlerHVHAPDMask(*this, vname_hapd2 + ".mask", hapd2CrateNumber[i][j], hapd2SlotNumber[i][j],
                                    hapd2ChannelNumber[i][j]));



    }
  }


}

void ArichHVControlCallback::initialize(const HVConfig& hvconf) throw()
{
  /*  const unsigned short *slotlist = 0;
      float *temp;
      if ((ret = CAENHV_GetBdParam(handle, 2, slotlist, "Temp", &temp)) != CAENHV_OK) {
      LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
      }  else {
      LogFile::info("temp is: %d", temp);
      }
  */

  /*
    unsigned short  NrOfSlot;
    unsigned short * NrOfChList;
    char * ModelList;
    char * DescriptionList;
    unsigned short * SerNumList;
    unsigned char * FmwRelMinList;
    unsigned char * FmwRelMaxList;
    CAENHV_GetCrateMap(handle, &NrOfSlot, &NrOfChList, &ModelList, &DescriptionList, &SerNumList, &FmwRelMinList, &FmwRelMaxList);
  */
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  m_handle = std::vector<int>();
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& crate(crate_v[i]);
    //    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    //    const std::string host0 = crate.getHost(); //should be used
    //    std::cout << "host0 = " << host0 << std::endl;
    //char host[256];
    std::string host = crate.getHost();
    LogFile::notice("host = %s", host.c_str());
    //    printf("host = %s",host_tmp.c_str());
    //strcpy(host,"192.168.0.1");
    const char* user = "admin";
    const char* pass = "admin";
    int handle = 0;
    int ret = 0;
    if ((ret = CAENHV_InitSystem(SY4527, LINKTYPE_TCPIP, (char*)host.c_str(), user, pass, &handle)) != CAENHV_OK) {
      LogFile::error("Error on initialization: %d %s", ret, CAENHV_GetError(handle));
    }  else {
      LogFile::info("handler: %d", handle);
    }
    m_handle.push_back(handle);
    if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
      LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
    }  else {
      LogFile::info("Clear Alarm was done: %d", handle);
    }
  }
  g_mutex_arich.unlock();
  signal(SIGPIPE, SIG_IGN);

  fill_num();

  LogFile::debug("initialize : done");
  HVControlCallback::configure(hvconf);

}

void ArichHVControlCallback::deinitialize(int handle) throw()
{

  int ret = 0;
  g_mutex_arich.lock();
  if ((ret = CAENHV_DeinitSystem(handle)) != CAENHV_OK) {
    LogFile::error("Error on deinitialization: %d", ret);
  }  else {
    LogFile::info("handler: %d", handle);
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::fill_num() throw()
{
  LogFile::debug("fill number between HAPD and PS");
  printf("start fill number\n");
  //  std::string str_mask="mask";
  //  ch_mask = std::vector<std::vector<std::vector<std::string> > >(7, std::vector<std::vector<std::string> >(15,std::vector<std::string>(48)));
  //  hapd_mask = std::vector<std::string>(420);

  for (int i = 0; i < 420; i++) {
    hapd_mask[i] = "mask"; // true : hapd is disable, false : hapd is enable
    //    printf("i = %d\n",i);
    for (int l = 0; l < 6; l++) {
      hapd2CrateNumber[i][l] = 0;
      hapd2SlotNumber[i][l] = 0;
      hapd2ChannelNumber[i][l] = 0;
    }
  }
  //  printf("filename=%s,line=%d\n",__FILE__,__LINE__);

  for (int i = 0; i < 7; i++) {
    for (int l = 0; l < 15; l++) {
      for (int k = 0; k < 48; k++) {
        ps2modid[i][l][k] = 0;
        ps2typeid[i][l][k] = 0;
        ch_mask[i][l][k] = "mask";
        //  turnon_ch[i][l][k]=false; // true : channel is enable, false : channel is disable
      }
    }
  }
  //  printf("filename=%s,line=%d\n",__FILE__,__LINE__);


  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    HVCrate crate(i + 1);
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    DBObject& c_crate(c_crate_v[i]);
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);

    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      int modid = c_ch.getInt("modid");
      int sector = c_ch.getInt("sector");
      std::string str_mask = c_ch.getValueText("mask");
      //      bool turn = c_ch.getBool("turnon");


      int crateid = crate.getId();
      int slot =  c_ch.getInt("slot");
      int channel =  c_ch.getInt("channel");
      //      printf("%d,%d,%d : %s\n",crateid,slot,channel,mask.c_str());

      int type_id = 0;

      //type bias-a:1, bias-b:2, bias-c:3, bias-d:4, guard:5, hv:6
      if (type == "bias-a")type_id = 1;
      if (type == "bias-b")type_id = 2;
      if (type == "bias-c")type_id = 3;
      if (type == "bias-d")type_id = 4;
      if (type == "guard")type_id = 5;
      if (type == "hv")type_id = 6;

      hapd2CrateNumber[modid - 1][type_id - 1] = crateid;
      hapd2SlotNumber[modid - 1][type_id - 1] = slot;
      hapd2ChannelNumber[modid - 1][type_id - 1] = channel;

      //      printf("%d,%d,%d(%d)\n",hapd2CrateNumber[modid-1][5],hapd2SlotNumber[modid-1][5],hapd2ChannelNumber[modid-1][5],modid);

      ps2modid[crateid - 1][slot][channel] = modid;
      ps2typeid[crateid - 1][slot][channel] = type_id;
      ps2sector[crateid - 1][slot][channel] = sector;
      ch_mask[crateid - 1][slot][channel] = str_mask;
      //      turnon_ch[crateid][slot][channel]=turn;


      //      LogFile::debug("masked=%d",masked); // true : channel is disable, false : channel is enable
      //      setMaskedChannel(crate, slot, channel, masked);
      //      printf("%d,%d,%d (1): %s\n",crateid,slot,channel,mask.c_str());
      //      printf("%d,%d,%d (2): %s\n",crateid,slot,channel,ch_mask[crateid-1][slot][channel].c_str());
      //      LogFile::debug("masked2=%d",masked); // true : channel is disable, false : channel is enable
      //      LogFile::debug("masked(%d,%d,%d)=%d",crateid,slot,channel,ch_mask[crateid][slot][channel]); // true : channel is disable, false : channel is enable

      //      printf("%d,%d,%d : %d,%d,%d\n",crateid, slot, channel,
      //       hapd2CrateNumber[modid][type_id], hapd2SlotNumber[modid][type_id], hapd2ChannelNumber[modid][type_id]);
      //      printf("%d,%d : %d,%d\n",modid, type_id,
      //       ps2modid[crateid][slot][channel], ps2typeid[crateid][slot][channel]);
      //      printf("%d,%d,%d : %s\n",crateid,slot,channel,mask.c_str());

    }
  }

  //  printf("filename=%s,line=%d\n",__FILE__,__LINE__);


  /*
  for(int i=0;i<420;i++){
    //    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    printf("%d,%d,%d\n",hapd2CrateNumber[i][0],hapd2SlotNumber[i][0],hapd2ChannelNumber[i][0]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][0]-1][hapd2SlotNumber[i][0]][hapd2ChannelNumber[i][0]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][1],hapd2SlotNumber[i][1],hapd2ChannelNumber[i][1]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][1]-1][hapd2SlotNumber[i][1]][hapd2ChannelNumber[i][1]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][2],hapd2SlotNumber[i][2],hapd2ChannelNumber[i][2]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][2]-1][hapd2SlotNumber[i][2]][hapd2ChannelNumber[i][2]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][3],hapd2SlotNumber[i][3],hapd2ChannelNumber[i][3]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][3]-1][hapd2SlotNumber[i][3]][hapd2ChannelNumber[i][3]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][4],hapd2SlotNumber[i][4],hapd2ChannelNumber[i][4]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][4]-1][hapd2SlotNumber[i][4]][hapd2ChannelNumber[i][4]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][5],hapd2SlotNumber[i][5],hapd2ChannelNumber[i][5]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][5]-1][hapd2SlotNumber[i][5]][hapd2ChannelNumber[i][5]]<<std::endl;
  }
  */

  /*
  std::string str_no="no";
  for(int i=0;i<420;i++){
    printf("filename=%s,line=%d\n",__FILE__,__LINE__);
    printf("%d,%d,%d\n",hapd2CrateNumber[i][4],hapd2SlotNumber[i][4],hapd2ChannelNumber[i][4]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][4]-1][hapd2SlotNumber[i][4]][hapd2ChannelNumber[i][4]]<<std::endl;
    printf("%d,%d,%d\n",hapd2CrateNumber[i][5],hapd2SlotNumber[i][5],hapd2ChannelNumber[i][5]);
    std::cout<<"modid("<<i<<")="<<ch_mask[hapd2CrateNumber[i][5]-1][hapd2SlotNumber[i][5]][hapd2ChannelNumber[i][5]]<<std::endl;

    if( (ch_mask[hapd2CrateNumber[i][4]-1][hapd2SlotNumber[i][4]][hapd2ChannelNumber[i][4]].find("no")!=std::string::npos)&&
  (ch_mask[hapd2CrateNumber[i][5]-1][hapd2SlotNumber[i][5]][hapd2ChannelNumber[i][5]].find("no")!=std::string::npos) ){
      printf("%d (1): %s\n",i,hapd_mask[i].c_str());
      hapd_mask[i]=str_no;
      printf("%d (2): %s\n",i,hapd_mask[i].c_str());
    }
  }
  */

  /*
  for(int i=0;i<7;i++){
    for(int l=0;l<15;l++){
      for(int k=0;k<48;k++){
  std::cout<<"ch_mask = "<<ch_mask[i][l][k]<<std::endl;
      }
    }
  }

  for(int i=0;i<420;i++){
    std::cout<<"hapd_mask = "<<hapd_mask[i]<<std::endl;
  }
  */
  printf("finish fill number\n");
  LogFile::debug("finish fill number between HAPD and PS");

  //  printf("modid=%d,type=%s : %d,%d,%d\n",modid,type,crate_num[modid][type_id],slot_num[modid][type_id],channel_num[modid][type_id]);

}


int g_day = -1;
zlibstream g_zs;

void ArichHVControlCallback::update() throw()
{

#ifdef Recovery
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  int state[10][100][100];
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& c_crate(crate_v[i]);
    const HVChannelList& channel_v(c_crate.getChannels());
    int crate = crate_v[i].getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& ch(*ichannel);
      int slot = ch.getSlot();
      int channel = ch.getChannel();
      int handle = m_handle[i];
      //      unsigned short chan = channel;
      //      unsigned Status;
      //paramLock();
      state[crate][slot][channel] = getState(crate, slot, channel);//(int)unit.getStatus().getState();
      //paramUnlock();
      //      std::cout<<state[1][0][4]<<std::endl;
      //      int ret = 0;
      /*
      if(state[crate][slot][channel] & (1<<6)){
      LogFile::warning("%d, %d, %d is external Trip", crate, slot, channel);
      //   return HVMessage::ETRIP;
      }

      if(state[crate][slot][channel] & (1<<8)){
      LogFile::notice("%d, %d, %d is external disable", crate, slot, channel);
      int recovered = -1;
      if(recovered != crate){
      LogFile::debug("Run RecoveryInterlock");
      RecoveryInterlock(handle,crate,slot,channel);
      //   return HVMessage::INTERLOCK;
      recovered = crate;
      }
      }
      */
      //  if(state[crate][slot][channel] & (1<<9)){
      if (state[crate][slot][channel] == 7) {
        LogFile::warning("%d, %d, %d is internal Trip", crate, slot, channel);
        LogFile::debug("Run trip down");
        Trip_down(crate, slot, channel);
        //    return HVMessage::TRIP;
      }

    }
  }


#endif

  /*
  static unsigned long long count = 0;
  if (count % 5 == 0) {
    Date date;
    if (g_day != date.getDay()) {
      g_zs.close();
      g_zs = zlibstream();
      g_zs.open(StringUtil::form("/disk/data/datfiles/data/%s.dat", date.toString("%Y.%m.%d.%H.%M")).c_str(), "w");
      g_day = date.getDay();
    }
    const HVConfig& config(getConfig());
    const HVCrateList& crate_v(config.getCrates());
    //    int state[10][100][100];
    for (size_t i = 0; i < crate_v.size(); i++) {
      const HVCrate& c_crate(crate_v[i]);
      const HVChannelList& channel_v(c_crate.getChannels());
      int crate = crate_v[i].getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
     ichannel != channel_v.end(); ichannel++) {

  const HVChannel& ch(*ichannel);
  int slot = ch.getSlot();
  int channel = ch.getChannel();
  bool sw_tmp = getSwitch(crate,slot,channel);
  //  std::cout << "crate_v.size() = " << crate_v.size() << std::endl;
  //  int handle = m_handle[i];
  hvinfo info;
  info.record_time = (unsigned int)date.get();
  info.crate = crate;
  info.slot = slot;
  info.channel = channel;
  info.switchon = getState(info.crate, info.slot, info.channel);//(int)unit.getStatus().getState();
  info.vdemand = getVoltageDemand(info.crate, info.slot, info.channel);//param.getVoltageDemand();
  info.vmon = getVoltageMonitor(info.crate, info.slot, info.channel);//unit.getStatus().getVoltageMon();
  info.cmon = getCurrentMonitor(info.crate, info.slot, info.channel);//unit.getStatus().getCurrentMon();
  g_zs.write(info);
      }
    }
    g_zs.flush();
  }
  count++;
  */
}


void ArichHVControlCallback::configure(const HVConfig& config) throw(HVHandlerException)
{
  try {
    const HVConfig& config(getConfig());
    const HVCrateList& crate_v(config.getCrates());
    for (HVCrateList::const_iterator icrate = crate_v.begin();
         icrate != crate_v.end(); icrate++) {
      const HVCrate& crate(*icrate);
      const HVChannelList& channel_v(crate.getChannels());
      //      int crateid = crate.getId();
      for (HVChannelList::const_iterator ichannel = channel_v.begin();
           ichannel != channel_v.end(); ichannel++) {
        //        const HVChannel& channel(*ichannel);
        //        int slot = channel.getSlot();
        //        int ch = channel.getChannel();
      }
    }
  } catch (const IOException& e) {
    LogFile::error(e.what());
    throw (HVHandlerException(e.what()));
  }
}


void ArichHVControlCallback::store(int index) throw(IOException)
{
  //  for (size_t i = 0; i < m_acomm.size(); i++) {
  //m_acomm[i].store(index);
  //m_acomm[i].requestValueAll(0, 0);
  //  }
}

void ArichHVControlCallback::recall(int index) throw(IOException)
{
  //  for (size_t i = 0; i < m_acomm.size(); i++) {
  //m_acomm[i].recall(index);
  //m_acomm[i].requestValueAll(0, 0);
  //  }
}

/*
  for (size_t i = 0; i < m_acomm.size(); i++) {
  for (int j = 0; j < m_acomm[i].getNUnits(); j++) {
  ArichHVUnit& unit(m_acomm[i].getUnit(j));
  HVValue& param(unit.getValue());
  HVChannel& ch(unit.getChannel());
  if (unit.getStatus().getState() == 1) {
  std::cout << m_acomm[i].getId() << " "
  << ch.getSlot() << " "
  << ch.getChannel() << " "
  << unit.getStatus().getVoltageMon() << " "
  << unit.getStatus().getCurrentMon() << std::endl;
  }
  }

*/


void ArichHVControlCallback::turnon() throw(HVHandlerException)
{

#ifdef parallel_HAPD
  LogFile::debug("start turn on");
  bool hapdon = false;
  int count = 0;//count for timeout

  while (hapdon != true) {
    hapdon = true;

    for (int i = 1; i < 421; i++) {
      HAPD_on(i);
    }

    for (int i = 1; i < 421; i++) {
      if (check_HAPD_on(i) == false) hapdon = false;
    }
    count++;
    wait(5);

    if (count > 30) {
      LogFile::debug("turn on is failed (time out)");
      return;
    }

  }
  LogFile::debug("finish turn on");
#endif


#ifdef whole_HAPD
  LogFile::debug("start turn on");
  int count = 0;//count for timeout
  //  for(int i=0;i<16;i++)setVoltageDemand(1,3,i,1000); // for debug

  while (!(check_all_switch("guard", true) && check_all_switch("hv", true) && check_all_switch("bias", true)) &&
         !(getNode().getState() == HVState::STANDBY_S)) {
    if ((count < 20) &&
        ((getNode().getState() == HVState::OFF_S) ||
         (getNode().getState() == HVState::TURNINGON_TS))) {

      if (check_all_switch("guard", false) && check_all_switch("hv", false) && check_all_switch("bias", false)) {
        all_switch("guard", true);
        count = 0;
      } else if (check_all_switch("guard", true) && check_all_switch("hv", false) && check_all_switch("bias", false)) {
        all_switch("hv", true);
        count = 0;
      } else if (check_all_switch("guard", true) && check_all_switch("hv", true) && check_all_switch("bias", false)) {
        all_switch("bias", true);
        count = 0;
      } else {
        count++;
        std::cout << "count = " << count << std::endl;
      }
      wait(5);

    } else {
      LogFile::warning("turn on is failed");
      return;
    }
  }

  LogFile::debug("turn on is succeeded");
#endif

}

void ArichHVControlCallback::turnoff() throw(HVHandlerException)
{
#ifdef parallel_HAPD
  LogFile::debug("start turn off");
  bool hapdoff = false;
  int count = 0;//count for timeout

  while (hapdoff != true) {
    hapdoff = true;

    for (int i = 1; i < 421; i++) {
      HAPD_off(i);
    }

    for (int i = 1; i < 421; i++) {
      if (check_HAPD_off(i) == false) hapdoff = false;
    }
    count++;
    wait(5);

    if (count > 30) {
      LogFile::debug("turn off is failed (time out)");
      return;
    }
  }
  LogFile::debug("finish turn off");
#endif


#ifdef whole_HAPD
  LogFile::debug("start turn off");
  int count = 0;//count for timeout


  while (!(check_all_switch("guard", false) && check_all_switch("hv", false) && check_all_switch("bias", false)) &&
         !(getNode().getState() == HVState::OFF_S)) {

    if ((count < 20) &&
        ((getNode().getState() == HVState::STANDBY_S) ||
         (getNode().getState() == HVState::PEAK_S) ||
         (getNode().getState() == HVState::TURNINGOFF_TS))) {
      if (check_all_switch("guard", true) && check_all_switch("hv", true) && check_all_switch("bias", true)) {
        all_switch("bias", false);
        count = 0;
      } else if (check_all_switch("guard", true) && check_all_switch("hv", true) && check_all_switch("bias", false)) {
        all_switch("hv", false);
        count = 0;
      } else if (check_all_switch("guard", true) && check_all_switch("hv", false) && check_all_switch("bias", false)) {
        all_switch("guard", false);
        count = 0;
      } else {
        count++;
        std::cout << "count = " << count << std::endl;
      }
      wait(5);
    } else {
      LogFile::warning("turn off is failed");
      return;
    }

  }
  LogFile::debug("turn off is succeeded");
#endif

}

void ArichHVControlCallback::standby() throw(HVHandlerException)
{
  std::cout << "start standby" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish standby" << std::endl;
}

void ArichHVControlCallback::shoulder() throw(HVHandlerException)
{
  std::cout << "start shoulder" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish shoulder" << std::endl;
}

void ArichHVControlCallback::peak() throw(HVHandlerException)
{
  std::cout << "start peak" << std::endl;
  //load(getConfig(0), false, false);
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  for (HVCrateList::const_iterator icrate = crate_v.begin();
       icrate != crate_v.end(); icrate++) {
    const HVCrate& crate(*icrate);
    const HVChannelList& channel_v(crate.getChannels());
    //    int crateid = crate.getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      //      const HVChannel& channel(*ichannel);
      //      int slot = channel.getSlot();
      //      int ch = channel.getChannel();
    }
  }
  std::cout << "finish peak" << std::endl;
}

void ArichHVControlCallback::GBmdtest(int crate, int slot, float vset) throw(HVHandlerException)
{

  for (int i = 0; i < 48; i++) {
    setVoltageLimit(crate, slot, i, 500);
    setCurrentLimit(crate, slot, i, 500);
    setTripTime(crate, slot, i, 0.5);
    setVoltageDemand(crate, slot, i, vset + i);
  }

}
void ArichHVControlCallback::HVmdtest(int crate, int slot, float vset) throw(HVHandlerException)
{

  for (int i = 0; i < 16; i++) {
    setCurrentLimit(crate, slot, i, 50);
    setVoltageDemand(crate, slot, i, vset + (i * 10));
  }

}


void ArichHVControlCallback::temp_setup() throw(HVHandlerException)
{

}


void ArichHVControlCallback::test() throw(HVHandlerException)
{

  for (int i = 1; i < 421; i++) {
    printf("modid(%d)=%s\n", i, getHAPDMask(i).c_str());
  }

}


void ArichHVControlCallback::panel_test() throw(HVHandlerException)
{

  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      int modid = c_ch.getInt("modid");
      //      int modid = c_ch.getInt("modid");
      int type_id = -1;

      float vset = 0.0;


      int crateid = crate.getId();
      int slot =  c_ch.getInt("slot");

      if (type == "bias-a")type_id = 0;
      if (type == "bias-b")type_id = 1;
      if (type == "bias-c")type_id = 2;
      if (type == "bias-d")type_id = 3;
      if (type == "guard")type_id = 4;






      if (crateid == 1 && (type_id != -1) && (modid > 0)) {
        int channel =  c_ch.getInt("channel");
        vset = 450 + (type_id * 10);
        //  vset = 100+((modid-1)%9)*5+type_id;
        setVoltageLimit(crateid, slot, channel, 500);
        setCurrentLimit(crateid, slot, channel, 500);
        setRampUpSpeed(crateid, slot, channel, 50);
        setRampDownSpeed(crateid, slot, channel, 50);
        setVoltageDemand(crateid, slot, channel, vset);
      }


      /*
      if(crateid==1 && (slot==0||slot==1) && (type_id!=-1)){
      int channel =  c_ch.getInt("channel");
      vset=450+(type_id*10);
      // vset = (modid%9)*5+type_id
      setVoltageLimit(crateid,slot,channel,500);
      setCurrentLimit(crateid,slot,channel,500);
      setRampUpSpeed(crateid,slot,channel,50);
      setRampDownSpeed(crateid,slot,channel,50);
      setVoltageDemand(crateid,slot,channel,vset);
      }
      */

      if (crateid == 2 && (modid != -1)) {
        int channel =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        vset = 100 + (modid - 1) % 20 * 10.0;

        setVoltageLimit(crateid, slot, channel, 9000);
        setCurrentLimit(crateid, slot, channel, 50);
        setRampUpSpeed(crateid, slot, channel, 200);
        setRampDownSpeed(crateid, slot, channel, 500);
        setVoltageDemand(crateid, slot, channel, vset);
      }
    }
  }
}





void ArichHVControlCallback::Trip_down(int trip_crate, int trip_slot, int trip_channel) throw(HVHandlerException)
{

  //  std::cout << "test start" << std::endl;


  //HV tripped
  if (ps2typeid[trip_crate][trip_slot][trip_channel] == 6) {
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 1) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          1, false);
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 2) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          2, false);
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 3) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          3, false);
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 4) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          4, false);

    if ((getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 1) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 2) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 3) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 4) == 0)) {
      printf("trip down for modid:%d,type:%d is done\n", ps2modid[trip_crate][trip_slot][trip_channel],
             ps2typeid[trip_crate][trip_slot][trip_channel]);
      //      LogFile::debug("finish turn off");
    } else {
      printf("trip down for modid:%d,type:%d is working\n", ps2modid[trip_crate][trip_slot][trip_channel],
             ps2typeid[trip_crate][trip_slot][trip_channel]);
    }

  }


  //guard tripped
  if (ps2typeid[trip_crate][trip_slot][trip_channel] == 5) {
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 1) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          1, false);//should be kill
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 2) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          2, false);//should be kill
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 3) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          3, false);//should be kill
    if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 4) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
          4, false);//should be kill

    if ((getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 1) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 2) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 3) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 4) == 0)) {
      if (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 6) == 1)setHAPDSwitch(ps2modid[trip_crate][trip_slot][trip_channel],
            6, false);
    }

    if ((getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 1) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 2) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 3) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 4) == 0) &&
        (getHAPDState(ps2modid[trip_crate][trip_slot][trip_channel], 6) == 0)) {
      printf("trip down for modid:%d,type:%d is done\n", ps2modid[trip_crate][trip_slot][trip_channel],
             ps2typeid[trip_crate][trip_slot][trip_channel]);
      //      LogFile::debug("finish turn off");
    } else {
      printf("trip down for modid:%d,type:%d is working\n", ps2modid[trip_crate][trip_slot][trip_channel],
             ps2typeid[trip_crate][trip_slot][trip_channel]);
    }
  }



  /*
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj=obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++){
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")){
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));

    int modid = 0;
    for (size_t j = 0; j<c_ch_v.size(); j++){

      DBObject& c_ch(c_ch_v[j]);
      //      HVChannel ch(j, c_ch.getInt("slot"),
      //       c_ch.getInt("channel"),
      //       c_ch.getInt("turnon"));
      int crateid = crate.getId();
      int slot =  c_ch.getInt("slot");
      int channel =  c_ch.getInt("channel");
      //      printf("index = %d(%d)\n",index,j);
      //      printf("%d, %d, %d\n",crateid,slot,channel);

      //      if(modid == (c_ch.hasValue("modid")?c_ch.getInt("modid"):-1)){
      //  printf("switchoff(%d, %d, %d) %d(%s)\n",crateid,slot,channel,modid,type.c_str());
      //      }

      //      printf("current channel is (%d, %d, %d)\n",crateid,slot,channel);
      //      std::string type = c_ch.hasText("type")?c_ch.getValueText("type"):"no";
      //      std::cout << "type = " << type << std::endl;

      if( (trip_crate == crateid) &&
    (trip_slot == slot) &&
    (trip_channel == channel) ){
  //  index = c_ch.getIndex();
  modid = c_ch.hasValue("modid")?c_ch.getInt("modid"):-1;
        std::string type = c_ch.getValueText("type");
  //  printf("(%d, %d, %d) %d(%s) is tripped\n",crateid,slot,channel,modid,type.c_str());
  //  printf("(%d, %d, %d) %d\n",crateid,slot,channel,modidA);

  //guard tripped
  if(type == "guard"){
    bool bias_a_on = 1;
    bool bias_b_on = 1;
    bool bias_c_on = 1;
    bool bias_d_on = 1;
    for(size_t k = 0; k<c_ch_v.size(); k++){
      DBObject& c_ch1(c_ch_v[k]);
      //      std::cout <<  c_ch1.getValueText("type") << std::endl;
      std::string type_n = c_ch1.hasText("type")?c_ch1.getValueText("type"):"no";
      //      std::cout<<"type_n =" << type_n<<std::endl;
      //      std::string type_n = "bias";

      if((modid == (c_ch1.hasValue("modid")?c_ch1.getInt("modid"):-1)) &&
         (type_n.find("bias") != std::string::npos) ){
        int crateid = crate.getId();
        int slot =  c_ch1.getInt("slot");
        int channel =  c_ch1.getInt("channel");
        if(getState(crateid,slot,channel) == 1){
    printf("kill(%d, %d, %d) %d(%s)\n",crateid,slot,channel,modid,type_n.c_str());
        }else if(getState(crateid,slot,channel) == 0){
    printf("kill(%d, %d, %d) %d(%s) is done\n",crateid,slot,channel,modid,type_n.c_str());
    if(type_n == "bias-a")bias_a_on=0;
    if(type_n == "bias-b")bias_b_on=0;
    if(type_n == "bias-c")bias_c_on=0;
    if(type_n == "bias-d")bias_d_on=0;
        }
      }//bias finish

      if((modid == (c_ch1.hasValue("modid")?c_ch1.getInt("modid"):-1)) &&
         (type_n == "hv") ){
        if(bias_a_on==0 &&
     bias_b_on==0 &&
     bias_c_on==0 &&
     bias_d_on==0  ){
    if(getState(crateid,slot,channel) == 1){
      printf("switch off(%d, %d, %d) %d(%s)\n",crateid,slot,channel,modid,type_n.c_str());
    }else if(getState(crateid,slot,channel) == 0){
      printf("switch off(%d, %d, %d) %d(%s) is done\n",crateid,slot,channel,modid,type_n.c_str());
      printf("trip dwon(%d, %d, %d) %d(%s) is done\n",trip_crate,trip_slot,trip_channel,modid,type_n.c_str());
    }
        }
      }//hv finished

    }
  }

  //HV tripped
  if (type == "hv"){
    for (size_t l = 0; l<c_ch_v.size(); l++){
      DBObject& c_ch2(c_ch_v[l]);
      std::string type_n = c_ch2.getValueText("type");
      if((modid == (c_ch2.hasValue("modid")?c_ch2.getInt("modid"):-1) ) &&
         (type_n.find("bias") != std::string::npos) ){
        int crateid = crate.getId();
        int slot =  c_ch2.getInt("slot");
        int channel =  c_ch2.getInt("channel");
        if(getState(crateid,slot,channel) == 1){
    printf("switch off(%d, %d, %d) %d(%s)\n",crateid,slot,channel,modid,type_n.c_str());
        }else if(getState(crateid,slot,channel) == 0){
    printf("switch off(%d, %d, %d) %d(%s) is done\n",crateid,slot,channel,modid,type_n.c_str());
    printf("trip dwon(%d, %d, %d) %d(%s) is done\n",trip_crate,trip_slot,trip_channel,modid,type_n.c_str());
        }
        //        printf("switch off(%d, %d, %d) %d(%s)\n",crateid,slot,channel,modid,type_n.c_str());
      }
    }
  }


      }


    }
  }


  std::cout << "test end" << std::endl;
  */
}

void ArichHVControlCallback::md_all_switch(int crate, int slot, bool sw) throw(HVHandlerException)
{

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  //    int state[10][100][100];
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& c_crate(crate_v[i]);
    const HVChannelList& channel_v(c_crate.getChannels());
    int crateid = crate_v[i].getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& ch(*ichannel);
      int slotid = ch.getSlot();
      if (crateid == crate && slotid == slot) {
        int channel = ch.getChannel();
        setSwitch(crate, slot, channel, sw);
      }

    }
  }
}

void ArichHVControlCallback::all_off() throw(HVHandlerException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  //    int state[10][100][100];
  for (size_t i = 0; i < crate_v.size(); i++) {
    const HVCrate& c_crate(crate_v[i]);
    const HVChannelList& channel_v(c_crate.getChannels());
    int crate = crate_v[i].getId();
    for (HVChannelList::const_iterator ichannel = channel_v.begin();
         ichannel != channel_v.end(); ichannel++) {
      const HVChannel& ch(*ichannel);
      int slot = ch.getSlot();
      int channel = ch.getChannel();
      setSwitch(crate, slot, channel, false);

    }
  }

}

void ArichHVControlCallback::all_switch(std::string set_type, bool sw) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type.find(set_type) != std::string::npos) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        setSwitch(crateid, slot, ch, sw);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}

void ArichHVControlCallback::setAllHV(float vset) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type == "hv") {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        setVoltageLimit(crateid, slot, ch, vset + 100);
        setVoltageDemand(crateid, slot, ch, vset);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}

void ArichHVControlCallback::setAllBias(float vset) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if ((type == "bias-a") ||
          (type == "bias-b") ||
          (type == "bias-c") ||
          (type == "bias-d")) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        //  setVoltageLimit(crateid,slot,ch,vset+5);
        setVoltageDemand(crateid, slot, ch, vset);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}

void ArichHVControlCallback::setAllGuard(float vset) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type == "guard") {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        setVoltageLimit(crateid, slot, ch, vset + 5);
        setVoltageDemand(crateid, slot, ch, vset);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}

void ArichHVControlCallback::setAllBias_nominal(float less_vset) throw(HVHandlerException)
{
  //  printf("start all %s switch %d\n",set_type.c_str(),sw);
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  //  std::string m_type="";
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if ((type == "bias-a") ||
          (type == "bias-b") ||
          (type == "bias-c") ||
          (type == "bias-d")) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");
        float vdemand = c_ch.getFloat("vdemand");
        float vlimit = c_ch.getFloat("vlimit");
        float vset = vdemand - less_vset;
        //  printf("crateid %d,slot %d ,ch %d, vset = %f (vdemand:%f)\n",crateid,slot,ch,vset,vdemand);
        setVoltageLimit(crateid, slot, ch, vlimit);
        setVoltageDemand(crateid, slot, ch, vset);

        //  printf("setSwitch(%d, %d, %d, %d) %s\n",crateid,slot,ch,sw,type.c_str());
        //  setVoltageLimit(crateid,slot,ch,vset+100);
      }

    }
  }
  //  printf("start all %s switch sw",set_type);
}


bool ArichHVControlCallback::check_all_switch(std::string set_type, bool sw) throw(HVHandlerException)
{
  const HVConfig& config(getConfig());
  const DBObject& obj(config.get());
  DBObject m_obj = obj;
  if (!m_obj.hasObject("crate")) {
    LogFile::error("No crate entry in %s", m_obj.getName().c_str());
  }
  DBObjectList& c_crate_v(m_obj.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    HVCrate crate(i + 1);
    DBObject& c_crate(c_crate_v[i]);
    if (c_crate.hasText("name")) {
      crate.setName(c_crate.getText("name"));
    }
    if (!c_crate.hasObject("channel")) {
      LogFile::error("No channel entry in %s", m_obj.getName().c_str());
      continue;
    }
    DBObjectList& c_ch_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_ch_v.size(); j++) {

      DBObject& c_ch(c_ch_v[j]);
      HVChannel ch(j, c_ch.getInt("slot"),
                   c_ch.getInt("channel"),
                   c_ch.getInt("turnon"));
      std::string type = c_ch.getValueText("type");
      if (type.find(set_type) != std::string::npos) {
        int crateid = crate.getId();
        int slot =  c_ch.getInt("slot");
        int ch =  c_ch.getInt("channel");

        if (getChannelMask(crateid, slot, ch) == "mask")continue;
        if (getState(crateid, slot, ch) == sw ? 1 : 0) {
          continue;
        } else {
          return false;
        }

      }
    }
  }
  return true;
}


#ifdef Recovery_Function
void ArichHVControlCallback::RecoveryTrip(int handle, int crate, int slot, int channel) throw(IOException)
{

}

void ArichHVControlCallback::RecoveryInterlock(int handle, int crate, int slot, int channel) throw(IOException)
{
  int ret = 0;
  g_mutex_arich.lock();
  if ((ret = CAENHV_ExecComm(handle, "ClearAlarm")) != CAENHV_OK) {
    LogFile::error("error in Clear Alarm %d %s", ret, CAENHV_GetError(handle));
  }  else {
    LogFile::info("Clear Alarm was done: %d", handle);
    std::cout << "recovery interlock" << std::endl;
  }
  g_mutex_arich.unlock();

}

#endif

void ArichHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  if ((ch_mask[crate - 1][slot][channel] == "mask")) {
    LogFile::info("%d,%d,%d is mask", crate, slot, channel);
    return;
  }

  if ((ch_mask[crate - 1][slot][channel] == "mask") && (switchon == false) && (getSwitch(crate, slot, channel) == false)) {
    LogFile::info("%d,%d,%d is mask", crate, slot, channel);
    return;
  }

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned sw = switchon;
      if (switchon) {
        if ((ret = CAENHV_SetChParam(handle, slot, "Pw", 1, &chan, &sw)) == CAENHV_OK) {
          LogFile::info("switch %d.%d.%d %s", crate, slot, channel, "ON");
          std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
          set(vname + "switch", "ON");
        } else {
          LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, slot, channel, ret, CAENHV_GetError(handle));
        }
      } else {
        if ((ret = CAENHV_SetChParam(handle, slot, "Pw", 1, &chan, &sw)) == CAENHV_OK) {
          LogFile::info("switch %d.%d.%d %s", crate, slot, channel, "OFF");
          std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
          set(vname + "switch", "OFF");
        } else {
          LogFile::error("error in writing Pw %d.%d.%d %d %s", crate, slot, channel, ret, CAENHV_GetError(handle));
        }
      }

    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float rampup) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, RUp: %f", crate, slot, channel, rampup);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "RUp", 1, &chan, &rampup) != CAENHV_OK) {
        LogFile::error("error in writing RUp %f %s", rampup, CAENHV_GetError(handle));
      }
      LogFile::info("rampup %d.%d.%d %f [V/sec]", crate, slot, channel, rampup);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "rampup", rampup);

    }
  }
  g_mutex_arich.unlock();

}


void ArichHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float rampdown) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, RDwn: %f", crate, slot, channel, rampdown);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "RDWn", 1, &chan, &rampdown) != CAENHV_OK) {
        LogFile::error("error in writing RDwn %f %s", rampdown, CAENHV_GetError(handle));
      }
      LogFile::info("rampdown %d.%d.%d %f [V/sec]", crate, slot, channel, rampdown);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "rampdown", rampdown);
    }
  }
  g_mutex_arich.unlock();

}

void ArichHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, vdemand: %f", crate, slot, channel, voltage);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "V0Set", 1, &chan, &voltage) != CAENHV_OK) {
        LogFile::error("error in writing V0Set %f %s", voltage, CAENHV_GetError(handle));
      }
      LogFile::info("Vdemand %d.%d.%d %f [V]", crate, slot, channel, voltage);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "vdemand", voltage);
    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, SVMAx: %f", crate, slot, channel, voltage);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "SVMax", 1, &chan, &voltage) != CAENHV_OK) {
        LogFile::error("error in writing SVMax %f %s", voltage, CAENHV_GetError(handle));
      }
      LogFile::info("Vlimit %d.%d.%d %f [V] ", crate, slot, channel, voltage);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "vlimit", voltage);
    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, I0Set: %f", crate, slot, channel, current);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "I0Set", 1, &chan, &current) != CAENHV_OK) {
        LogFile::error("error in writing I0Set %f %s", current, CAENHV_GetError(handle));
      }
      LogFile::info("Ilimit %d.%d.%d %f [uA]", crate, slot, channel, current);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "climit", current);
    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setTripTime(int crate, int slot, int channel, float triptime) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, TripTime: %f", crate, slot, channel, triptime);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "Trip", 1, &chan, &triptime) != CAENHV_OK) {
        LogFile::error("error in writing TripTime %f %s", triptime, CAENHV_GetError(handle));
      }
      LogFile::info("TripTime %d.%d.%d %f [sec]", crate, slot, channel, triptime);
      std::string vname = StringUtil::form("crate[%d].slot[%d].channel[%d].", crateid, slot, channel);
      set(vname + "trip", triptime);
    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setPOn(int crate, int slot, int channel, bool pon) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, POn: %f", crate, slot, channel, pon);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "POn", 1, &chan, &pon) != CAENHV_OK) {
        LogFile::error("error in writing POn %f %s", pon, CAENHV_GetError(handle));
      }
      LogFile::info("POn %d.%d.%d %d ", crate, slot, channel, pon);
    }
  }
  g_mutex_arich.unlock();
}

void ArichHVControlCallback::setPDown(int crate, int slot, int channel, bool pdown) throw(IOException)
{
  LogFile::info("crate: %d, slot: %d, channel: %d, PDown: %f", crate, slot, channel, pdown);

  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int handle = m_handle[i];
      unsigned short chan = channel;
      if (CAENHV_SetChParam(handle, slot, "PDwn", 1, &chan, &pdown) != CAENHV_OK) {
        LogFile::error("error in writing PDown %f %s", pdown, CAENHV_GetError(handle));
      }
      LogFile::info("PDwon %d.%d.%d %d ", crate, slot, channel, pdown);
    }
  }
  g_mutex_arich.unlock();
}

float ArichHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Rup;
      if ((ret = CAENHV_GetChParam(handle, slot, "RUp", 1, &chan, &Rup)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return Rup;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading Rup %d.%d.%d %d %f %s", crate, slot, channel, ret, Rup, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float RDwn;
      if ((ret = CAENHV_GetChParam(handle, slot, "RDWn", 1, &chan, &RDwn)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return RDwn;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading RDwn %d.%d.%d %d %f %s", crate, slot, channel, ret, RDwn, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float V0Set;
      if ((ret = CAENHV_GetChParam(handle, slot, "V0Set", 1, &chan, &V0Set)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return V0Set;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading V0Set %d.%d.%d %d %f %s", crate, slot, channel, ret, V0Set, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float SVMax;
      if ((ret = CAENHV_GetChParam(handle, slot, "SVMax", 1, &chan, &SVMax)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return SVMax;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading SVMax %d.%d.%d %d %f %s", crate, slot, channel, ret, SVMax, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}


float ArichHVControlCallback::getTripTime(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float TripTime;
      if ((ret = CAENHV_GetChParam(handle, slot, "Trip", 1, &chan, &TripTime)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return TripTime;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading TripTime %d.%d.%d %d %f %s", crate, slot, channel, ret, TripTime, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float I0Set;
      if ((ret = CAENHV_GetChParam(handle, slot, "I0Set", 1, &chan, &I0Set)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return I0Set;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading Trip %d.%d.%d %d %f %s", crate, slot, channel, ret, I0Set, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Vmon;
      if ((ret = CAENHV_GetChParam(handle, slot, "VMon", 1, &chan, &Vmon)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return Vmon;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading Vmon %d.%d.%d %d %f %s", crate, slot, channel, ret, Vmon, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      float Imon;
      if ((ret = CAENHV_GetChParam(handle, slot, "IMon", 1, &chan, &Imon)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return Imon;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return -1;
      } else {
        LogFile::error("error in reading Imon %d.%d.%d %d %f %s", crate, slot, channel, ret, Imon, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

bool ArichHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned PW;
      if ((ret = CAENHV_GetChParam(handle, slot, "Pw", 1, &chan, &PW)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return PW == 1;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return false;
      } else {
        LogFile::error("error in reading PW %d.%d.%d %d %u %s", crate, slot, channel, ret, PW, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return false;
      }
    }
  }
  g_mutex_arich.unlock();
  return false;
}

bool ArichHVControlCallback::getPOn(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned POn;
      if ((ret = CAENHV_GetChParam(handle, slot, "POn", 1, &chan, &POn)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return POn == 1;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        //  LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return false;
      } else {
        LogFile::error("error in reading POn %d.%d.%d %d %u %s", crate, slot, channel, ret, POn, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return false;
      }
    }
  }
  g_mutex_arich.unlock();
  return false;
}

bool ArichHVControlCallback::getPDown(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned PDown;
      if ((ret = CAENHV_GetChParam(handle, slot, "PDwn", 1, &chan, &PDown)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return PDown == 1;
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        LogFile::info("%d,%d,%d is mask", crate, slot, channel);
        g_mutex_arich.unlock();
        return false;
      } else {
        LogFile::error("error in reading PDown %d.%d.%d %d %u %s", crate, slot, channel, ret, PDown, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return false;
      }
    }
  }
  g_mutex_arich.unlock();
  return false;
}

float ArichHVControlCallback::getBdTemp(int crate, int slot) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short sl = slot;
      float BdTemp;
      if ((ret = CAENHV_GetBdParam(handle, 1, &sl, "Temp", &BdTemp)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return BdTemp;
      } else {
        LogFile::error("error in reading BdTemp %d.%d %d %u %s", crate, slot,  ret, BdTemp, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }
    }
  }
  g_mutex_arich.unlock();
  return 0;
}

float ArichHVControlCallback::getBdHVMax(int crate, int slot) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {
      int ret = 0;
      int handle = m_handle[i];
      unsigned short sl = slot;
      float BdHVMax;
      if ((ret = CAENHV_GetBdParam(handle, 1, &sl, "HVMax", &BdHVMax)) == CAENHV_OK) {
        g_mutex_arich.unlock();
        return BdHVMax;
      } else {
        LogFile::error("error in reading BdTemp %d.%d %d %u %s", crate, slot, ret, BdHVMax, CAENHV_GetError(handle));
        g_mutex_arich.unlock();
        return -1;
      }

    }
  }
  g_mutex_arich.unlock();
  return 0;
}

int ArichHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  const HVConfig& config(getConfig());
  const HVCrateList& crate_v(config.getCrates());
  g_mutex_arich.lock();
  for (size_t i = 0; i < crate_v.size(); i++) {
    int crateid = crate_v[i].getId();
    if (crateid == crate) {

      int ret = 0;
      int handle = m_handle[i];
      unsigned short chan = channel;
      unsigned Status = 0;
      if ((ret = CAENHV_GetChParam(handle, slot, "Status", 1, &chan, &Status)) == CAENHV_OK) {
        //LogFile::info("current %d.%d.%d 0x%x", crate, slot, channel, Status);
        if (Status & (1 << 1)) {
          g_mutex_arich.unlock();
          return HVMessage::RAMPUP;
        } else if (Status & (1 << 2)) {
          g_mutex_arich.unlock();
          return HVMessage::RAMPDOWN;
        } else if (Status & (1 << 3)) {
          LogFile::error("%d, %d, %d is over current", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::OCP;
        } else  if (Status & (1 << 4)) {
          LogFile::error("%d, %d, %d is over voltage", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::OVP;
        } else if (Status & (1 << 5)) {
          LogFile::error("%d, %d, %d is under voltage", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 6)) {
          g_mutex_arich.unlock();
          return HVMessage::ETRIP;
        } else  if (Status & (1 << 7)) {
          LogFile::error("%d, %d, %d is Max V %d", crate, slot, channel, Status);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 8)) {
          if (getNode().getState() != HVState::ERROR_ES)setHVState(HVState::ERROR_ES);
          g_mutex_arich.unlock();
          return HVMessage::INTERLOCK;
        } else if (Status & (1 << 9)) {
          LogFile::error("%d, %d, %d is TRIP %d", crate, slot, channel, Status);
          if (getNode().getState() != HVState::TRIP_ES)setHVState(HVState::TRIP_ES);
          g_mutex_arich.unlock();
          return HVMessage::TRIP;
        } else if (Status & (1 << 10)) {
          LogFile::error("%d, %d, %d is calibration error", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 11)) {
          LogFile::fatal("%d, %d, %d is unplugged", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 12)) {
          LogFile::error("%d, %d, %d is error", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 13)) {
          LogFile::error("%d, %d, %d is Over Voltage Protection", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 14)) {
          LogFile::error("%d, %d, %d is Power Fail", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 15)) {
          LogFile::error("%d, %d, %d is Temperature Error", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ERR;
        } else if (Status & (1 << 0)) {
          //LogFile::info("%d, %d, %d is ON", crate, slot, channel);
          g_mutex_arich.unlock();
          return HVMessage::ON;
        } else if (!(Status & (1 << 0))) {
          g_mutex_arich.unlock();
          return HVMessage::OFF;
        }
      } else if (ch_mask[crate - 1][slot][channel] == "mask") {
        LogFile::info("%d,%d,%d is mask", crate, slot, channel);
      } else {
        LogFile::error("error in reading Status %d.%d.%d %d %u %s", crate, slot, channel, ret, Status, CAENHV_GetError(handle));
      }

    }
  }
  g_mutex_arich.unlock();
  return 0;
}


void ArichHVControlCallback::setHAPDSwitch(int modid, int type_id, bool switchon) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], switchon);
}

void ArichHVControlCallback::setHAPDRampUpSpeed(int modid, int type_id, float rampup) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], rampup);
}

void ArichHVControlCallback::setHAPDRampDownSpeed(int modid, int type_id, float rampdown) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], rampdown);
}

void ArichHVControlCallback::setHAPDVoltageDemand(int modid, int type_id, float voltage) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], voltage);
}

void ArichHVControlCallback::setHAPDVoltageLimit(int modid, int type_id, float voltage) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], voltage);
}

void ArichHVControlCallback::setHAPDCurrentLimit(int modid, int type_id, float current) throw(IOException)
{
  setSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
            hapd2ChannelNumber[modid - 1][type_id - 1], current);
}


bool ArichHVControlCallback::getHAPDSwitch(int modid, int type_id) throw(IOException)
{
  return getSwitch(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                   hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDRampUpSpeed(int modid, int type_id) throw(IOException)
{
  return getRampUpSpeed(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                        hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDRampDownSpeed(int modid, int type_id) throw(IOException)
{
  return getRampDownSpeed(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                          hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDVoltageDemand(int modid, int type_id) throw(IOException)
{
  return getVoltageDemand(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                          hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDVoltageLimit(int modid, int type_id) throw(IOException)
{
  return getVoltageLimit(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                         hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDCurrentLimit(int modid, int type_id) throw(IOException)
{
  return getCurrentLimit(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                         hapd2ChannelNumber[modid - 1][type_id - 1]);
}

int ArichHVControlCallback::getHAPDState(int modid, int type_id) throw(IOException)
{
  return getState(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                  hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDVoltageMonitor(int modid, int type_id) throw(IOException)
{
  return getVoltageMonitor(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                           hapd2ChannelNumber[modid - 1][type_id - 1]);
}

float ArichHVControlCallback::getHAPDCurrentMonitor(int modid, int type_id) throw(IOException)
{
  return getCurrentMonitor(hapd2CrateNumber[modid - 1][type_id - 1], hapd2SlotNumber[modid - 1][type_id - 1],
                           hapd2ChannelNumber[modid - 1][type_id - 1]);
}


void ArichHVControlCallback::setChannelMask(int crate, int slot, int channel, std::string mask) throw(IOException)
{
  ch_mask[crate - 1][slot][channel] = mask;
  LogFile::info("%d.%d.%d : %s", crate, slot, channel, mask.c_str());
}
std::string ArichHVControlCallback::getChannelMask(int crate, int slot, int channel) throw(IOException)
{
  //  LogFile::info("%d,%d,%d masked : %d", crate, slot, channel, ch_mask[crate][slot][channel]);
  return ch_mask[crate - 1][slot][channel];
}

void ArichHVControlCallback::setHAPDMask(int modid, std::string mask) throw(IOException)
{
  for (int type_id = 0; type_id < 6; type_id++) {
    int crate = hapd2CrateNumber[modid - 1][type_id];
    int slot = hapd2SlotNumber[modid - 1][type_id];
    int channel = hapd2ChannelNumber[modid - 1][type_id];
    ch_mask[crate - 1][slot][channel] = mask;
  }
  hapd_mask[modid - 1] = mask;
  LogFile::info("HAPD(%d) : %s", modid,  mask.c_str());
}

std::string ArichHVControlCallback::getHAPDMask(int modid) throw(IOException)
{
  return hapd_mask[modid - 1];
}


void ArichHVControlCallback::HAPD_on(int modid) throw(HVHandlerException)
{

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 0) &&
      (getHAPDState(modid, 6) == 0)) setHAPDSwitch(modid, 5, true);

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 0)) setHAPDSwitch(modid, 6, true);

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 1)) {
    setHAPDSwitch(modid, 1, true);
    setHAPDSwitch(modid, 2, true);
    setHAPDSwitch(modid, 3, true);
    setHAPDSwitch(modid, 4, true);
  }

  if ((getHAPDState(modid, 1) == 1) &&
      (getHAPDState(modid, 2) == 1) &&
      (getHAPDState(modid, 3) == 1) &&
      (getHAPDState(modid, 4) == 1) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 1)) printf("HAPD #%d is ready\n", modid);

}

void ArichHVControlCallback::HAPD_off(int modid) throw(HVHandlerException)
{

  if ((getHAPDState(modid, 1) == 1) &&
      (getHAPDState(modid, 2) == 1) &&
      (getHAPDState(modid, 3) == 1) &&
      (getHAPDState(modid, 4) == 1) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 1)) {
    setHAPDSwitch(modid, 1, false);
    setHAPDSwitch(modid, 2, false);
    setHAPDSwitch(modid, 3, false);
    setHAPDSwitch(modid, 4, false);
  }

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 1)) setHAPDSwitch(modid, 6, false);

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 0)) setHAPDSwitch(modid, 5, false);

  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 0) &&
      (getHAPDState(modid, 6) == 0)) printf("HAPD #%d is off\n", modid);

}


bool ArichHVControlCallback::check_HAPD_on(int modid) throw(HVHandlerException)
{
  if ((getHAPDState(modid, 1) == 1) &&
      (getHAPDState(modid, 2) == 1) &&
      (getHAPDState(modid, 3) == 1) &&
      (getHAPDState(modid, 4) == 1) &&
      (getHAPDState(modid, 5) == 1) &&
      (getHAPDState(modid, 6) == 1)) {
    return true;
  } else {
    return false;
  }

}

bool ArichHVControlCallback::check_HAPD_off(int modid) throw(HVHandlerException)
{
  if ((getHAPDState(modid, 1) == 0) &&
      (getHAPDState(modid, 2) == 0) &&
      (getHAPDState(modid, 3) == 0) &&
      (getHAPDState(modid, 4) == 0) &&
      (getHAPDState(modid, 5) == 0) &&
      (getHAPDState(modid, 6) == 0)) {
    return true;
  } else {
    return false;
  }

}
