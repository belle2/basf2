#ifndef _Belle2_DQMViewCallback_h
#define _Belle2_DQMViewCallback_h

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/dqm/DQMPackage.h>
#include <daq/slc/dqm/DQMHistMap.h>

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/Cond.h>

#include <daq/slc/base/ConfigFile.h>

#include <vector>

namespace Belle2 {

  class DQMViewCallback : public NSMCallback {

  public:
    DQMViewCallback(const NSMNode& node, ConfigFile& config);
    virtual ~DQMViewCallback() throw() {}

  public:
    virtual void init(NSMCommunicator& com) throw();
    virtual void timeout(NSMCommunicator& com) throw();
    virtual void vset(NSMCommunicator& com, const NSMVar& var) throw();

    bool record() throw();

  public:
    void addReader(const std::string& pack_name,
                   const std::string& file_path) {
      m_reader_v.push_back(DQMFileReader(pack_name, file_path));
    }
    std::vector<DQMFileReader>& getReaders() { return m_reader_v; }
    unsigned int getExpNumber() const { return m_expno; }
    unsigned int getRunNumber() const { return m_runno; }
    void notify() { m_cond.broadcast(); }
    void wait() { m_cond.wait(m_mutex); }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    void update() throw();

  private:
    ConfigFile& m_config;
    Mutex m_mutex;
    Cond m_cond;
    int m_expno;
    int m_runno;
    int m_subno;
    int m_count;
    std::string m_dump_path;
    std::vector<DQMFileReader> m_reader_v;
    NSMNode m_runcontrol;

  };

}

#endif
