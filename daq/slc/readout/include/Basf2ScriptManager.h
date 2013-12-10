#ifndef _Belle2_Basf2ScriptManager_h
#define _Belle2_Basf2ScriptManager_h

#include "daq/slc/base/NSMNode.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/PThread.h"

#include "daq/slc/readout/RunInfoBuffer.h"
#include "daq/slc/readout/RunLogMessanger.h"

namespace Belle2 {

  class RCCallback;

  class Basf2ScriptManager {

  public:
    Basf2ScriptManager() : _node(NULL), _callback(NULL) {}
    Basf2ScriptManager(const std::string& name);
    Basf2ScriptManager(RCCallback* callback) {
      setCallback(callback);
    }
    ~Basf2ScriptManager();

  public:
    void setNode(NSMNode* node);
    void setCallback(RCCallback* callback);
    void setScriptDir(const std::string& scriptdir) { _scriptdir = scriptdir; }
    void setScript(const std::string& script) { _script = script; }
    void addArgument(const std::string& arg) { _arg_v.push_back(arg); }
    void clearArguments() { _arg_v = std::vector<std::string>(); }
    bool create();
    bool open();
    bool close();
    bool unlink();
    bool load();
    bool start();
    bool stop();
    void run();

  private:
    NSMNode* _node;
    RCCallback* _callback;
    RunInfoBuffer _buf;
    RunLogMessanger _msg;
    Fork _fork;
    PThread _thread;
    std::string _script;
    std::string _scriptdir;
    std::vector<std::string> _arg_v;
    std::string _buf_path;
    std::string _fifo_path;

  };

}

#endif
