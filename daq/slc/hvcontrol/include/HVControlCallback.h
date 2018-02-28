#ifndef _Belle2_HVControlCallback_hh
#define _Belle2_HVControlCallback_hh

#include "daq/slc/hvcontrol/HVCallback.h"

#include <daq/slc/system/Mutex.h>

#include <daq/slc/nsm/NSMData.h>

namespace Belle2 {

  class DBInterface;

  class HVControlCallback : public HVCallback {

  public:
    HVControlCallback(const NSMNode& node)
    throw() : HVCallback(), m_db(NULL)
    {
      setNode(node);
    }
    virtual ~HVControlCallback() throw() {}

  public:
    virtual void configure(const HVConfig&) throw(HVHandlerException);
    virtual void turnon() throw(HVHandlerException);
    virtual void turnoff() throw(HVHandlerException);
    virtual void standby() throw(HVHandlerException);
    virtual void shoulder() throw(HVHandlerException);
    virtual void peak() throw(HVHandlerException);
    virtual void update() throw(HVHandlerException) {}

  public:
    virtual void init(NSMCommunicator&) throw();
    virtual void timeout(NSMCommunicator&) throw();

  public:
    void setDB(const std::string& table, DBInterface* db) throw()
    {
      m_table = table;
      m_db = db;
    }
    DBInterface* getDB() throw() { return m_db; }

  protected:
    void load(const HVConfig& config,
              bool alloff, bool loadpars) throw(HVHandlerException);
    void dbload(HVConfig& config, const std::string& confignames) throw(IOException);

    void monitor() throw();

  private:
    DBInterface* m_db;
    NSMData m_data;
    std::string m_table;

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
