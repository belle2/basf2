#ifndef _Belle2_HVControlCallback_hh
#define _Belle2_HVControlCallback_hh

#include "daq/slc/apps/hvcontrold/HVCallback.h"
#include "daq/slc/apps/hvcontrold/hv_status.h"

#include <daq/slc/database/DBInterface.h>

#include <daq/slc/system/TCPSocket.h>

#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class HVControlCallback : public HVCallback {

    friend class HVNodeMonitor;

  public:
    HVControlCallback(NSMNode* node) throw();
    virtual ~HVControlCallback() throw();

  public:
    virtual void initialize() throw() = 0;
    virtual bool configure() throw() = 0;
    virtual bool turnon() throw() { return standby(); }
    virtual bool turnoff() throw() { return true; }
    virtual bool standby() throw() { return true; }
    virtual bool standby2() throw() { return true; }
    virtual bool standby3() throw() { return true; }
    virtual bool peak() throw() { return true; }

  public:
    void setDB(DBInterface* db) { m_db = db; }
    virtual void init() throw();
    virtual bool rampup() throw();
    virtual bool rampdown() throw();
    virtual bool config() throw();
    virtual bool save() throw();
    HVNodeInfo& getInfo() throw() { return m_info; }

  protected:
    HVNodeInfo m_info;
    hv_status* m_status;

  private:
    NSMData* m_data;
    DBInterface* m_db;

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
