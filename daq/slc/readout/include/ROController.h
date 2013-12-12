#ifndef _Belle2_ROController_h
#define _Belle2_ROController_h

#include "daq/slc/readout/ROMessanger.h"

#include "daq/slc/nsm/RCCallback.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

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
    void setCallback(RCCallback* callback) {
      _callback = callback;
    }
    void setScriptDir(const std::string& scriptdir) { _scriptdir = scriptdir; }
    void setScript(const std::string& script) { _script = script; }
    const std::string& getScriptDir() { return _scriptdir; }
    const std::string& getScript() { return _script; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }
    State wait(int timeout);
    bool init();
    bool load(int timeout);
    bool start(int timeout);
    bool stop(int timeout);
    bool abort();

  private:
    ROMessanger _msg;
    RCCallback* _callback;
    Fork _fork;
    PThread _thread;
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
