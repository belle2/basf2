#ifndef _Belle2_HVControlCallback_hh
#define _Belle2_HVControlCallback_hh

#include "daq/slc/hvcontrol/HVCallback.h"
#include "daq/slc/hvcontrol/HVChannelStatus.h"
#include "daq/slc/hvcontrol/HVConfig.h"

#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class DBInterface;

  class HVControlCallback : public HVCallback {

    friend class HVNodeMonitor;

  public:
    HVControlCallback(const NSMNode& node) throw();
    virtual ~HVControlCallback() throw();

  public:
    virtual void initialize() throw() = 0;
    virtual bool configure() throw() = 0;
    virtual bool turnon() throw() { return true; }
    virtual bool turnoff() throw() { return true; }
    virtual bool standby() throw() { return true; }
    virtual bool shoulder() throw() { return true; }
    virtual bool peak() throw() { return true; }

  public:
    void setDB(DBInterface* db) throw() { m_db = db; }
    size_t getNChannels() const throw() { return m_config.getNChannels(); }
    HVConfig& getConfig() throw() { return m_config; }
    HVChannelStatus& getChannelStatus(int i) throw() { return m_status_v[i]; }

  public:
    virtual void init() throw();
    virtual bool config() throw();
    void monitor() throw();

  private:
    DBInterface* m_db;
    NSMData m_data;
    HVConfig m_config;
    HVChannelStatusList m_status_v;

  private:
    class HVNodeMonitor {

    public:
      HVNodeMonitor(HVControlCallback* callback)
        : m_callback(callback) {}
      ~HVNodeMonitor() throw() {}

    public:
      void run();

    private:
      HVControlCallback* m_callback;

    };

  };

};

#endif
