#ifndef _Belle2_ROControlMessanger_h
#define _Belle2_ROControlMessanger_h

#include "daq/slc/readout/modules/RunInfoBuffer.h"
#include "daq/slc/readout/modules/RunLogMessanger.h"

#include <vector>

namespace Belle2 {

  class RCCallback;

  class ROControlMessanger {

  public:
    ROControlMessanger() {}
    ROControlMessanger(const std::string& nodename, int nodeid);
    ~ROControlMessanger();

  public:
    void setNode(const std::string& nodename, int nodeid);
    void setScriptDir(const std::string& scriptdir) { _scriptdir = scriptdir; }
    void setScript(const std::string& script) { _script = script; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }
    RunInfoBuffer& getInfo() { return _buf; }
    RunLogMessanger& getLog() { return _msg; }
    bool reportStarted();
    bool reportStopped();
    int waitStarted(int timeout);
    int waitStopped(int timeout);
    bool start(int exp_no, int cold_no, int hot_no);
    bool stop();
    bool create();
    bool open();
    bool close();
    bool unlink();

  private:
    RunInfoBuffer _buf;
    RunLogMessanger _msg;
    std::string _nodename;
    int _nodeid;
    std::string _script;
    std::string _scriptdir;
    std::vector<std::string> _arg_v;
    std::string _buf_path;
    std::string _fifo_path;

  };

}

#endif
