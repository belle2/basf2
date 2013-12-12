#ifndef _Belle2_ROMessanger_h
#define _Belle2_ROMessanger_h

#include "daq/slc/readout/RunInfoBuffer.h"
#include "daq/slc/readout/RunLogMessanger.h"

#include <vector>

namespace Belle2 {

  class RCCallback;

  class ROMessanger {

  public:
    ROMessanger() {}
    ROMessanger(const std::string& nodename, int nodeid);
    ~ROMessanger();

  public:
    void setNode(const std::string& nodename, int nodeid);
    const std::string& getNodeName() { return _nodename; }
    int getNodeId() { return _nodeid; }
    RunInfoBuffer& getInfo() { return _buf; }
    RunLogMessanger& getLog() { return _msg; }
    bool reportReady();
    bool reportRunning();
    bool reportError(const std::string message);
    bool reportFatal(const std::string message);
    bool create();
    bool open();
    bool close();
    bool unlink();

  private:
    RunInfoBuffer _buf;
    RunLogMessanger _msg;
    std::string _nodename;
    int _nodeid;
    std::string _buf_path;
    std::string _fifo_path;

  };

}

#endif
