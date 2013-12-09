#ifndef _Belle2_RunInfoBuffer_h
#define _Belle2_RunInfoBuffer_h

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

namespace Belle2 {

  struct run_info {
    unsigned int state;
    unsigned int exp_number;
    unsigned int cold_number;
    unsigned int hot_number;
    unsigned int nodeid;
  };

  class RunInfoBuffer {

  public:
    RunInfoBuffer() {}
    ~RunInfoBuffer() {}

  public:
    size_t size() throw();
    bool open(const std::string& path);
    bool init();
    bool close();
    bool unlink();
    bool lock() throw();
    bool unlock() throw();
    bool wait() throw();
    bool wait(int time) throw();
    bool notify() throw();

  public:
    const std::string getPath() const throw() { return _path; }
    unsigned int getState() const throw() { return _info->state; }
    unsigned int getExpNumber() const throw() { return _info->exp_number; }
    unsigned int getColdNumber() const throw() { return _info->cold_number; }
    unsigned int getHotNumber() const throw() { return _info->hot_number; }
    unsigned int getNodeId() const throw() { return _info->nodeid; }
    void setState(unsigned int state) { _info->state = state; }
    void setExpNumber(unsigned int number) { _info->exp_number = number; }
    void setColdNumber(unsigned int number) { _info->cold_number = number; }
    void setHotNumber(unsigned int number) { _info->hot_number = number; }
    void setNodeId(unsigned int id) { _info->nodeid = id; }
    void clear();

  private:
    std::string _path;
    SharedMemory _memory;
    MMutex _mutex;
    MCond _cond;
    run_info* _info;
  };

}

#endif
