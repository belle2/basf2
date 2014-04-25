#ifndef _Belle2_DQMViewMaster_h
#define _Belle2_DQMViewMaster_h

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/system/Cond.h>

#include <daq/slc/runcontrol/RCState.h>

#include <string>
#include <vector>

namespace Belle2 {

  class RCCallback;

  class DQMViewMaster {

  public:
    void add(const std::string& pack_name, int port,
             const std::string& file_path) {
      m_reader_v.push_back(DQMFileReader(pack_name, file_path));
      m_port_v.push_back(port);
    }
    void init();
    bool boot();
    bool abort();
    std::vector<DQMFileReader>& getReaders() { return m_reader_v; }
    void setRunNumbers(unsigned int expno, unsigned int runno) {
      m_expno = expno;
      m_runno = runno;
    }
    RCState& getState() { return m_state; }
    void setState(RCState state) { m_state = state; }
    unsigned int getExpNumber() const { return m_expno; }
    unsigned int getRunNumber() const { return m_runno; }
    void setCallback(RCCallback* callback) { m_callback = callback; }
    void notify() { m_cond.broadcast(); }
    void wait() { m_cond.wait(m_mutex); }
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  private:
    Mutex m_mutex;
    Cond m_cond;
    RCState m_state;
    unsigned int m_expno;
    unsigned int m_runno;
    std::vector<DQMFileReader> m_reader_v;
    std::vector<ProcessController> m_con_v;
    std::vector<int> m_port_v;
    RCCallback* m_callback;

  };

}

#endif
