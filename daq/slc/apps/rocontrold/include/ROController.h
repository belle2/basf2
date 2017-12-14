#ifndef _Belle2_ROController_h
#define _Belle2_ROController_h

#include <daq/slc/readout/ProcessController.h>
#include <daq/slc/readout/FlowMonitor.h>

#include <daq/slc/database/DBObject.h>

namespace Belle2 {

  class ROCallback;

  class ROController {

  public:
    ROController() : m_used(true) {}
    virtual ~ROController() throw();

  public:
    bool init(ROCallback* callback, int id,
              const std::string& name,
              const DBObject& obj) throw();
    bool load(const DBObject& obj, int timeout) throw(RCHandlerException);
    bool start(int expno, int runno) throw(RCHandlerException);
    bool pause() throw();
    bool resume(int subno) throw();
    bool abort() throw();
    bool term() throw();
    bool stop() throw(RCHandlerException);

  public:
    virtual void check() {}

  protected:
    virtual void initArguments(const DBObject&) = 0;
    virtual bool loadArguments(const DBObject&) = 0;

  public:
    bool isUsed() const throw() { return m_used; }
    void setUsed(bool used) throw() { m_used = used; }
    FlowMonitor& getFlow() { return m_flow; }
    const FlowMonitor& getFlow() const { return m_flow; }
    ProcessController& getControl() { return m_con; }

  protected:
    bool m_used;
    ROCallback* m_callback;
    int m_id;
    std::string m_name;
    ProcessController m_con;
    FlowMonitor m_flow;
    std::string m_executable;

  };

}

#endif
