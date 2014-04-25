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
      m_nreserved = 0;
      m_buf = NULL;
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
    const std::string getName() const throw() { return m_nodename; }
    const std::string getPath() const throw() { return m_path; }
    bool isAvailable() const throw() { return m_buf != NULL; }
    unsigned int* getParams() throw() { return m_buf; }
    unsigned int getState() const throw() { return m_buf[0]; }
    unsigned int getExpNumber() const throw() { return m_buf[1]; }
    unsigned int getRunNumber() const throw() { return m_buf[2]; }
    unsigned int getSubNumber() const throw() { return m_buf[3]; }
    unsigned int getNodeId() const throw() { return m_buf[4]; }
    unsigned int* getReserved() throw() { return m_buf + 5; }
    void setState(unsigned int state) { m_buf[0] = state; }
    void setExpNumber(unsigned int number) { m_buf[1] = number; }
    void setRunNumber(unsigned int number) { m_buf[2] = number; }
    void setSubNumber(unsigned int number) { m_buf[3] = number; }
    void setNodeId(unsigned int id) { m_buf[4] = id; }
    bool waitRunning(int timeout);
    bool reportRunning();
    bool reportError();
    bool reportReady();
    bool reportNotReady();

  private:
    std::string m_nodename;
    std::string m_path;
    int m_nreserved;
    SharedMemory m_memory;
    unsigned int* m_buf;
    MMutex m_mutex;
    MCond m_cond;

  };

}

#endif
