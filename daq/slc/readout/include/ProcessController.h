#ifndef _Belle2_ProcessController_h
#define _Belle2_ProcessController_h

#include "daq/slc/readout/RunInfoBuffer.h"

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/runcontrol/RCState.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"
#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"

#include <vector>

namespace Belle2 {

  class ProcessController {

    friend class ProcessSubmitter;

  public:
    ProcessController() {}
    ~ProcessController() throw() {}

  public:
    bool init(const std::string& name, int nreserved = 0);
    void clear();
    bool load(int timeout);
    bool start();
    bool stop();
    bool abort();
    const std::string& getName() { return _name; }
    const std::string& getExecutable() { return _exename; }
    RunInfoBuffer& getInfo() { return _info; }
    RCCallback* getCallback() { return _callback; }
    const Fork& getFork() const { return  _fork; }
    void setCallback(RCCallback* callback) { _callback = callback; }
    void setName(const std::string& name) { _name = name; }
    void setExecutable(const std::string& exe) { _exename = exe; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }
    const RCState& getState() const { return _state; }
    void setState(const RCState& state) { _state = state; }

  public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    RunInfoBuffer _info;
    std::string _name;
    RCCallback* _callback;
    std::string _exename;
    std::vector<std::string> _arg_v;
    Fork _fork;
    PThread _thread;
    Mutex _mutex;
    std::string _message;
    RCState _state;

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
