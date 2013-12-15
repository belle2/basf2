#ifndef _Belle2_ProcessController_h
#define _Belle2_ProcessController_h

#include "daq/slc/readout/ProcessStatusBuffer.h"

#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"
#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"

#include "daq/slc/base/State.h"

#include <vector>

namespace Belle2 {

  class ProcessController {

    friend class ProcessSubmitter;

  public:
    ProcessController() {}
    ~ProcessController() throw() {}

  public:
    State wait(int timeout);
    bool init(const std::string& name = "");
    void clear();
    bool load(int timeout);
    bool start(int timeout);
    bool stop(int timeout);
    bool abort();
    const std::string& getName() { return _name; }
    const std::string& getExecutable() { return _exename; }
    ProcessStatusBuffer& getMessanger() { return _msg; }
    RunInfoBuffer& getInfo() { return _msg.getInfo(); }
    ProcessLogBuffer& getLog() { return _msg.getLog(); }
    RCCallback* getCallback() { return _callback; }
    const State& getState() const { return  _state; }
    const Fork& getFork() const { return  _fork; }
    void setState(const State& state) { _state = state; }
    void setCallback(RCCallback* callback) { _callback = callback; }
    void setName(const std::string& name) { _name = name; }
    void setExecutable(const std::string& exe) { _exename = exe; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }

  public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
    void signal() { _cond.signal(); }
    const std::string& getMessage() { return _message; }
    void setMessage(const std::string& msg) { _message = msg; }

  private:
    ProcessStatusBuffer _msg;
    std::string _name;
    RCCallback* _callback;
    std::string _exename;
    std::vector<std::string> _arg_v;
    Fork _fork;
    PThread _thread;
    Mutex _mutex;
    Cond _cond;
    State _state;
    std::string _message;

  };

  class ProcessSubmitter {

  public:
    ProcessSubmitter(ProcessController* con, int iopipe[2])
      : _con(con) {
      _iopipe[0] = iopipe[0];
      _iopipe[1] = iopipe[1];
    }

  public:
    void run();

  private:
    ProcessController* _con;
    int _iopipe[2];

  };

}

#endif
