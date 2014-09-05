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
    virtual void init() throw();
    virtual bool perform(const NSMMessage& msg) throw();
    virtual void timeout() throw();
    virtual bool start() throw();
    virtual bool stop() throw();

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

  private:
    ConfigFile& m_config;
    Mutex m_mutex;
    Cond m_cond;
    unsigned int m_expno;
    unsigned int m_runno;
    unsigned int m_count;
    std::vector<DQMFileReader> m_reader_v;

  };

}

#endif
