#ifndef _Belle2_COPPERCallback_h
#define _Belle2_COPPERCallback_h

#include "HSLBController.h"

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/runcontrol/RCCallback.h"

namespace Belle2 {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(const NSMNode& node,
                   const std::string& rc_config);
    virtual ~COPPERCallback() throw();

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    ProcessController m_con;
    HSLBController m_hslbcon_v[4];
    int m_confno;
    std::string m_path;
    std::string m_hostname;

  };

}

#endif
