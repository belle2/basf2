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
    bool start();
    bool stop();
    bool abort();
    const std::string& getName() { return _name; }
    const std::string& getExecutable() { return _exename; }
    ProcessStatusBuffer& getMessanger() { return _msg; }
    RunInfoBuffer& getInfo() { return _msg.getInfo(); }
    ProcessLogBuffer& getLog() { return _msg.getLog(); }
    RCCallback* getCallback() { return _callback; }
    const Fork& getFork() const { return  _fork; }
    void setCallback(RCCallback* callback) { _callback = callback; }
    void setName(const std::string& name) { _name = name; }
    void setExecutable(const std::string& exe) { _exename = exe; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }

  public:
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    ProcessStatusBuffer _msg;
    std::string _name;
    RCCallback* _callback;
    std::string _exename;
    std::vector<std::string> _arg_v;
    Fork _fork;
    PThread _thread;
    Mutex _mutex;
    std::string _message;

  };

  class ProcessSubmitter {

  public:
    ProcessSubmitter(ProcessController* con)
      : _con(con) {}

  public:
    void run();

  private:
    ProcessController* _con;

  };

}

#endif
