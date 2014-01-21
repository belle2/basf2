#ifndef _Belle2_DQMViewMaster_h
#define _Belle2_DQMViewMaster_h

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/system/Fork.h>
#include <daq/slc/system/Cond.h>

#include <daq/slc/base/State.h>

#include <string>
#include <vector>

namespace Belle2 {

  class DQMViewMaster {

  public:
    void add(const std::string& pack_name, int port,
             const std::string& file_path) {
      _reader_v.push_back(DQMFileReader(pack_name, file_path));
      _port_v.push_back(port);
    }
    bool boot();
    bool abort();
    std::vector<DQMFileReader>& getReaders() { return _reader_v; }
    void setRunNumbers(unsigned int expno, unsigned int runno) {
      _expno = expno;
      _runno = runno;
    }
    State& getState() { return _state; }
    void setState(State state) { _state = state; }
    unsigned int getExpNumber() const { return _expno; }
    unsigned int getRunNumber() const { return _runno; }
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    Mutex _mutex;
    Cond _cond;
    State _state;
    unsigned int _expno;
    unsigned int _runno;
    std::vector<DQMFileReader> _reader_v;
    std::vector<int> _port_v;
    std::vector<Fork> _fork_v;

  };

  class HSeverExecutor {

  public:
    HSeverExecutor(int port, const std::string& mapfile,
                   const std::string& dir)
      : _port(port), _mapfile(mapfile), _dir(dir) {}

  public:
    void run();

  private:
    int _port;
    std::string _mapfile;
    std::string _dir;

  };

}

#endif
