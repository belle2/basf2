#ifndef _Belle2_ProcessStatusBuffer_h
#define _Belle2_ProcessStatusBuffer_h

#include "daq/slc/readout/RunInfoBuffer.h"
#include "daq/slc/readout/ProcessLogBuffer.h"

#include <vector>

namespace Belle2 {

  class RCCallback;

  class ProcessStatusBuffer {

  public:
    ProcessStatusBuffer() {}
    ProcessStatusBuffer(const std::string& nodename, int nodeid);
    ~ProcessStatusBuffer();

  public:
    void setNode(const std::string& nodename, int nodeid);
    const std::string& getNodeName() { return _nodename; }
    int getNodeId() { return _nodeid; }
    RunInfoBuffer& getInfo() { return _buf; }
    ProcessLogBuffer& getLog() { return _msg; }
    bool reportReady();
    bool reportRunning();
    bool reportError(const std::string message);
    bool reportFatal(const std::string message);
    bool create();
    bool open(const std::string& nodename, int nodeid) {
      setNode(nodename, nodeid);
      return open();
    }
    bool open();
    bool close();
    bool unlink();

  private:
    RunInfoBuffer _buf;
    ProcessLogBuffer _msg;
    std::string _nodename;
    int _nodeid;
    std::string _buf_path;
    std::string _fifo_path;

  };

}

#endif
