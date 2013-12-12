#ifndef _Belle2_ROMessanger_h
#define _Belle2_ROMessanger_h

#include "daq/slc/readout/modules/RunInfoBuffer.h"
#include "daq/slc/readout/modules/RunLogMessanger.h"

namespace Belle2 {

  class RCCallback;

  class ROMessanger {

  public:
    ROMessanger() {}
    ROMessanger(const std::string& nodename, int nodeid);
    ~ROMessanger();

  public:
    void setNode(const std::string& nodename, int nodeid);
    RunInfoBuffer& getInfo() { return _buf; }
    RunLogMessanger& getLog() { return _msg; }
    bool reportReady();
    bool reportRunning();
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
