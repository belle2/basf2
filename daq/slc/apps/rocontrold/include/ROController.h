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
    bool load(int timeout) throw();
    bool start(int expno, int runno) throw();
    bool recover(int timeout) throw();
    bool abort() throw();
    bool term() throw();

  protected:
    virtual bool initArguments(const DBObject&) throw() = 0;
    virtual void loadArguments() throw() = 0;

  public:
    bool isUsed() const throw() { return m_used; }
    void setUsed(bool used) throw() { m_used = used; }
    FlowMonitor& getFlow() { return m_flow; }
    const FlowMonitor& getFlow() const { return m_flow; }

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
