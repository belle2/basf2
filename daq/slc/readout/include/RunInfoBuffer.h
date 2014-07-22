#ifndef _Belle2_RunInfoBuffer_h
#define _Belle2_RunInfoBuffer_h

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"
#include "daq/slc/readout/ronode_info.h"

namespace Belle2 {

  class RunInfoBuffer {

  public:
    enum State {
      NOTREADY = 0,
      READY = 1,
      RUNNING = 2
    };

  public:
    enum EFlag {
      NOERRO,
      PROCESS_DOWN = 0x01,
      SOCKET_BIND = 0x02,
      SOCKET_IN = 0x04,
      SOCKET_OUT = 0x08,
      EVENTFORMAT = 0x10,
      CPRFIFO_EMPTY = 0x20,
      CPRFIFO_FULL = 0x40,
      CPRFIFO_LEF_FULL = 0x80
    };

  public:
    RunInfoBuffer() {
      m_info = NULL;
    }
    ~RunInfoBuffer() {}

  public:
    size_t size() throw();
    bool open(const std::string& nodename,
              int nodeid = 0, bool recreate = false);
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
    bool isAvailable() const throw() { return m_info != NULL; }
    ronode_info* get() throw() { return m_info; }
    unsigned int getNodeId() const throw() { return m_info->nodeid; }
    unsigned int getState() const throw() { return m_info->state; }
    unsigned int getErrorFlag() const throw() { return m_info->eflag; }
    unsigned int getExpNumber() const throw() { return m_info->expno; }
    unsigned int getRunNumber() const throw() { return m_info->runno; }
    unsigned int getSubNumber() const throw() { return m_info->subno; }
    int getInputPort() const throw() { return m_info->io[0].port; }
    int getInputAddress() const throw() { return m_info->io[0].addr; }
    unsigned int getInputCount() const throw() { return m_info->io[0].count; }
    unsigned long long getInputNBytes() const throw() { return m_info->io[0].nbyte; }
    int getOutputPort() const throw() { return m_info->io[1].port; }
    int getOutputAddress() const throw() { return m_info->io[1].addr; }
    unsigned int getOutputCount() const throw() { return m_info->io[1].count; }
    unsigned long long getOutputNBytes() const throw() { return m_info->io[1].nbyte; }
    void setNodeId(unsigned int id) { m_info->nodeid = id; }
    void setState(State state) { m_info->state = (unsigned int)state; }
    void setErrorFlag(EFlag eflag) { m_info->eflag = (unsigned int)eflag; }
    void setExpNumber(unsigned int expno) { m_info->expno = expno; }
    void setRunNumber(unsigned int runno) { m_info->runno = runno; }
    void setSubNumber(unsigned int subno) { m_info->subno = subno; }
    void setInputPort(int port) { m_info->io[0].port = port; }
    void setInputAddress(int addr) { m_info->io[0].addr = addr; }
    void setInputCount(unsigned int count) { m_info->io[0].count = count; }
    void setInputNBytes(unsigned long long nbyte) { m_info->io[0].nbyte = nbyte; }
    void addInputCount(unsigned int count) { m_info->io[0].count += count; }
    void addInputNBytes(unsigned long long nbyte) { m_info->io[0].nbyte += nbyte; }
    void setOutputPort(int port) { m_info->io[1].port = port; }
    void setOutputAddress(int addr) { m_info->io[1].addr = addr; }
    void setOutputCount(unsigned int count) { m_info->io[1].count = count; }
    void setOutputNBytes(unsigned long long nbyte) { m_info->io[1].nbyte = nbyte; }
    void addOutputCount(unsigned int count) { m_info->io[1].count += count; }
    void addOutputNBytes(unsigned long long nbyte) { m_info->io[1].nbyte += nbyte; }

    bool waitRunning(int timeout);
    bool reportRunning();
    bool reportError(EFlag eflag);
    bool reportReady();
    bool reportNotReady();

  private:
    std::string m_nodename;
    std::string m_path;
    SharedMemory m_memory;
    ronode_info* m_info;
    MMutex m_mutex;
    MCond m_cond;

  };

}

#endif
