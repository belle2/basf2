#ifndef _Belle2_ProcessController_h
#define _Belle2_ProcessController_h

#include "daq/slc/readout/RunInfoBuffer.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/system/Fork.h"
#include "daq/slc/system/Mutex.h"
#include "daq/slc/system/Cond.h"

#include <vector>
#include <sstream>

namespace Belle2 {

  class ProcessController {

    friend class ProcessSubmitter;

  public:
    ProcessController() throw() {}
    ProcessController(RCCallback* callback) throw() {
      m_callback = callback;
    }
    ~ProcessController() throw() {
      m_info.close();
    }

  public:
    bool init(const std::string& name, int nodeid = 0);
    void clear();
    bool load(int timeout);
    bool start(int expno, int runno);
    bool stop();
    bool abort();
    const std::string& getName() { return m_name; }
    const std::string& getExecutable() { return m_exename; }
    RunInfoBuffer& getInfo() { return m_info; }
    RCCallback* getCallback() { return m_callback; }
    const Fork& getFork() const { return  m_fork; }
    Fork& getFork() { return  m_fork; }
    void setCallback(RCCallback* callback) { m_callback = callback; }
    void setName(const std::string& name) { m_name = name; }
    void setExecutable(const std::string& exe) { m_exename = exe; }
    void addArgument(const std::string& arg) { m_arg_v.push_back(arg); }
    void addArgument(const char* format, ...);
    template<typename T>
    void addArgument(T arg);
    void clearArguments() { m_arg_v = std::vector<std::string>(); }
    bool isAlive() throw() { return m_fork.isAlive(); }

  public:
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }

  private:
    RunInfoBuffer m_info;
    std::string m_name;
    RCCallback* m_callback;
    std::string m_exename;
    std::vector<std::string> m_arg_v;
    Fork m_fork;
    Mutex m_mutex;
    std::string m_message;

  };

  template<typename T>
  inline void ProcessController::addArgument(T arg)
  {
    std::stringstream ss; ss << arg;
    m_arg_v.push_back(ss.str());
  }

  class ProcessSubmitter {

  public:
    ProcessSubmitter(ProcessController* con, int iopipe[2])
      : m_con(con) {
      m_iopipe[0] = iopipe[0];
      m_iopipe[1] = iopipe[1];
    }

  public:
    void run();

  private:
    ProcessController* m_con;
    int m_iopipe[2];

  };

}

#endif
