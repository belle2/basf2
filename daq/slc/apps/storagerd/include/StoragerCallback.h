#ifndef _Belle2_StoragerCallback_hh
#define _Belle2_StoragerCallback_hh

#include <daq/slc/readout/ProcessController.h>

#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMData.h>

#include <vector>

namespace Belle2 {

  class StoragerCallback : public RCCallback {

  public:
    StoragerCallback(const NSMNode& node);
    virtual ~StoragerCallback() throw();

  public:
    size_t getNControllers() const { return m_con.size(); }
    ProcessController& getController(int n) { return m_con[n]; }
    std::vector<ProcessController>& getControllers() { return m_con; }
    NSMData& getData() throw() { return m_data; }
    const NSMData& getData() const throw() { return m_data; }

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
    std::vector<ProcessController> m_con;
    NSMData m_data;

  };

}

#endif
