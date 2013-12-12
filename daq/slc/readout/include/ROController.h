#ifndef _Belle2_ROController_h
#define _Belle2_ROController_h

#include "daq/slc/readout/ROMessanger.h"

#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"
#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"

#include "daq/slc/base/State.h"

#include <vector>

namespace Belle2 {

  class ROController {

    friend class ROSubmitter;

  public:
    ROController(RCCallback* callback = NULL) {
      setCallback(callback);
    }
    ~ROController() throw() {}

  public:
    State wait(int timeout);
    bool init();
    void clear();
    bool load(int timeout);
    bool start(int timeout);
    bool stop(int timeout);
    bool abort();
    ROMessanger& getMessanger() { return _msg; }
    RunInfoBuffer& getInfo() { return _msg.getInfo(); }
    RunLogMessanger& getLog() { return _msg.getLog(); }
    RCCallback* getCallback() { return _callback; }
    const State& getState() const { return  _state; }
    const Fork& getFork() const { return  _fork; }
    void setState(const State& state) { _state = state; }
    void setCallback(RCCallback* callback) { _callback = callback; }
    void setScriptDir(const std::string& scriptdir) { _scriptdir = scriptdir; }
    void setScript(const std::string& script) { _script = script; }
    const std::string& getScriptDir() { return _scriptdir; }
    const std::string& getScript() { return _script; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }

  public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }
    void signal() { _cond.signal(); }
    const std::string& getMessage() { return _message; }
    void setMessage(const std::string& msg) { _message = msg; }

  private:
    ROMessanger _msg;
    RCCallback* _callback;
    State _state;
    Fork _fork;
    PThread _thread;
    PThread _thread_msg;
    Mutex _mutex;
    Cond _cond;
    std::string _message;
    std::string _script;
    std::string _scriptdir;
    std::vector<std::string> _arg_v;

  };

  class ROSubmitter {
  public:
    ROSubmitter(ROController* con) : _con(con) {}
  public:
    void run();
  private:
    ROController* _con;
  };

}

#endif
