#ifndef _Belle2_ECLShaperControllerCallback_h
#define _Belle2_ECLShaperControllerCallback_h

#include "daq/slc/apps/ecldspd/ECLShaperController.h"
#include "daq/slc/apps/ecldspd/ECLShaperConfig.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class ECLShaperControllerCallback : public RCCallback {

  public:
    ECLShaperControllerCallback() : m_forced(true) {}
    virtual ~ECLShaperControllerCallback() throw() {}

  public:
    virtual void load(const DBObject& obj) throw(RCHandlerException);
    virtual void recover(const DBObject& obj) throw(RCHandlerException);
    virtual void abort() throw(RCHandlerException);

  private:
    ECLShaperController m_con;
    ECLShaperConfig m_config;
    NSMData m_data;
    bool m_forced;

  };

}

#endif
