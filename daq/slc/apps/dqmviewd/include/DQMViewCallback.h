#ifndef _Belle2_DQMViewCallback_h
#define _Belle2_DQMViewCallback_h

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/apps/dqmviewd/HistMemory.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback : public NSMCallback {

  public:
    DQMViewCallback(const NSMNode& node, ConfigFile& config);
    virtual ~DQMViewCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();

  public:
    std::vector<TH1*>& getHists() { return m_hist; }
    void notify() { m_cond.broadcast(); }
    void wait() { m_cond.wait(m_mutex); }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    void update() throw();

  private:
    ConfigFile& m_config;
    Mutex m_mutex;
    Cond m_cond;
    int m_count;
    std::vector<HistMemory> m_memory;
    std::vector<TH1*> m_hist;

  };

}

#endif
