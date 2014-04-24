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
      _reader_v.push_back(DQMFileReader(pack_name, file_path));
      _port_v.push_back(port);
    }
    void init();
    bool boot();
    bool abort();
    std::vector<DQMFileReader>& getReaders() { return _reader_v; }
    void setRunNumbers(unsigned int expno, unsigned int runno) {
      _expno = expno;
      _runno = runno;
    }
    RCState& getState() { return _state; }
    void setState(RCState state) { _state = state; }
    unsigned int getExpNumber() const { return _expno; }
    unsigned int getRunNumber() const { return _runno; }
    void setCallback(RCCallback* callback) { _callback = callback; }
    void notify() { _cond.broadcast(); }
    void wait() { _cond.wait(_mutex); }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    Mutex _mutex;
    Cond _cond;
    RCState _state;
    unsigned int _expno;
    unsigned int _runno;
    std::vector<DQMFileReader> _reader_v;
    std::vector<ProcessController> _con_v;
    std::vector<int> _port_v;
    RCCallback* _callback;

  };

}

#endif
