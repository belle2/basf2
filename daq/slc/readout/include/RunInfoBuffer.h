#ifndef _Belle2_RunInfoBuffer_h
#define _Belle2_RunInfoBuffer_h

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

namespace Belle2 {

  class RunInfoBuffer {

  public:
    static const unsigned int NOTREADY = 0;
    static const unsigned int READY = 1;
    static const unsigned int RUNNING = 2;
    static const unsigned int ERROR = 3;

  public:
    RunInfoBuffer() {
      _nreserved = 0;
      _buf = NULL;
    }
    ~RunInfoBuffer() {}

  public:
    size_t size() throw();
    bool open(const std::string& nodename,
              int nreserved = 0,
              bool recreate = false);
    bool init();
    bool close();
    bool unlink();
    bool lock() throw();
    bool unlock() throw();
    bool wait() throw();
    bool wait(int time) throw();
    bool notify() throw();
    void clear();

  public:
    const std::string getPath() const throw() { return _path; }
    bool isAvailable() const throw() { return _buf != NULL; }
    unsigned int* getParams() throw() { return _buf; }
    unsigned int getState() const throw() { return _buf[0]; }
    unsigned int getExpNumber() const throw() { return _buf[1]; }
    unsigned int getColdNumber() const throw() { return _buf[2]; }
    unsigned int getHotNumber() const throw() { return _buf[3]; }
    unsigned int getNodeId() const throw() { return _buf[4]; }
    unsigned int* getReserved() throw() { return _buf + 5; }
    void setState(unsigned int state) { _buf[0] = state; }
    void setExpNumber(unsigned int number) { _buf[1] = number; }
    void setColdNumber(unsigned int number) { _buf[2] = number; }
    void setHotNumber(unsigned int number) { _buf[3] = number; }
    void setNodeId(unsigned int id) { _buf[4] = id; }
    bool waitRunning(int timeout);
    bool reportRunning();
    bool reportError();
    bool reportReady();
    bool reportNotReady();

  private:
    std::string _path;
    int _nreserved;
    SharedMemory _memory;
    unsigned int* _buf;
    MMutex _mutex;
    MCond _cond;

  };

}

#endif
